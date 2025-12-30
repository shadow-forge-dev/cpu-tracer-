#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdatomic.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static Vec3 trace(const Scene* scene, Ray r, int depth, int max_depth, Sampler* sampler) {
    if (depth >= max_depth) return (Vec3){0};

    float t, u, v;
    int tri_index;
    
    if (!bvh_intersect(&scene->bvh, r, 0.001f, MAX_FLOAT, &t, &tri_index, &u, &v)) {
        return (Vec3){0.05f, 0.05f, 0.05f}; 
    }

    Triangle tri = scene->triangles[tri_index];
    Material mat = scene->materials[tri.material_id];
    
    Vec3 n = vec3_normalize(vec3_add(vec3_scale(tri.n0, 1.0f - u - v), vec3_add(vec3_scale(tri.n1, u), vec3_scale(tri.n2, v))));
    Vec3 p = ray_at(r, t);
    
    Vec3 emission = mat.emission;
    if (vec3_length_sq(emission) > 0.0f) {
        if (depth == 0) return emission;
        return emission; 
    }

    Vec3 s, t_vec;
    vec3_coordinate_system(n, &s, &t_vec);
    Vec3 wo = vec3_scale(r.direction, -1.0f);

    Vec3 Ld = {0};
    if (scene->light_count > 0) {
        int light_idx = (int)(sampler_next_1d(sampler) * scene->light_count);
        if (light_idx >= scene->light_count) light_idx = scene->light_count - 1;
        const Light* light = &scene->lights[light_idx];
        
        Vec3 wi_light;
        float pdf_light, dist_light;
        Vec3 Li = light_sample(light, p, sampler, &wi_light, &pdf_light, &dist_light);
        
        if (pdf_light > 0.0f && vec3_length_sq(Li) > 0.0f) {
            Ray shadow_ray = { .origin = p, .direction = wi_light };
            float t_shadow, u_s, v_s;
            int tri_shadow;
            if (!bvh_intersect(&scene->bvh, shadow_ray, 0.001f, dist_light - 0.001f, &t_shadow, &tri_shadow, &u_s, &v_s)) {
                Vec3 f = material_eval(&mat, wo, wi_light, n, s, t_vec);
                Ld = vec3_add(Ld, vec3_scale(vec3_mul(f, Li), 1.0f / pdf_light));
            }
        }
        Ld = vec3_scale(Ld, (float)scene->light_count);
    }

    Vec3 wi;
    float pdf;
    Vec3 f = material_sample(&mat, wo, &wi, n, s, t_vec, sampler, &pdf);
    
    if (pdf > 0.0f && vec3_length_sq(f) > 0.0f) {
        float max_comp = fmaxf(f.x, fmaxf(f.y, f.z));
        if (depth > 3) {
            float q = fmaxf(0.05f, 1.0f - max_comp);
            if (sampler_next_1d(sampler) < q) return Ld;
            f = vec3_scale(f, 1.0f / (1.0f - q));
        }
        
        Ray next_ray = { .origin = p, .direction = wi };
        Vec3 Li = trace(scene, next_ray, depth + 1, max_depth, sampler);
        return vec3_add(Ld, vec3_mul(f, vec3_scale(Li, 1.0f / pdf)));
    }
    return Ld;
}

typedef struct {
    int id;
    const Scene* scene;
    const Camera* camera;
    const RenderOptions* options;
    float* buffer;
    atomic_int* tile_index;
    int tiles_x, tiles_y;
} ThreadData;

#define TILE_SIZE 32

void* render_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int width = data->options->width;
    int height = data->options->height;
    int total_tiles = data->tiles_x * data->tiles_y;
    
    Sampler sampler;
    sampler_init(&sampler, data->id * 123456789ULL, data->id);

    while (1) {
        int tile = atomic_fetch_add(data->tile_index, 1);
        if (tile >= total_tiles) break;
        
        int tx = tile % data->tiles_x;
        int ty = tile / data->tiles_x;
        
        int x_start = tx * TILE_SIZE;
        int y_start = ty * TILE_SIZE;
        int x_end = (x_start + TILE_SIZE < width) ? x_start + TILE_SIZE : width;
        int y_end = (y_start + TILE_SIZE < height) ? y_start + TILE_SIZE : height;
        
        for (int y = y_start; y < y_end; y++) {
            for (int x = x_start; x < x_end; x++) {
                Vec3 color = {0};
                for (int s = 0; s < data->options->samples_per_pixel; s++) {
                    float u = (float)x + sampler_next_1d(&sampler);
                    float v = (float)y + sampler_next_1d(&sampler);
                    Ray r = camera_get_ray(data->camera, u / width, (height - v) / height, &sampler);
                    color = vec3_add(color, trace(data->scene, r, 0, data->options->max_bounces, &sampler));
                }
                color = vec3_scale(color, 1.0f / data->options->samples_per_pixel);
                
                int idx = (y * width + x) * 3;
                data->buffer[idx + 0] = color.x;
                data->buffer[idx + 1] = color.y;
                data->buffer[idx + 2] = color.z;
            }
        }
    }
    return NULL;
}

void render(const Scene* scene, const Camera* camera, const RenderOptions* options) {
    int width = options->width;
    int height = options->height;
    float* buffer = (float*)calloc(width * height * 3, sizeof(float));
    
    int tiles_x = (width + TILE_SIZE - 1) / TILE_SIZE;
    int tiles_y = (height + TILE_SIZE - 1) / TILE_SIZE;
    atomic_int tile_index = 0;
    
    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * options->num_threads);
    ThreadData* thread_data = (ThreadData*)malloc(sizeof(ThreadData) * options->num_threads);
    
    printf("Rendering %dx%d with %d samples, %d threads...\n", width, height, options->samples_per_pixel, options->num_threads);
    
    for (int i = 0; i < options->num_threads; i++) {
        thread_data[i].id = i;
        thread_data[i].scene = scene;
        thread_data[i].camera = camera;
        thread_data[i].options = options;
        thread_data[i].buffer = buffer;
        thread_data[i].tile_index = &tile_index;
        thread_data[i].tiles_x = tiles_x;
        thread_data[i].tiles_y = tiles_y;
        pthread_create(&threads[i], NULL, render_thread, &thread_data[i]);
    }
    
    for (int i = 0; i < options->num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    char hdr_filename[256];
    snprintf(hdr_filename, sizeof(hdr_filename), "%s.hdr", options->output_filename);
    if (stbi_write_hdr(hdr_filename, width, height, 3, buffer)) {
        printf("Saved %s\n", hdr_filename);
    } else {
        printf("Error saving HDR\n");
    }
    
    char png_filename[256];
    snprintf(png_filename, sizeof(png_filename), "%s.png", options->output_filename);
    unsigned char* png_data = (unsigned char*)malloc(width * height * 3);
    for (int i = 0; i < width * height * 3; i++) {
        float val = buffer[i];
        val = val / (1.0f + val);
        val = powf(val, 1.0f / 2.2f);
        png_data[i] = (unsigned char)(fminf(val, 1.0f) * 255.0f);
    }
    stbi_write_png(png_filename, width, height, 3, png_data, width * 3);
    printf("Saved %s\n", png_filename);
    
    free(threads);
    free(thread_data);
    free(buffer);
    free(png_data);
}

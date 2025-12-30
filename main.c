#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "renderer.h"
#include "scene.h"
#include "camera.h"

void print_usage(const char* prog) {
    printf("Usage: %s [options]\n", prog);
    printf("  --spp <n>       Samples per pixel (default: 16)\n");
    printf("  --threads <n>   Number of threads (default: 4)\n");
    printf("  --output <file> Output filename (default: output.exr)\n");
    printf("  --scene <n>     Scene ID (0: Cornell Box) (default: 0)\n");
    printf("  --bounces <n>   Max bounces (default: 4)\n");
}

int main(int argc, char** argv) {
    RenderOptions options = {
        .width = 512,
        .height = 512,
        .samples_per_pixel = 16,
        .max_bounces = 4,
        .num_threads = 4,
        .output_filename = "output.exr"
    };
    
    int scene_id = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--spp") == 0 && i + 1 < argc) options.samples_per_pixel = atoi(argv[++i]);
        else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) options.num_threads = atoi(argv[++i]);
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) options.output_filename = argv[++i];
        else if (strcmp(argv[i], "--scene") == 0 && i + 1 < argc) scene_id = atoi(argv[++i]);
        else if (strcmp(argv[i], "--bounces") == 0 && i + 1 < argc) options.max_bounces = atoi(argv[++i]);
        else { print_usage(argv[0]); return 1; }
    }
    
    Scene scene;
    scene_init(&scene);
    Camera camera;
    
    if (scene_id == 0) {
        Material white = { .base_color = {0.73f, 0.73f, 0.73f}, .roughness = 0.5f };
        Material red = { .base_color = {0.65f, 0.05f, 0.05f}, .roughness = 0.5f };
        Material green = { .base_color = {0.12f, 0.45f, 0.15f}, .roughness = 0.5f };
        Material light_mat = { .base_color = {1,1,1}, .emission = {15, 15, 15} };
        
        scene_add_material(&scene, white);
        scene_add_material(&scene, red);
        scene_add_material(&scene, green);
        scene_add_material(&scene, light_mat);
        
        Triangle t1 = { {-2,-2,0}, {2,-2,0}, {2,2,0}, {0,0,1},{0,0,1},{0,0,1}, 0 };
        Triangle t2 = { {-2,-2,0}, {2,2,0}, {-2,2,0}, {0,0,1},{0,0,1},{0,0,1}, 0 };
        Triangle t3 = { {-2,-2,4}, {2,2,4}, {2,-2,4}, {0,0,-1},{0,0,-1},{0,0,-1}, 0 };
        Triangle t4 = { {-2,-2,4}, {-2,2,4}, {2,2,4}, {0,0,-1},{0,0,-1},{0,0,-1}, 0 };
        Triangle t5 = { {-2,2,0}, {2,2,0}, {2,2,4}, {0,-1,0},{0,-1,0},{0,-1,0}, 0 };
        Triangle t6 = { {-2,2,0}, {2,2,4}, {-2,2,4}, {0,-1,0},{0,-1,0},{0,-1,0}, 0 };
        Triangle t7 = { {-2,-2,0}, {-2,2,0}, {-2,2,4}, {1,0,0},{1,0,0},{1,0,0}, 1 };
        Triangle t8 = { {-2,-2,0}, {-2,2,4}, {-2,-2,4}, {1,0,0},{1,0,0},{1,0,0}, 1 };
        Triangle t9 = { {2,-2,0}, {2,-2,4}, {2,2,4}, {-1,0,0},{-1,0,0},{-1,0,0}, 2 };
        Triangle t10 = { {2,-2,0}, {2,2,4}, {2,2,0}, {-1,0,0},{-1,0,0},{-1,0,0}, 2 };
        
        Light light = { .type = LIGHT_QUAD, .position = {-0.5f, -0.5f, 3.9f}, .u = {1,0,0}, .v = {0,1,0}, .emission = {15,15,15} };
        scene_add_light(&scene, light);
        
        Triangle l1 = { {-0.5f,-0.5f,3.9f}, {0.5f,-0.5f,3.9f}, {0.5f,0.5f,3.9f}, {0,0,-1},{0,0,-1},{0,0,-1}, 3 };
        Triangle l2 = { {-0.5f,-0.5f,3.9f}, {0.5f,0.5f,3.9f}, {-0.5f,0.5f,3.9f}, {0,0,-1},{0,0,-1},{0,0,-1}, 3 };
        
        scene.triangles = malloc(sizeof(Triangle) * 12);
        scene.triangles[0] = t1; scene.triangles[1] = t2;
        scene.triangles[2] = t3; scene.triangles[3] = t4;
        scene.triangles[4] = t5; scene.triangles[5] = t6;
        scene.triangles[6] = t7; scene.triangles[7] = t8;
        scene.triangles[8] = t9; scene.triangles[9] = t10;
        scene.triangles[10] = l1; scene.triangles[11] = l2;
        scene.tri_count = 12;
        
        scene_build(&scene);
        camera_init(&camera, (Vec3){0, -8, 2}, (Vec3){0, 0, 2}, (Vec3){0, 0, 1}, 40.0f, (float)options.width/options.height, 0.0f, 10.0f);
    }
    
    render(&scene, &camera, &options);
    scene_free(&scene);
    return 0;
}

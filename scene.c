#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void scene_init(Scene* scene) {
    memset(scene, 0, sizeof(Scene));
}

void scene_add_material(Scene* scene, Material material) {
    scene->material_count++;
    scene->materials = realloc(scene->materials, sizeof(Material) * scene->material_count);
    scene->materials[scene->material_count - 1] = material;
}

void scene_add_light(Scene* scene, Light light) {
    scene->light_count++;
    scene->lights = realloc(scene->lights, sizeof(Light) * scene->light_count);
    scene->lights[scene->light_count - 1] = light;
}

void scene_load_obj(Scene* scene, const char* filename, int material_id) {
    FILE* f = fopen(filename, "r");
    if (!f) return;
    
    Vec3* temp_verts = NULL;
    int v_count = 0;
    Vec3* temp_normals = NULL;
    int vn_count = 0;
    char line[1024];
    
    while (fgets(line, 1024, f)) {
        if (line[0] == 'v' && line[1] == ' ') {
            Vec3 v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            v_count++;
            temp_verts = realloc(temp_verts, sizeof(Vec3) * v_count);
            temp_verts[v_count - 1] = v;
        } else if (line[0] == 'v' && line[1] == 'n') {
            Vec3 vn;
            sscanf(line, "vn %f %f %f", &vn.x, &vn.y, &vn.z);
            vn_count++;
            temp_normals = realloc(temp_normals, sizeof(Vec3) * vn_count);
            temp_normals[vn_count - 1] = vn;
        } else if (line[0] == 'f') {
            char* p = line + 2;
            int vertices[3] = {0};
            int normals[3] = {0};
            
            for(int i=0; i<3; ++i) {
                while(*p == ' ') p++;
                vertices[i] = atoi(p);
                while(*p != ' ' && *p != '/' && *p != '\0') p++;
                if (*p == '/') {
                    p++;
                    if (*p == '/') {
                        p++;
                        normals[i] = atoi(p);
                        while(*p != ' ' && *p != '\0') p++;
                    } else {
                        while(*p != ' ' && *p != '/' && *p != '\0') p++;
                        if (*p == '/') {
                            p++;
                            normals[i] = atoi(p);
                            while(*p != ' ' && *p != '\0') p++;
                        }
                    }
                }
            }
            
            scene->tri_count++;
            scene->triangles = realloc(scene->triangles, sizeof(Triangle) * scene->tri_count);
            Triangle* t = &scene->triangles[scene->tri_count - 1];
            t->v0 = temp_verts[vertices[0] - 1];
            t->v1 = temp_verts[vertices[1] - 1];
            t->v2 = temp_verts[vertices[2] - 1];
            
            if (normals[0] > 0 && vn_count > 0) {
                t->n0 = temp_normals[normals[0] - 1];
                t->n1 = temp_normals[normals[1] - 1];
                t->n2 = temp_normals[normals[2] - 1];
            } else {
                Vec3 e1 = vec3_sub(t->v1, t->v0);
                Vec3 e2 = vec3_sub(t->v2, t->v0);
                Vec3 n = vec3_normalize(vec3_cross(e1, e2));
                t->n0 = t->n1 = t->n2 = n;
            }
            t->material_id = material_id;
        }
    }
    free(temp_verts);
    free(temp_normals);
    fclose(f);
}

void scene_build(Scene* scene) {
    bvh_build(&scene->bvh, scene->triangles, scene->tri_count);
}

void scene_free(Scene* scene) {
    bvh_free(&scene->bvh);
    free(scene->triangles);
    free(scene->materials);
    free(scene->lights);
}

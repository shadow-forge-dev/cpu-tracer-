#ifndef SCENE_H
#define SCENE_H

#include "types.h"
#include "bvh.h"
#include "material.h"
#include "light.h"

typedef struct {
    BVH bvh;
    Triangle* triangles;
    int tri_count;
    Material* materials;
    int material_count;
    Light* lights;
    int light_count;
} Scene;

void scene_init(Scene* scene);
void scene_load_obj(Scene* scene, const char* filename, int material_id);
void scene_add_light(Scene* scene, Light light);
void scene_add_material(Scene* scene, Material material);
void scene_build(Scene* scene);
void scene_free(Scene* scene);

#endif

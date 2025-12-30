#ifndef BVH_H
#define BVH_H

#include "types.h"
#include "ray.h"

typedef struct {
    Vec3 v0, v1, v2;
    Vec3 n0, n1, n2;
    int material_id;
} Triangle;

typedef struct { Vec3 min, max; } AABB;

typedef struct BVHNode {
    AABB bounds;
    struct BVHNode* left;
    struct BVHNode* right;
    int first_prim_offset;
    int prim_count;
    int axis;
} BVHNode;

typedef struct {
    BVHNode* nodes;
    int* prim_indices;
    Triangle* triangles;
    int node_count;
    int tri_count;
} BVH;

void bvh_build(BVH* bvh, Triangle* triangles, int count);
bool bvh_intersect(const BVH* bvh, Ray r, float t_min, float t_max, float* t, int* tri_index, float* u, float* v);
void bvh_free(BVH* bvh);

#endif

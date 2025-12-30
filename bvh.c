#include "bvh.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_PRIMS_PER_LEAF 4

static AABB aabb_union(AABB a, AABB b) {
    return (AABB){vec3_min(a.min, b.min), vec3_max(a.max, b.max)};
}

static AABB triangle_bounds(Triangle t) {
    Vec3 min = vec3_min(t.v0, vec3_min(t.v1, t.v2));
    Vec3 max = vec3_max(t.v0, vec3_max(t.v1, t.v2));
    return (AABB){min, max};
}

typedef struct {
    int index;
    AABB bounds;
    Vec3 centroid;
} PrimInfo;

static int partition(PrimInfo* prims, int start, int end, int axis, float mid) {
    int i = start;
    int j = end - 1;
    while (i <= j) {
        float c = (axis == 0) ? prims[i].centroid.x : ((axis == 1) ? prims[i].centroid.y : prims[i].centroid.z);
        if (c < mid) i++;
        else {
            PrimInfo temp = prims[i];
            prims[i] = prims[j];
            prims[j] = temp;
            j--;
        }
    }
    return i;
}

static BVHNode* build_recursive(PrimInfo* prims, int start, int end, int* total_nodes, Triangle* triangles, int* indices) {
    (*total_nodes)++;
    BVHNode* node = (BVHNode*)calloc(1, sizeof(BVHNode));
    
    AABB bounds = prims[start].bounds;
    for (int i = start + 1; i < end; i++) bounds = aabb_union(bounds, prims[i].bounds);
    node->bounds = bounds;
    
    int count = end - start;
    if (count <= MAX_PRIMS_PER_LEAF) {
        node->first_prim_offset = start;
        node->prim_count = count;
        for(int i=start; i<end; ++i) indices[i] = prims[i].index;
        return node;
    }
    
    Vec3 extent = vec3_sub(bounds.max, bounds.min);
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > (axis == 0 ? extent.x : extent.y)) axis = 2;
    
    float mid = (axis == 0) ? (bounds.min.x + bounds.max.x) * 0.5f : 
                ((axis == 1) ? (bounds.min.y + bounds.max.y) * 0.5f : 
                               (bounds.min.z + bounds.max.z) * 0.5f);
                               
    int split = partition(prims, start, end, axis, mid);
    
    if (split == start || split == end) {
        node->first_prim_offset = start;
        node->prim_count = count;
        for(int i=start; i<end; ++i) indices[i] = prims[i].index;
        return node;
    }
    
    node->axis = axis;
    node->left = build_recursive(prims, start, split, total_nodes, triangles, indices);
    node->right = build_recursive(prims, split, end, total_nodes, triangles, indices);
    return node;
}

void bvh_build(BVH* bvh, Triangle* triangles, int count) {
    bvh->triangles = triangles;
    bvh->tri_count = count;
    bvh->prim_indices = (int*)malloc(sizeof(int) * count);

    PrimInfo* prims = (PrimInfo*)malloc(sizeof(PrimInfo) * count);
    for (int i = 0; i < count; i++) {
        prims[i].index = i;
        prims[i].bounds = triangle_bounds(triangles[i]);
        prims[i].centroid = vec3_scale(vec3_add(triangles[i].v0, vec3_add(triangles[i].v1, triangles[i].v2)), 1.0f/3.0f);
    }
    
    int total_nodes = 0;
    bvh->nodes = build_recursive(prims, 0, count, &total_nodes, triangles, bvh->prim_indices);
    bvh->node_count = total_nodes;
    free(prims);
}

static bool intersect_aabb(AABB bounds, Ray r, float t_min, float t_max) {
    float t0 = t_min, t1 = t_max;
    for (int i = 0; i < 3; i++) {
        float invD = 1.0f / ((float*)&r.direction)[i];
        float tNear = (((float*)&bounds.min)[i] - ((float*)&r.origin)[i]) * invD;
        float tFar = (((float*)&bounds.max)[i] - ((float*)&r.origin)[i]) * invD;
        if (invD < 0.0f) { float tmp = tNear; tNear = tFar; tFar = tmp; }
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t1 <= t0) return false;
    }
    return true;
}

static bool intersect_triangle(Triangle tri, Ray r, float t_min, float t_max, float* t, float* u, float* v) {
    Vec3 e1 = vec3_sub(tri.v1, tri.v0);
    Vec3 e2 = vec3_sub(tri.v2, tri.v0);
    Vec3 h = vec3_cross(r.direction, e2);
    float a = vec3_dot(e1, h);
    if (a > -EPSILON && a < EPSILON) return false;
    
    float f = 1.0f / a;
    Vec3 s = vec3_sub(r.origin, tri.v0);
    *u = f * vec3_dot(s, h);
    if (*u < 0.0f || *u > 1.0f) return false;
    
    Vec3 q = vec3_cross(s, e1);
    *v = f * vec3_dot(r.direction, q);
    if (*v < 0.0f || *u + *v > 1.0f) return false;
    
    float temp_t = f * vec3_dot(e2, q);
    if (temp_t > t_min && temp_t < t_max) {
        *t = temp_t;
        return true;
    }
    return false;
}

bool bvh_intersect(const BVH* bvh, Ray r, float t_min, float t_max, float* t, int* tri_index, float* u, float* v) {
    bool hit = false;
    float closest_t = t_max;
    const BVHNode* stack[64];
    int stack_ptr = 0;
    stack[stack_ptr++] = bvh->nodes;
    
    while (stack_ptr > 0) {
        const BVHNode* node = stack[--stack_ptr];
        if (!intersect_aabb(node->bounds, r, t_min, closest_t)) continue;
        
        if (node->prim_count > 0) {
            for (int i = 0; i < node->prim_count; i++) {
                int idx = bvh->prim_indices[node->first_prim_offset + i];
                float temp_u, temp_v, temp_t;
                if (intersect_triangle(bvh->triangles[idx], r, t_min, closest_t, &temp_t, &temp_u, &temp_v)) {
                    closest_t = temp_t;
                    *t = closest_t;
                    *u = temp_u;
                    *v = temp_v;
                    *tri_index = idx;
                    hit = true;
                }
            }
        } else {
            stack[stack_ptr++] = node->left;
            stack[stack_ptr++] = node->right;
        }
    }
    return hit;
}

void bvh_free(BVH* bvh) {
    free(bvh->prim_indices);
}

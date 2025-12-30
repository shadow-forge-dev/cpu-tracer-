#ifndef VEC3_H
#define VEC3_H

#include "types.h"
#include <math.h>

static inline Vec3 vec3_create(float x, float y, float z) { return (Vec3){x, y, z}; }
static inline Vec3 vec3_add(Vec3 a, Vec3 b) { return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline Vec3 vec3_sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline Vec3 vec3_mul(Vec3 a, Vec3 b) { return (Vec3){a.x * b.x, a.y * b.y, a.z * b.z}; }
static inline Vec3 vec3_scale(Vec3 a, float s) { return (Vec3){a.x * s, a.y * s, a.z * s}; }
static inline float vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
static inline float vec3_length_sq(Vec3 a) { return vec3_dot(a, a); }
static inline float vec3_length(Vec3 a) { return sqrtf(vec3_length_sq(a)); }
static inline Vec3 vec3_normalize(Vec3 a) {
    float len = vec3_length(a);
    return len > 0 ? vec3_scale(a, 1.0f / len) : a;
}
static inline Vec3 vec3_min(Vec3 a, Vec3 b) { return (Vec3){fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z)}; }
static inline Vec3 vec3_max(Vec3 a, Vec3 b) { return (Vec3){fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z)}; }
static inline Vec3 vec3_reflect(Vec3 v, Vec3 n) {
    return vec3_sub(v, vec3_scale(n, 2.0f * vec3_dot(v, n)));
}
static inline bool vec3_refract(Vec3 v, Vec3 n, float ni_over_nt, Vec3* refracted) {
    Vec3 uv = vec3_normalize(v);
    float dt = vec3_dot(uv, n);
    float d = 1.0f - ni_over_nt * ni_over_nt * (1.0f - dt * dt);
    if (d > 0) {
        *refracted = vec3_sub(vec3_scale(vec3_sub(uv, vec3_scale(n, dt)), ni_over_nt), vec3_scale(n, sqrtf(d)));
        return true;
    }
    return false;
}

void vec3_coordinate_system(Vec3 n, Vec3* s, Vec3* t);

#endif

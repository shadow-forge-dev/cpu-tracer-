#ifndef RAY_H
#define RAY_H

#include "vec3.h"

static inline Vec3 ray_at(Ray r, float t) {
    return vec3_add(r.origin, vec3_scale(r.direction, t));
}

#endif

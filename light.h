#ifndef LIGHT_H
#define LIGHT_H

#include "types.h"
#include "vec3.h"
#include "sampler.h"

typedef enum { LIGHT_QUAD, LIGHT_SPHERE, LIGHT_DISK, LIGHT_ENV } LightType;

typedef struct {
    LightType type;
    Vec3 position;
    Vec3 u, v;
    float radius;
    Vec3 emission;
} Light;

Vec3 light_sample(const Light* light, Vec3 p, Sampler* sampler, Vec3* wi, float* pdf, float* dist);
float light_pdf(const Light* light, Vec3 p, Vec3 wi);

#endif

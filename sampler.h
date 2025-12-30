#ifndef SAMPLER_H
#define SAMPLER_H

#include "types.h"
#include "vec3.h"

typedef struct {
    uint64_t state;
    uint64_t inc;
} Sampler;

void sampler_init(Sampler* s, uint64_t seed, uint64_t seq);
float sampler_next_1d(Sampler* s);
Vec3 sampler_next_2d(Sampler* s);

Vec3 sample_cosine_hemisphere(float r1, float r2);
Vec3 sample_uniform_sphere(float r1, float r2);
Vec3 sample_uniform_hemisphere(float r1, float r2);
Vec3 sample_disk(float r1, float r2);

#endif

#include "sampler.h"

void sampler_init(Sampler* s, uint64_t seed, uint64_t seq) {
    s->state = 0U;
    s->inc = (seq << 1u) | 1u;
    sampler_next_1d(s);
    s->state += seed;
    sampler_next_1d(s);
}

float sampler_next_1d(Sampler* s) {
    uint64_t oldstate = s->state;
    s->state = oldstate * 6364136223846793005ULL + s->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    uint32_t result = (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    return (float)result * (1.0f / 4294967296.0f);
}

Vec3 sampler_next_2d(Sampler* s) {
    return (Vec3){sampler_next_1d(s), sampler_next_1d(s), 0.0f};
}

Vec3 sample_cosine_hemisphere(float r1, float r2) {
    float phi = 2.0f * PI * r1;
    float r = sqrtf(r2);
    float x = r * cosf(phi);
    float y = r * sinf(phi);
    float z = sqrtf(fmaxf(0.0f, 1.0f - x*x - y*y));
    return (Vec3){x, y, z};
}

Vec3 sample_uniform_sphere(float r1, float r2) {
    float z = 1.0f - 2.0f * r1;
    float r = sqrtf(fmaxf(0.0f, 1.0f - z*z));
    float phi = 2.0f * PI * r2;
    return (Vec3){r * cosf(phi), r * sinf(phi), z};
}

Vec3 sample_uniform_hemisphere(float r1, float r2) {
    float z = r1;
    float r = sqrtf(fmaxf(0.0f, 1.0f - z*z));
    float phi = 2.0f * PI * r2;
    return (Vec3){r * cosf(phi), r * sinf(phi), z};
}

Vec3 sample_disk(float r1, float r2) {
    float r = sqrtf(r1);
    float phi = 2.0f * PI * r2;
    return (Vec3){r * cosf(phi), r * sinf(phi), 0.0f};
}

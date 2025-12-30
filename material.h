#ifndef MATERIAL_H
#define MATERIAL_H

#include "types.h"
#include "vec3.h"
#include "sampler.h"

typedef struct {
    Vec3 base_color;
    float metallic;
    float roughness;
    float specular;
    float specular_tint;
    float sheen;
    float sheen_tint;
    float clearcoat;
    float clearcoat_gloss;
    float ior;
    float transmission;
    Vec3 emission;
} Material;

Vec3 material_eval(const Material* mat, Vec3 wo, Vec3 wi, Vec3 n, Vec3 s, Vec3 t);
float material_pdf(const Material* mat, Vec3 wo, Vec3 wi, Vec3 n, Vec3 s, Vec3 t);
Vec3 material_sample(const Material* mat, Vec3 wo, Vec3* wi, Vec3 n, Vec3 s, Vec3 t, Sampler* sampler, float* pdf);

#endif

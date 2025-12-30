#include "light.h"

Vec3 light_sample(const Light* light, Vec3 p, Sampler* sampler, Vec3* wi, float* pdf, float* dist) {
    if (light->type == LIGHT_QUAD) {
        float r1 = sampler_next_1d(sampler);
        float r2 = sampler_next_1d(sampler);
        Vec3 point_on_light = vec3_add(light->position, vec3_add(vec3_scale(light->u, r1), vec3_scale(light->v, r2)));
        
        Vec3 to_light = vec3_sub(point_on_light, p);
        float d2 = vec3_length_sq(to_light);
        *dist = sqrtf(d2);
        *wi = vec3_scale(to_light, 1.0f / *dist);
        
        Vec3 normal = vec3_normalize(vec3_cross(light->u, light->v));
        float area = vec3_length(vec3_cross(light->u, light->v));
        float cosine = fabsf(vec3_dot(normal, vec3_scale(*wi, -1.0f)));
        
        *pdf = d2 / (area * cosine);
        return light->emission;
    } else if (light->type == LIGHT_SPHERE) {
        Vec3 point_on_sphere = vec3_add(light->position, vec3_scale(sample_uniform_sphere(sampler_next_1d(sampler), sampler_next_1d(sampler)), light->radius));
        Vec3 to_light = vec3_sub(point_on_sphere, p);
        float d2 = vec3_length_sq(to_light);
        *dist = sqrtf(d2);
        *wi = vec3_scale(to_light, 1.0f / *dist);
        
        float area = 4.0f * PI * light->radius * light->radius;
        Vec3 normal = vec3_normalize(vec3_sub(point_on_sphere, light->position));
        float cosine = fabsf(vec3_dot(normal, vec3_scale(*wi, -1.0f)));
        
        *pdf = d2 / (area * cosine);
        return light->emission;
    }
    return (Vec3){0};
}

float light_pdf(const Light* light, Vec3 p, Vec3 wi) {
    (void)light; (void)p; (void)wi;
    return 0.0f;
}

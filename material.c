#include "material.h"

static float sqr(float x) { return x * x; }
static float mix(float a, float b, float t) { return a * (1.0f - t) + b * t; }
static float schlick_weight(float cos_theta) {
    float m = 1.0f - cos_theta;
    float m2 = m * m;
    return m2 * m2 * m;
}

static float gtr1(float ndoth, float a) {
    if (a >= 1.0f) return INV_PI;
    float a2 = a * a;
    float t = 1.0f + (a2 - 1.0f) * ndoth * ndoth;
    return (a2 - 1.0f) / (PI * logf(a2) * t);
}

static float gtr2(float ndoth, float a) {
    float a2 = a * a;
    float t = 1.0f + (a2 - 1.0f) * ndoth * ndoth;
    return a2 / (PI * t * t);
}

static float smith_g_ggx(float ndotv, float alpha_g) {
    float a = alpha_g * alpha_g;
    float b = ndotv * ndotv;
    return 1.0f / (ndotv + sqrtf(a + b - a * b));
}

Vec3 material_eval(const Material* mat, Vec3 wo, Vec3 wi, Vec3 n, Vec3 s, Vec3 t_vec) {
    (void)s; (void)t_vec;
    float ndotl = vec3_dot(n, wi);
    float ndotv = vec3_dot(n, wo);
    if (ndotl <= 0.0f || ndotv <= 0.0f) return (Vec3){0};

    Vec3 h = vec3_normalize(vec3_add(wo, wi));
    float ndoth = vec3_dot(n, h);
    float ldoth = vec3_dot(wi, h);

    float fd90 = 0.5f + 2.0f * mat->roughness * ldoth * ldoth;
    float light_scatter = 1.0f + (fd90 - 1.0f) * schlick_weight(ndotl);
    float view_scatter = 1.0f + (fd90 - 1.0f) * schlick_weight(ndotv);
    float diffuse = light_scatter * view_scatter * INV_PI;
    
    float alpha = sqr(mat->roughness);
    float ds = gtr2(ndoth, alpha);
    float fh = schlick_weight(ldoth);
    Vec3 f0 = vec3_scale((Vec3){1,1,1}, mix(0.08f * mat->specular, 1.0f, mat->metallic));
    if (mat->metallic > 0.0f) {
        f0 = vec3_add(vec3_scale(f0, 1.0f - mat->metallic), vec3_scale(mat->base_color, mat->metallic));
    }
    
    Vec3 fs = vec3_add(f0, vec3_scale(vec3_sub((Vec3){1,1,1}, f0), fh));
    float gs = smith_g_ggx(ndotl, alpha) * smith_g_ggx(ndotv, alpha);
    Vec3 specular = vec3_scale(fs, ds * gs);

    float dr = gtr1(ndoth, mix(0.1f, 0.001f, mat->clearcoat_gloss));
    float fr = mix(0.04f, 1.0f, fh);
    float gr = smith_g_ggx(ndotl, 0.25f) * smith_g_ggx(ndotv, 0.25f);
    float clearcoat = 0.25f * mat->clearcoat * gr * fr * dr;

    Vec3 result = vec3_scale(mat->base_color, diffuse * (1.0f - mat->metallic) * (1.0f - mat->transmission));
    result = vec3_add(result, specular);
    result = vec3_add(result, (Vec3){clearcoat, clearcoat, clearcoat});
    
    return vec3_scale(result, ndotl);
}

float material_pdf(const Material* mat, Vec3 wo, Vec3 wi, Vec3 n, Vec3 s, Vec3 t_vec) {
    (void)s; (void)t_vec;
    float ndotl = vec3_dot(n, wi);
    if (ndotl <= 0.0f) return 0.0f;
    
    Vec3 h = vec3_normalize(vec3_add(wo, wi));
    float ndoth = vec3_dot(n, h);
    float ldoth = vec3_dot(wi, h);
    
    float alpha = sqr(mat->roughness);
    float ds = gtr2(ndoth, alpha);
    float pdf_spec = ds * ndoth / (4.0f * ldoth);
    float pdf_diff = ndotl * INV_PI;
    
    float w_diff = (1.0f - mat->metallic) * (1.0f - mat->transmission);
    float w_spec = 1.0f;
    return (w_diff * pdf_diff + w_spec * pdf_spec) / (w_diff + w_spec);
}

Vec3 material_sample(const Material* mat, Vec3 wo, Vec3* wi, Vec3 n, Vec3 s, Vec3 t_vec, Sampler* sampler, float* pdf) {
    float r1 = sampler_next_1d(sampler);
    float r2 = sampler_next_1d(sampler);
    float rnd = sampler_next_1d(sampler);
    
    float p_spec = 1.0f / (2.0f - mat->metallic);
    float p_diff = 1.0f - p_spec;
    
    if (mat->transmission > 0.0f) {
        float ni_over_nt = 1.0f / mat->ior;
        if (vec3_dot(n, wo) < 0.0f) {
            n = vec3_scale(n, -1.0f);
            ni_over_nt = mat->ior;
        }
        
        Vec3 refracted;
        float cosine = vec3_dot(wo, n);
        float r0 = (1.0f - mat->ior) / (1.0f + mat->ior);
        r0 = r0 * r0;
        float reflect_prob = r0 + (1.0f - r0) * powf(1.0f - cosine, 5.0f);
        
        if (sampler_next_1d(sampler) < reflect_prob) {
            *wi = vec3_reflect(vec3_scale(wo, -1.0f), n);
            *pdf = reflect_prob;
        } else {
            if (vec3_refract(vec3_scale(wo, -1.0f), n, ni_over_nt, &refracted)) {
                *wi = refracted;
                *pdf = 1.0f - reflect_prob;
            } else {
                *wi = vec3_reflect(vec3_scale(wo, -1.0f), n);
                *pdf = 1.0f;
            }
        }
        return mat->base_color;
    }
    
    if (rnd < p_diff && mat->metallic < 1.0f) {
        *wi = sample_cosine_hemisphere(r1, r2);
        *wi = vec3_add(vec3_scale(s, wi->x), vec3_add(vec3_scale(t_vec, wi->y), vec3_scale(n, wi->z)));
        *pdf = material_pdf(mat, wo, *wi, n, s, t_vec);
    } else {
        float alpha = sqr(mat->roughness);
        float phi = 2.0f * PI * r1;
        float cos_theta = sqrtf((1.0f - r2) / (1.0f + (alpha*alpha - 1.0f) * r2));
        float sin_theta = sqrtf(1.0f - cos_theta*cos_theta);
        
        Vec3 h_local = {sin_theta * cosf(phi), sin_theta * sinf(phi), cos_theta};
        Vec3 h = vec3_add(vec3_scale(s, h_local.x), vec3_add(vec3_scale(t_vec, h_local.y), vec3_scale(n, h_local.z)));
        
        *wi = vec3_reflect(vec3_scale(wo, -1.0f), h);
        if (vec3_dot(*wi, n) <= 0.0f) {
            *pdf = 0.0f;
            return (Vec3){0};
        }
        *pdf = material_pdf(mat, wo, *wi, n, s, t_vec);
    }
    return material_eval(mat, wo, *wi, n, s, t_vec);
}

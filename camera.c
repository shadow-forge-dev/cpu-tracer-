#include "camera.h"

void camera_init(Camera* cam, Vec3 lookfrom, Vec3 lookat, Vec3 vup, float vfov, float aspect, float aperture, float focus_dist) {
    cam->lens_radius = aperture / 2.0f;
    float theta = vfov * PI / 180.0f;
    float half_height = tanf(theta / 2.0f);
    float half_width = aspect * half_height;
    
    cam->origin = lookfrom;
    cam->w = vec3_normalize(vec3_sub(lookfrom, lookat));
    cam->u = vec3_normalize(vec3_cross(vup, cam->w));
    cam->v = vec3_cross(cam->w, cam->u);
    
    cam->lower_left_corner = vec3_sub(cam->origin, vec3_scale(cam->u, half_width * focus_dist));
    cam->lower_left_corner = vec3_sub(cam->lower_left_corner, vec3_scale(cam->v, half_height * focus_dist));
    cam->lower_left_corner = vec3_sub(cam->lower_left_corner, vec3_scale(cam->w, focus_dist));
    
    cam->horizontal = vec3_scale(cam->u, 2.0f * half_width * focus_dist);
    cam->vertical = vec3_scale(cam->v, 2.0f * half_height * focus_dist);
}

Ray camera_get_ray(const Camera* cam, float s, float t, Sampler* sampler) {
    Vec3 rd = vec3_scale(sample_disk(sampler_next_1d(sampler), sampler_next_1d(sampler)), cam->lens_radius);
    Vec3 offset = vec3_add(vec3_scale(cam->u, rd.x), vec3_scale(cam->v, rd.y));
    Vec3 target = vec3_add(cam->lower_left_corner, vec3_scale(cam->horizontal, s));
    target = vec3_add(target, vec3_scale(cam->vertical, t));
    return (Ray){
        .origin = vec3_add(cam->origin, offset),
        .direction = vec3_normalize(vec3_sub(target, vec3_add(cam->origin, offset)))
    };
}

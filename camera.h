#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"
#include "ray.h"
#include "sampler.h"

typedef struct {
    Vec3 origin;
    Vec3 lower_left_corner;
    Vec3 horizontal;
    Vec3 vertical;
    Vec3 u, v, w;
    float lens_radius;
} Camera;

void camera_init(Camera* cam, Vec3 lookfrom, Vec3 lookat, Vec3 vup, float vfov, float aspect, float aperture, float focus_dist);
Ray camera_get_ray(const Camera* cam, float s, float t, Sampler* sampler);

#endif

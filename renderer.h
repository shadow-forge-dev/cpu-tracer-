#ifndef RENDERER_H
#define RENDERER_H

#include "scene.h"
#include "camera.h"

typedef struct {
    int width;
    int height;
    int samples_per_pixel;
    int max_bounces;
    int num_threads;
    const char* output_filename;
} RenderOptions;

void render(const Scene* scene, const Camera* camera, const RenderOptions* options);

#endif

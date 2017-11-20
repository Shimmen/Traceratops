#ifndef TRACERATOPS_RENDERING_H
#define TRACERATOPS_RENDERING_H

#include "image.h"
#include "scene.h"

struct hit_info
{
    tracemath::vec3 Point;
    tracemath::vec3 Normal;
    int             Material;
};

bool get_first_intersection(const scene& Scene, const ray& Ray, hit_info *Hit);
void render_scene(const scene& Scene, image& Image, int RaysPerPixel, int MaxRayDepth);
tracemath::vec3 trace_ray(ray Ray, const scene& Scene, tracemath::rng& Rng, int Depth);

uint32_t pixel_from_color(tracemath::vec3 color);

#endif // TRACERATOPS_RENDERING_H

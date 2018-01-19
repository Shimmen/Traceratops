#ifndef TRACERATOPS_RENDERING_H
#define TRACERATOPS_RENDERING_H

#include "image.h"
#include "scene.h"
#include "camera.h"

bool get_first_intersection(const scene& Scene, const ray& Ray, float MinT, float MaxT, hit_info *Hit);
void render_scene(const scene& Scene, const camera& Camera, image& Image, int RaysPerPixel, int MaxRayDepth);
vec3 trace_ray(ray Ray, const scene& Scene, rng& Rng, int Depth);

uint32_t pixel_from_color(vec3 color);

#endif // TRACERATOPS_RENDERING_H

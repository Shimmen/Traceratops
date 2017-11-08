#ifndef TRACERATOPS_RENDERING_H
#define TRACERATOPS_RENDERING_H

#include "image.h"
#include "scene.h"

void render_scene(scene *Scene, image *Image, int RaysPerPixel);
tracemath::vec3 trace_ray(ray *Ray, scene *Scene);

uint32_t pixel_from_color(tracemath::vec3 color);

#endif // TRACERATOPS_RENDERING_H

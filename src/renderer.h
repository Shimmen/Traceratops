#ifndef TRACERATOPS_RENDERING_H
#define TRACERATOPS_RENDERING_H

#include "image.h"
#include "scene.h"
#include "camera.h"

struct renderer
{
public:

    virtual void render_scene(const scene& Scene, const camera& Camera, image& Image) const = 0;

private:

    virtual bool get_first_intersection(const scene& Scene, const ray& Ray, float MinT, float MaxT, hit_info *Hit) const = 0;
    virtual vec3 trace_ray(ray Ray, const scene& Scene, rng& Rng) const = 0;

};

#endif // TRACERATOPS_RENDERING_H

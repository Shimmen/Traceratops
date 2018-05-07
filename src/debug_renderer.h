#ifndef TRACERATOPS_BASIC_RENDERER_H
#define TRACERATOPS_BASIC_RENDERER_H

#include "renderer.h"

struct debug_renderer: public renderer
{
public:

    debug_renderer() = default;
    virtual ~debug_renderer() = default;

    void render_scene(const scene& Scene, const camera& Camera, image& Image) const override;

private:

    bool get_first_intersection(const scene& Scene, const ray& Ray, float MinT, float MaxT, hit_info *Hit) const override;
    vec3 trace_ray(ray Ray, const scene& Scene, rng& Rng) const override;

    vec3 tone_map_hdr_to_ldr(const vec3& Hdr) const;

};

#endif // TRACERATOPS_BASIC_RENDERER_H

#ifndef TRACERATOPS_BASIC_RENDERER_H
#define TRACERATOPS_BASIC_RENDERER_H

#include "renderer.h"

struct basic_renderer: public renderer
{
public:

    basic_renderer(int RaysPerPixel, int MaxRayDepth);
    virtual ~basic_renderer() = default;

    void render_scene(const scene& Scene, const camera& Camera, image& Image) const override;

private:

    bool get_first_intersection(const scene& Scene, const ray& Ray, float MinT, float MaxT, hit_info *Hit) const override;
    vec3 trace_ray(ray Ray, const scene& Scene, rng& Rng) const override;

    ray get_light_ray(const vec3& Origin, const scene& Scene, rng& Rng, hitable **LightSource, float *MaxDistance) const;
    bool intersects_shadow_cache(const ray& Ray, float DistanceToLight, int CurrentRayDepth) const;
    void save_shadow_cache(const hitable *Hitable, int CurrentRayDepth) const;

    vec3 tone_map_hdr_to_ldr(const vec3& Hdr) const;

    mutable const hitable *ShadowCache = nullptr;
    mutable int CacheHits = 0;
    mutable int CacheTries = 0;

    int RaysPerPixel;
    int MaxRayDepth;

};

#endif //TRACERATOPS_BASIC_RENDERER_H

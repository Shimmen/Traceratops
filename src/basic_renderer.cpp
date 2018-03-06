#include "basic_renderer.h"

basic_renderer::basic_renderer(int RaysPerPixel, int MaxRayDepth)
    : RaysPerPixel(RaysPerPixel), MaxRayDepth(MaxRayDepth)
{

}

void basic_renderer::render_scene(const scene &Scene, const camera &Camera, image &Image) const
{
    if (!Scene.is_prepared_for_rendering())
    {
        printf("Scene not prepared for rendering! Call prepare_for_rendering() on scene before rendering. Aborting!\n");
        return;
    }

    rng Rng{};

    for (uint32_t y = 0; y < Image.Height; ++y)
    {
        for (uint32_t x = 0; x < Image.Width; ++x)
        {
            vec3 AccumulatedHdrColor = vec3{};
            for (int i = 0; i < RaysPerPixel; ++i)
            {
                // (jittered UVs)
                float u = (x + Rng.random_01()) / Image.Width;
                float v = (y + Rng.random_01()) / Image.Height;
                const ray& Ray = Camera.get_ray(u, v, Rng);

                vec3 Color = trace_ray(Ray, Scene, Rng, MaxRayDepth);
                AccumulatedHdrColor = AccumulatedHdrColor + Color;
            }

            AccumulatedHdrColor = AccumulatedHdrColor * (1.0f / RaysPerPixel);
            vec3 LdrColor = tone_map_hdr_to_ldr(AccumulatedHdrColor);

            uint32_t Pixel = encode_rgb_to_packed_pixel_int(LdrColor);
            Image.set_pixel(x, y, Pixel);
        }

        Image.update_window();

        float PercentDone = 100.0f * y / Image.Height;
        printf("... %f%% done ...\r", PercentDone);
        fflush(stdout);
    }

    printf("... 100%% done ...\n");
}

bool
basic_renderer::get_first_intersection(const scene& Scene, const ray& Ray, float MinT, float MaxT, hit_info *Hit) const
{
#if USE_NAIVE_INTERSECTION_TEST

    float MinDistance = INFINITY;
    hit_info CurrentHit{};

    for (auto& Hitable: Scene.Hitables)
    {
        if (Hitable->intersect(Ray, MinT, MaxT, CurrentHit))
        {
            if (CurrentHit.Distance <= MinDistance)
            {
                *Hit = CurrentHit;
                MinDistance = CurrentHit.Distance;
            }
        }
    }

    return MinDistance < INFINITY;

#else

    return Scene.BVHRootNode->intersect(Ray, MinT, MaxT, *Hit);

#endif
}

vec3
basic_renderer::trace_ray(ray Ray, const scene& Scene, rng& Rng, int Depth) const
{
    vec3 BounceAttenuation = vec3{1, 1, 1};
    vec3 ResultColor = vec3{0, 0, 0};

    hit_info Hit{};
    for (int CurrentDepth = 0; CurrentDepth < Depth; ++CurrentDepth)
    {
        if (get_first_intersection(Scene, Ray, 0.0001f, INFINITY, &Hit))
        {
            // NOTE: For debugging enable this for rendering normals!
            //return Hit.Normal * vec3(0.5) + vec3(0.5);

            const material& Material = Scene.get_material(Hit.Material);

            ray Scattered{};
            float PDF;

            if (Material.calculate_scattered(Ray, Hit, Rng, Scattered, PDF))
            {
                //
                // Accumulate contribution
                //

                ResultColor = ResultColor + (BounceAttenuation * Material.EmitColor);

                float CosineAttenuation = std::abs(dot(Scattered.Direction, Hit.Normal));
                vec3 BRDF = Material.brdf(Scattered.Direction, -Ray.Direction, Hit, Rng);
                BounceAttenuation = BounceAttenuation * BRDF * CosineAttenuation / PDF;

                // Continue from scattered
                Ray = Scattered;

            }
            else
            {
                //
                // Ray absorbed, kill this path
                //
                return vec3{0.0f};
            }
        }
        else
        {
            if (Scene.EnvironmentMap)
            {
                const vec3& Dir = Ray.Direction;

                // Convert direction to spherical coordinates
                float Theta = acosf(fmaxf(-1.0f, fminf(1.0f, Dir.y)));
                float Phi = atan2f(Dir.z, Dir.x);
                if (Phi < 0.0f) Phi += tracemath::TWO_PI;

                float u = Phi / tracemath::TWO_PI;
                float v = Theta / tracemath::PI;

                // (nearest might be okay here since we trace multiple rays per pixel)
                vec3 EnvironmentColor = Scene.EnvironmentMap->sample_texel_nearest(u, v);
                ResultColor = ResultColor + (BounceAttenuation * EnvironmentColor * Scene.EnvironmentMultiplier);
            }

            break;
        }
    }

    return ResultColor;
}

vec3
basic_renderer::tone_map_hdr_to_ldr(const vec3& Hdr) const
{
    // TODO: Use better tonemap!!!
    return Hdr / (Hdr + vec3(1, 1, 1));
}

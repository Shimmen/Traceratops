#include "debug_renderer.h"

void debug_renderer::render_scene(const scene &Scene, const camera &Camera, image &Image) const
{
    if (!Scene.is_prepared_for_rendering())
    {
        printf("Scene not prepared for rendering! Call prepare_for_rendering() on scene before rendering. Aborting!\n");
        return;
    }

    printf("-------------\n- rendering -\n-------------\n");

    rng Rng{};
    for (uint32_t y = 0; y < Image.Height; ++y)
    {
        for (uint32_t x = 0; x < Image.Width; ++x)
        {
            const int NumSamplesPerPixel = 1;
            vec3 AccumulatedHdrColor = vec3{};
            for (int i = 0; i < NumSamplesPerPixel; ++i)
            {
                const ray& Ray = Camera.get_ray(float(x) / float(Image.Width), float(y) / float(Image.Height), Rng);
                vec3 Color = trace_ray(Ray, Scene, Rng);
                AccumulatedHdrColor += Color;
            }

            AccumulatedHdrColor = AccumulatedHdrColor * (1.0f / float(NumSamplesPerPixel));
            vec3 LdrColor = tone_map_hdr_to_ldr(AccumulatedHdrColor);

            uint32_t Pixel = encode_rgb_to_packed_pixel_int(LdrColor);
            Image.set_pixel(x, y, Pixel);
        }

        Image.update_window();

        float PercentDone = 100.0f * y / Image.Height;
        printf("%f%% done\r", PercentDone);
        fflush(stdout);
    }

    printf("100%% done\n");
    printf("-------------\n");
}

bool
debug_renderer::get_first_intersection(const scene& Scene, const ray& Ray, float MinT, float MaxT, rng& Rng, hit_info *Hit) const
{
    return Scene.BVHRootNode->intersect(Ray, MinT, MaxT, *Hit, Rng);
}

vec3
debug_renderer::trace_ray(ray Ray, const scene& Scene, rng& Rng) const
{
    vec3 ResultColor = vec3{0, 0, 0};

    hit_info Hit{};
    if (get_first_intersection(Scene, Ray, 0.001f, INFINITY, Rng, &Hit))
    {
        const material& Material = Scene.get_material(Hit.Hitable->Material);

        ray Scattered{};
        float PDF;

        if (Material.calculate_scattered(Ray, Hit, Rng, Scattered, PDF))
        {
            float CosineAttenuation = std::max(0.0f, dot(Scattered.Direction, Hit.Normal));
            vec3 BRDF = Material.brdf(Scattered.Direction, -Ray.Direction, Hit, Rng);
            ResultColor += Material.EmitColor + BRDF * CosineAttenuation / PDF;
        }
    }

    return ResultColor;
}

vec3
debug_renderer::tone_map_hdr_to_ldr(const vec3& Hdr) const
{
    // TODO: Use better tonemap!!!
    return Hdr / (Hdr + vec3(1, 1, 1));
}

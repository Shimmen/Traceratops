#include <random>

#ifdef USE_OPENMP
#include <omp.h>
#else
#include <thread>
#include <map>
#endif

#include "rendering.h"
#include "aabb.h"

using namespace tracemath;

#if 0
void make_test_image(image *Image)
{
    for (uint32_t y = 0; y < Image->Height; ++y) {
        for (uint32_t x = 0; x < Image->Width; ++x) {

            uint32_t a = 0xFF000000;
            uint32_t b = ((x + y) % 0xFF) << 16;
            uint32_t g = (y % 0xFF) << 8;
            uint32_t r = (x % 0xFF) << 0;

            uint32_t PixelValue = r | g | b | a;
            Image->set_pixel(x, y, PixelValue);
        }
    }

    Image->set_pixel(0, 0, 0xFFFFFFFF);
    Image->set_pixel(0, Image->Height - 1, 0xFFFFFFFF);
    Image->set_pixel(Image->Width - 1, 0, 0xFFFFFFFF);
    Image->set_pixel(Image->Width - 1, Image->Height - 1, 0xFFFFFFFF);
}
#endif

vec3 tone_map_hdr_to_ldr(const vec3& Hdr)
{
    // TODO: Use better tonemap!!!
    return Hdr / (Hdr + vec3(1, 1, 1));
}

void render_scene(const scene& Scene, const camera& Camera, image& Image, int RaysPerPixel, int MaxRayDepth)
{
    if (!Scene.is_prepared_for_rendering())
    {
        printf("Scene not prepared for rendering! Call prepare_for_rendering() on scene before rendering. Aborting!\n");
        return;
    }

#define USE_MULTITHREADING 0
#if USE_MULTITHREADING

    // TODO: Fix this! I'm very confused and most probably I'm doing something very weird and wrong.

    const int TotalRowCount = Image.Height;
    std::atomic<int> RowsCompleted{0};

    int NumThreadsToUse = std::thread::hardware_concurrency();
    std::map<std::thread::id, rng> RngsForThread{};
    std::vector<std::thread> Threads;

    for (int ThreadIndex = 0; ThreadIndex < NumThreadsToUse; ++ThreadIndex)
    {
        Threads.emplace_back([&]() {

            const auto& ThreadId = std::this_thread::get_id();
            RngsForThread[ThreadId] = rng{ThreadId};

            for (int y; (y = RowsCompleted++) < TotalRowCount; /* empty */)
            {
                rng& CurrentRng = RngsForThread[std::this_thread::get_id()];

                for (int x = 0; x < Image.Width; ++x)
                {
                    vec3 AccumulatedHdrColor = vec3{};
                    for (int i = 0; i < RaysPerPixel; ++i)
                    {
                        // TODO: Make a proper camera with fov etc!
                        Ray.Origin = vec3(0, 0, -1);
                        Ray.Direction = get_jittered_primary_ray(Image, x, y, CurrentRng);

                        vec3 Color = trace_ray(Ray, Scene, CurrentRng, MaxRayDepth);
                        AccumulatedHdrColor = AccumulatedHdrColor + Color;
                    }

                    AccumulatedHdrColor = AccumulatedHdrColor * (1.0f / RaysPerPixel);
                    vec3 LdrColor = tone_map_hdr_to_ldr(AccumulatedHdrColor);

                    uint32_t Pixel = pixel_from_color(LdrColor);
                    Image.set_pixel(x, Image.Height - y - 1, Pixel);
                }

                float PercentDone = 100.0f * y / Image.Height;
                printf("... %f%% done ...\r", PercentDone);
                fflush(stdout);
            }

        });
    }

    for (auto& Thread: Threads)
    {
        Thread.join();
    }

#else

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

            uint32_t Pixel = pixel_from_color(LdrColor);
            Image.set_pixel(x, y, Pixel);
        }

        Image.update_window();

        float PercentDone = 100.0f * y / Image.Height;
        printf("... %f%% done ...\r", PercentDone);
        fflush(stdout);
    }

#endif

    printf("... 100%% done ...\n");
}

bool get_first_intersection_naive(const scene& Scene, const ray& Ray, float MinT, float MaxT, hit_info *Hit)
{
    constexpr float Infinity = std::numeric_limits<float>::infinity();
    float MinDistance = Infinity;
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

    return MinDistance < Infinity;
}

bool get_first_intersection(const scene& Scene, const ray& Ray, float MinT, float MaxT, hit_info *Hit)
{
    return Scene.BVHRootNode->intersect(Ray, MinT, MaxT, *Hit);
}

vec3 trace_ray(ray Ray, const scene& Scene, rng& Rng, int Depth)
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
                if (Phi < 0.0f) Phi += TWO_PI;

                float u = Phi / TWO_PI;
                float v = Theta / PI;

                // (nearest might be okay here since we trace multiple rays per pixel)
                vec3 EnvironmentColor = Scene.EnvironmentMap->sample_texel_nearest(u, v);
                ResultColor = ResultColor + (BounceAttenuation * EnvironmentColor * Scene.EnvironmentMultiplier);
            }

            break;
        }
    }

    return ResultColor;
}

uint32_t pixel_from_color(vec3 color)
{
    static const float gamma = 2.2f;
    static const float gamma_pow = 1.0f / gamma;

    uint32_t r = 0xFF & uint32_t(powf(color.x, gamma_pow) * 255.99f);
    uint32_t g = 0xFF & uint32_t(powf(color.y, gamma_pow) * 255.99f);
    uint32_t b = 0xFF & uint32_t(powf(color.z, gamma_pow) * 255.99f);
    uint32_t a = 0xFF;

    return (a << 24) | (b << 16) | (g << 8) | (r << 0);
}

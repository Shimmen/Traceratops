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

    //ray Ray = {};

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

            // (Debug corner pixels)
            //if (x == 0 && y == 0) Pixel = pixel_from_color(vec3{1, 0, 1});
            //if (x == Image.Width - 1 && y == Image.Height - 1) Pixel = pixel_from_color(vec3{0, 1, 1});

            Image.set_pixel(x, Image.Height - y - 1, Pixel);
        }

        float PercentDone = 100.0f * y / Image.Height;
        printf("... %f%% done ...\r", PercentDone);
        fflush(stdout);
    }

#endif

    printf("... 100%% done ...\n");
}

bool get_first_intersection(const scene& Scene, const ray& Ray, float MinT, float MaxT, hit_info *Hit)
{
    constexpr float Infinity = std::numeric_limits<float>::infinity();

    float MinDistance = Infinity;
    float Distance;

    for (auto& Plane : Scene.Planes)
    {
        if (plane_intersect(Plane, Ray, &Distance))
        {
            if (Distance > MinT && Distance < MinDistance && Distance < MaxT)
            {
                MinDistance = Distance;

                Hit->Point = Ray.Origin + (Ray.Direction * Distance);
                Hit->Normal = Plane.N;
                Hit->Material = Plane.Material;
            }
        }
    }

    for (auto& Disc : Scene.Discs)
    {
        if (plane_intersect(Disc.Plane, Ray, &Distance))
        {
            if (Distance > MinT && Distance < MinDistance && Distance < MaxT)
            {
                Hit->Point = Ray.Origin + (Ray.Direction * Distance);
                if (length2(Hit->Point - Disc.Plane.P) <= Disc.r * Disc.r)
                {
                    MinDistance = Distance;

                    Hit->Normal = Disc.Plane.N;
                    Hit->Material = Disc.Plane.Material;
                }
            }
        }
    }

    for (auto& Sphere : Scene.Spheres)
    {
        if (sphere_intersect(Sphere, Ray, &Distance))
        {
            if (Distance > MinT && Distance < MinDistance && Distance < MaxT)
            {
                MinDistance = Distance;

                Hit->Point = Ray.Origin + (Ray.Direction * Distance);
                Hit->Normal = normalize(Hit->Point - Sphere.P);
                Hit->Material = Sphere.Material;
            }
        }
    }

/*
    std::vector<size_t> TraverseStack{};
    TraverseStack.push_back(0); // index 0 is root node(?)

    while (!TraverseStack.empty())
    {
        // Depth-first traversal
        size_t CurrentIndex = TraverseStack.back();
        TraverseStack.pop_back();

        const aabb& Current = Scene.BVHElements[CurrentIndex];

        if (aabb_ray_intersection(Current, Ray.Direction, Ray.Origin))
        {
            // If is leaf node
            if (Current.Children.empty())
            {
                const auto& Triangles = Current.ContainedTriangles;
                for (const triangle_face *Triangle: Triangles)
                {
                    // From GraphicsCodex

                    const vec3& V0 = Triangle->Vertices[0];
                    const vec3& V1 = Triangle->Vertices[1];
                    const vec3& V2 = Triangle->Vertices[2];

                    const vec3& E1 = V1 - V0;
                    const vec3& E2 = V2 - V0;

                    vec3 N = cross(E1, E2);
                    normalize(&N);

                    vec3 q = cross(Ray.Direction, E2);
                    float a = dot(E1, q);

                    // (Nearly) parallel or backfacing, or close to the limit of precision?
                    if (dot(N, Ray.Direction) >= 0 || fabsf(a) <= 0.0001f)
                    {
                        continue;
                    }

                    const vec3& s = (Ray.Origin - V0) / a;
                    const vec3& r = cross(s, E1);

                    // Barycentric coordinates
                    float b[3];
                    b[0] = dot(s, q);
                    b[1] = dot(r, Ray.Direction);
                    b[2] = 1.0f - b[0] - b[1];

                    // Intersected inside triangle?
                    Distance = dot(E2, r);
                    if ((b[0] >= 0) && (b[1] >= 0) && (b[2] >= 0) && (Distance >= 0))
                    {
                        if (Distance < MinDistance)
                        {
                            MinDistance = Distance;

                            Hit->Point = Ray.Origin + (Ray.Direction * Distance);
                            Hit->Normal = N; // TODO: Use smooth face normal using barycentric coords!
                            Hit->Material = 0; // TODO: Use face material!
                        }
                    }
                }
            }
            else
            {
                for (size_t ChildIndex: Current.Children)
                {
                    TraverseStack.push_back(ChildIndex);
                }
            }
        }
    }
*/
/*
    auto& TriangleVertices = Scene.get_triangle_vertices();
    for (int i = 0; i < TriangleVertices.size(); i += 3)
    {
        // From GraphicsCodex

        const vec3& V0 = TriangleVertices[i + 0];
        const vec3& V1 = TriangleVertices[i + 1];
        const vec3& V2 = TriangleVertices[i + 2];

        const vec3& E1 = V1 - V0;
        const vec3& E2 = V2 - V0;

        vec3 N = cross(E1, E2);
        normalize(&N);

        vec3 q = cross(Ray.Direction, E2);
        float a = dot(E1, q);

        // (Nearly) parallel or backfacing, or close to the limit of precision?
        if (dot(N, Ray.Direction) >= 0 || fabsf(a) <= 0.0001f)
        {
            continue;
        }

        const vec3& s = (Ray.Origin - V0) / a;
        const vec3& r = cross(s, E1);

        // Barycentric coordinates
        float b[3];
        b[0] = dot(s, q);
        b[1] = dot(r, Ray.Direction);
        b[2] = 1.0f - b[0] - b[1];

        // Intersected inside triangle?
        Distance = dot(E2, r);
        if ((b[0] >= 0) && (b[1] >= 0) && (b[2] >= 0) && (Distance >= 0))
        {
            if (Distance < MinDistance)
            {
                MinDistance = Distance;

                Hit->Point = Ray.Origin + (Ray.Direction * Distance);
                Hit->Normal = N; // TODO: Use smooth face normal using barycentric coords!
                Hit->Material = 0; // TODO: Use face material!
            }
        }
    }
*/

    return MinDistance != Infinity;
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
            const material& Material = Scene.get_material(Hit.Material);

            ray Scattered{};
            if (Material.calculate_scattered(Ray, Hit, Rng, Scattered))
            {
                //
                // Accumulate contribution
                //

                ResultColor = ResultColor + (BounceAttenuation * Material.EmitColor);

                float CosineAttenuation = std::max(0.0f, dot(Scattered.Direction, Hit.Normal));
                vec3 BRDF = Material.brdf(Scattered.Direction, -Ray.Direction, Hit, Rng);
                BounceAttenuation = BounceAttenuation * BRDF * CosineAttenuation;

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

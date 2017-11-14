#include <random>

#include "rendering.h"

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

tracemath::vec3 get_jittered_primary_ray(image *Image, int PixelX, int PixelY, rng& Rng)
{


    tracemath::vec3 Dir = {};

    auto x = (float)PixelX;
    auto y = (float)PixelY;
    auto w = (float)Image->Width;
    auto h = (float)Image->Height;

    // Should be [0, 1] for an offset inside the pixel
#if 0
    float OffsetX = 0.5f;
    float OffsetY = 0.5f;
#else
    float OffsetX = Rng.random_01();
    float OffsetY = Rng.random_01();
#endif

    Dir.x = (x + OffsetX) / w * 2.0f - 1.0f;
    Dir.y = (y + OffsetY) / h * 2.0f - 1.0f;
    Dir.z = 1.0f;

    // Aspect ratio adjust
    float AspectRatio = w / h;
    Dir.x *= AspectRatio;

    normalize(&Dir);
    return Dir;
}

vec3 tone_map_hdr_to_ldr(const vec3& Hdr)
{
    return Hdr / (Hdr + vec3(1, 1, 1));
}

void render_scene(scene *Scene, image *Image, int RaysPerPixel, int MaxRayDepth)
{
    static rng Rng{};

    ray Ray = {};

    for (uint32_t y = 0; y < Image->Height; ++y)
    {
        for (uint32_t x = 0; x < Image->Width; ++x)
        {
            if (y % 10 == 0 && x == 0)
            {
                printf("... %d%% done ...\r", (int)roundf(static_cast<float>(y) / Image->Height * 100.0f));
                fflush(stdout);
            }

            vec3 AccumulatedHdrColor = vec3{};
            for (int i = 0; i < RaysPerPixel; ++i)
            {
                Ray.Origin = vec3(0, 0, 0);
                Ray.Direction = get_jittered_primary_ray(Image, x, y, Rng);
                vec3 Color = trace_ray(&Ray, Scene, Rng, MaxRayDepth);
                AccumulatedHdrColor = AccumulatedHdrColor + Color;
            }

            AccumulatedHdrColor = AccumulatedHdrColor * (1.0f / RaysPerPixel);
            vec3 LdrColor = tone_map_hdr_to_ldr(AccumulatedHdrColor);

            uint32_t Pixel = pixel_from_color(LdrColor);
            Image->set_pixel(x, Image->Height - y - 1, Pixel);
        }
    }

    printf("... 100%% done ...\n");
}

vec3 random_hemisphere(vec3 N, rng& Rng)
{
    normalize(&N);

    vec3 RandomOffset = vec3{ Rng.random_neg11(), Rng.random_neg11(), Rng.random_neg11() };
    return normalize(N + RandomOffset);
}

vec3 reflect(const vec3& I, const vec3& N)
{
    // See http://docs.gl/sl4/reflects
    return I - N * 2.0f * dot(N, I);
}

static float NormalOffsetAmount = 0.001f;

vec3 trace_ray(ray *Ray, scene *Scene, rng& Rng, int Depth)
{
    float Distance;
    float MinDistance;

    int  HitMaterial = -1; // (default material)
    vec3 HitPoint = {};
    vec3 HitNormal = {};

    vec3 BounceAttenuation = vec3{1, 1, 1};
    vec3 ResultColor = vec3{0, 0, 0};

    for (int i = 0; i < Depth; ++i)
    {
        MinDistance = MAXFLOAT;

        for (auto &Plane : Scene->Planes)
        {
            if (plane_intersect(&Plane, Ray, &Distance))
            {
                if (Distance > 0 && Distance < MinDistance)
                {
                    MinDistance = Distance;

                    HitPoint = Ray->Origin + (Ray->Direction * Distance);
                    HitNormal = Plane.N;
                    HitMaterial = Plane.Material;
                }
            }
        }

        for (auto &Sphere : Scene->Spheres)
        {
            if (sphere_intersect(&Sphere, Ray, &Distance))
            {
                if (Distance > 0 && Distance < MinDistance)
                {
                    MinDistance = Distance;

                    HitPoint = Ray->Origin + (Ray->Direction * Distance);
                    HitNormal = normalize(HitPoint - Sphere.P);
                    HitMaterial = Sphere.Material;
                }
            }
        }

        // No hit!
        if (MinDistance == MAXFLOAT)
        {
            // TODO: Add color from environment etc.
            break;
        }

        auto& Material = Scene->get_material(HitMaterial);

        //
        // Calculate ray for next bounce
        //

        // TODO: Use a proper light model!!!
        vec3 PerfectReflectedDirection = reflect(Ray->Direction, HitNormal);
        vec3 DiffuseRoughDirection = random_hemisphere(HitNormal, Rng);
        vec3 NewRayDirection = lerp(PerfectReflectedDirection, DiffuseRoughDirection, Material.Roughness);
        normalize(&NewRayDirection);

        // Offset slightly out by the normal so we aren't inside the currently hit object to begin with
        vec3 NewRayOrigin = HitPoint + (HitNormal * NormalOffsetAmount);

        //
        // Accumulate contribution
        //

        vec3 EmitColor = Material.Albedo * Material.Emittance;
        ResultColor = ResultColor + (BounceAttenuation * EmitColor);

        float CosineAttenuation = fmaxf(0.0f, dot(NewRayDirection, HitNormal));
        BounceAttenuation = BounceAttenuation * Material.Albedo * CosineAttenuation;

        Ray->Origin = NewRayOrigin;
        Ray->Direction = NewRayDirection;
    }

    return ResultColor;
}

uint32_t pixel_from_color(vec3 color)
{
    static const uint32_t a = 0xFF;
#if 0
    static const float gamma = 2.2f;
#else
    static const float gamma = 1.0f;
#endif
    static const float gamma_pow = 1.0f / gamma;

    uint32_t r = 0xFF & (uint32_t)(lroundf(powf(color.x, gamma_pow) * 255.0f));
    uint32_t g = 0xFF & (uint32_t)(lroundf(powf(color.y, gamma_pow) * 255.0f));
    uint32_t b = 0xFF & (uint32_t)(lroundf(powf(color.z, gamma_pow) * 255.0f));

    return (a << 24) | (b << 16) | (g << 8) | (r << 0);
}

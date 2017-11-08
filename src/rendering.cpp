#include "rendering.h"

using namespace tracemath;

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

float random_float_01()
{
    // TODO: Better randomness! Possibly also "thread safe"/RNG-per-thread?
    return (float)rand() / (float)RAND_MAX;
}

tracemath::vec3 get_jittered_primary_ray(image *Image, int PixelX, int PixelY)
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
    float OffsetX = random_float_01();
    float OffsetY = random_float_01();
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

void render_scene(scene *Scene, image *Image, int RaysPerPixel)
{
    //make_test_image(Image);

    ray Ray = {};
    Ray.Origin = vec3(0, 0, 0);

    for (uint32_t y = 0; y < Image->Height; ++y)
    {
        for (uint32_t x = 0; x < Image->Width; ++x)
        {
            vec3 AccumulatedColor = vec3{};
            for (int i = 0; i < RaysPerPixel; ++i)
            {
                Ray.Direction = get_jittered_primary_ray(Image, x, y);
                vec3 Color = trace_ray(&Ray, Scene);
                AccumulatedColor = AccumulatedColor + Color;
            }
            AccumulatedColor = AccumulatedColor * (1.0f / RaysPerPixel);

            uint32_t Pixel = pixel_from_color(AccumulatedColor);
            Image->set_pixel(x, Image->Height - y - 1, Pixel);
        }
    }

}

vec3 trace_ray(ray *Ray, scene *Scene)
{
    float Distance;
    float MinDistance = MAXFLOAT;

    // TODO: Implement material system!
    vec3 HitNormal = {};
    vec3 Color = {};

    for (auto& Plane : Scene->Planes)
    {
        if (plane_intersect(&Plane, Ray, &Distance))
        {
            if (Distance > 0 && Distance < MinDistance)
            {
                MinDistance = Distance;

                Color = vec3{0, 1, 0};
                HitNormal = Plane.N;
            }
        }
    }

    for (auto& Sphere : Scene->Spheres)
    {
        if (sphere_intersect(&Sphere, Ray, &Distance))
        {
            if (Distance > 0 && Distance < MinDistance)
            {
                MinDistance = Distance;

                Color = vec3{1, 0, 0};
                vec3 HitPoint = Ray->Origin + (Ray->Direction * Distance);
                HitNormal = sphere_normal(&Sphere, HitPoint);
            }
        }
    }

    if (MinDistance == MAXFLOAT)
    {
        return Color;
    }

    // TODO: Only like this for now, obviously...
    float Lightness = dot(HitNormal, -Scene->LightDirection);
    Lightness = fmaxf(0.0f, Lightness);
    Color = Color * Lightness;

    return Color;
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

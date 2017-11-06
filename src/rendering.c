#include "rendering.h"

#include <stdlib.h>

void make_test_image(image *Image)
{
    for (uint32_t y = 0; y < Image->Height; ++y) {
        for (uint32_t x = 0; x < Image->Width; ++x) {

            uint32_t a = 0xFF000000;
            uint32_t b = ((x + y) % 0xFF) << 16;
            uint32_t g = (y % 0xFF) << 8;
            uint32_t r = (x % 0xFF) << 0;

            uint32_t PixelValue = r | g | b | a;
            image_set_pixel(Image, x, y, PixelValue);
        }
    }

    image_set_pixel(Image, 0, 0, 0xFFFFFFFF);
    image_set_pixel(Image, 0, Image->Height - 1, 0xFFFFFFFF);
    image_set_pixel(Image, Image->Width - 1, 0, 0xFFFFFFFF);
    image_set_pixel(Image, Image->Width - 1, Image->Height - 1, 0xFFFFFFFF);
}

float random_float_01()
{
    // TODO: Better randomness! Possibly also "thread safe"/RNG-per-thread?
    return (float)rand() / (float)RAND_MAX;
}

vec3 get_primary_ray(image* Image, int PixelX, int PixelY)
{
    vec3 Dir = {};

    float x = (float)PixelX;
    float y = (float)PixelY;
    float w = (float)Image->Width;
    float h = (float)Image->Height;

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

    vec3_normalize(&Dir, Dir);
    return Dir;
}

void render_scene(scene *Scene, image *Image, int RaysPerPixel)
{
    //make_test_image(Image);

    ray Ray = {};
    Ray.Origin = vec3_make(0, 0, 0);

    for (uint32_t y = 0; y < Image->Height; ++y)
    {
        for (uint32_t x = 0; x < Image->Width; ++x)
        {
            vec3 AccumulatedColor = vec3_make(0, 0, 0);
            for (int i = 0; i < RaysPerPixel; ++i)
            {
                Ray.Direction = get_primary_ray(Image, x, y);
                vec3 Color = trace_ray(&Ray, Scene);
                vec3_add(&AccumulatedColor, AccumulatedColor, Color);
            }
            vec3_scale(&AccumulatedColor, AccumulatedColor, 1.0f / RaysPerPixel);

            uint32_t Pixel = pixel_from_color(AccumulatedColor);
            image_set_pixel(Image, x, Image->Height - y - 1, Pixel);
        }
    }

}

vec3 calculate_hit_point(ray *Ray, float Distance)
{
    vec3 HitPoint = {};
    vec3_scale(&HitPoint, Ray->Direction, Distance);
    vec3_add(&HitPoint, HitPoint, Ray->Origin);
    return HitPoint;
}

vec3 trace_ray(ray *Ray, scene *Scene)
{
    float Distance;
    float MinDistance = MAXFLOAT;

    // TODO: Implement material system!
    vec3 HitNormal = {};
    vec3 Color = {};

    for (int i = 0; i < Scene->NumPlanes; ++i)
    {
        plane *Plane = Scene->Planes + i;
        if (plane_intersect(Plane, Ray, &Distance))
        {
            if (Distance > 0 && Distance < MinDistance)
            {
                MinDistance = Distance;

                Color = vec3_make(0, 1, 0);
                HitNormal = Plane->N;
            }
        }
    }

    for (int i = 0; i < Scene->NumSpheres; ++i)
    {
        sphere *Sphere = Scene->Spheres + i;
        if (sphere_intersect(Sphere, Ray, &Distance))
        {
            if (Distance > 0 && Distance < MinDistance)
            {
                MinDistance = Distance;

                Color = vec3_make(1, 0, 0);
                vec3 HitPoint = calculate_hit_point(Ray, Distance);
                HitNormal = sphere_normal(Sphere, HitPoint);
            }
        }
    }

    if (MinDistance == MAXFLOAT)
    {
        return Color;
    }

    // TODO: Only like this for now, obviously...
    vec3 wo = {};
    vec3_negate(&wo, Scene->LightDirection);
    float Lightness = vec3_dot(HitNormal, wo);
    Lightness = fmaxf(0.0f, Lightness);
    vec3_scale(&Color, Color, Lightness);

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

    uint32_t r = 0xFF & (uint32_t)(powf(color.x, gamma_pow) * 255.0f + 0.5f);
    uint32_t g = 0xFF & (uint32_t)(powf(color.y, gamma_pow) * 255.0f + 0.5f);
    uint32_t b = 0xFF & (uint32_t)(powf(color.z, gamma_pow) * 255.0f + 0.5f);

    return (a << 24) | (b << 16) | (g << 8) | (r << 0);
}

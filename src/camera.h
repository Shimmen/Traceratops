#ifndef TRACERATOPS_CAMERA_H
#define TRACERATOPS_CAMERA_H

#include "geometry.h"

struct camera
{
    camera(vec3 Origin, vec3 LookAt, const vec3& Up, const image& Image, float VFov, float ApertureSize)
            : Origin(Origin), LookAt(LookAt), LensRadius(ApertureSize / 2.0f), FocusDistance(length(Origin - LookAt))
    {
        float AspectRatio = float(Image.Width) / float(Image.Height);
        float Theta = VFov * tracemath::PI / 180.0f;

        float HalfHeight = tan(Theta / 2.0f);
        float HalfWidth  = AspectRatio * HalfHeight;

        // Create camera basis (note that the camera looks down +W)
        // I.e, I use a LEFT HANDED COORDINATE SYSTEM!!! Don't forget this!
        W = normalize(LookAt - Origin);
        U = normalize(cross(Up, W));
        V = normalize(cross(W, U));

        LowerLeft = Origin - HalfWidth * FocusDistance * U - HalfHeight * FocusDistance * V + FocusDistance * W;
        Horizontal = 2.0f * HalfWidth * FocusDistance * U;
        Vertical   = 2.0f * HalfHeight * FocusDistance * V;
    }
    ~camera() = default;

    ray get_jittered_ray(int x, int y, int ImageWidth, int ImageHeight, rng& Rng) const
    {
        float u = (x + 0.5f + 0.5f * Rng.random_neg11()) / ImageWidth;
        float v = (y + 0.5f + 0.5f * Rng.random_neg11()) / ImageHeight;
        return get_ray(u, v, Rng);
    }

    ray get_ray(float u, float v, rng& Rng) const
    {
        ray Ray{};

        vec3 LensSample = LensRadius * Rng.random_in_unit_disk();
        vec3 Offset = LensSample.x * U + LensSample.y * V;

        Ray.Origin = Origin + Offset;
        Ray.Direction = normalize(LowerLeft + Horizontal * u + Vertical * v - Origin - Offset);

        return Ray;
    }

private:

    vec3 Origin;
    vec3 LookAt;

    float LensRadius;
    float FocusDistance;

    // Camera view basis
    vec3 U{};
    vec3 V{};
    vec3 W{};

    vec3 LowerLeft{};
    vec3 Horizontal{};
    vec3 Vertical{};

};

#endif // TRACERATOPS_CAMERA_H

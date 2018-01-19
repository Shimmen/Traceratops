#ifndef TRACERATOPS_CAMERA_H
#define TRACERATOPS_CAMERA_H

#include "geometry.h"

struct camera
{
    camera(vec3 Origin, vec3 LookAt, vec3 Up, const image& Image, float VFov)
            : Origin(Origin), LookAt(LookAt)
    {
        float AspectRatio = float(Image.Width) / float(Image.Height);
        float Theta = VFov * tracemath::PI / 180.0f;

        float HalfHeight = tan(Theta / 2.0f);
        float HalfWidth  = AspectRatio * HalfHeight;

        // Create camera basis (note that the camera looks down +W)
        // I.e, I use a LEFT HANDED COORDINATE SYSTEM!!! Don't forget this!
        vec3 W = normalize(LookAt - Origin);
        vec3 U = normalize(cross(Up, W));
        vec3 V = normalize(cross(W, U));

        LowerLeft = Origin - HalfWidth * U - HalfHeight * V + W;
        Horizontal = 2.0f * HalfWidth * U;
        Vertical   = 2.0f * HalfHeight * V;
    }
    ~camera() = default;

    ray get_ray(float u, float v) const
    {
        ray Ray{};
        Ray.Origin = Origin;
        Ray.Direction = normalize(LowerLeft + Horizontal * u + Vertical * v - Origin);
        return Ray;
    }

private:

    vec3 Origin;
    vec3 LookAt;

    vec3 LowerLeft{};
    vec3 Horizontal{};
    vec3 Vertical{};

};

#endif // TRACERATOPS_CAMERA_H

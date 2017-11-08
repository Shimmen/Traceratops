#include "geometry.h"

using namespace tracemath;

static const float IntersectionTolerance = 0.0001f;

int sphere_intersect(sphere *Sphere, ray *Ray, float *t)
{
    vec3 RelOrigin = Ray->Origin - Sphere->P;

    // (quadratic formula)
    float a = dot(Ray->Direction, Ray->Direction);
    float b = 2.0f * dot(Ray->Direction, RelOrigin);
    float c = dot(RelOrigin, RelOrigin) - (Sphere->r * Sphere->r);

    float Denom = 2.0f * a;
    float SqrtInner = b * b - 4.0f * a * c;

    if (SqrtInner < 0) return 0;
    if (Denom == 0) return 0;

    float RootTerm = sqrtf(SqrtInner);
    if (RootTerm < IntersectionTolerance) return 0;

    float tp = (-b + RootTerm) / Denom;
    float tn = (-b - RootTerm) / Denom;

    // Select closest distance (that is greater than zero)
    *t = tp;
    if (tn > 0 && tn < tp)
    {
        *t = tn;
    }

    return *t > 0;
}

vec3 sphere_normal(sphere *Sphere, vec3 Point)
{
    vec3 Normal = Point - Sphere->P;
    normalize(&Normal);
    return Normal;
}

int plane_intersect(plane *Plane, ray *Ray, float *t)
{
    float Denominator = dot(Plane->N, Ray->Direction);
    if (fabsf(Denominator) < IntersectionTolerance) return 0;

    vec3 ToPlaneCenter = Plane->P - Ray->Origin;

    *t = dot(ToPlaneCenter, Plane->N) / Denominator;
    return *t > 0.0f;
}

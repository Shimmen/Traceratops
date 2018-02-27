#include "geometry.h"

static const float IntersectionTolerance = 0.0001f;

bool sphere::intersect(const ray &Ray, float TMin, float TMax, hit_info& Hit) const
{
    vec3 RelOrigin = Ray.Origin - C;

    // (quadratic formula)
    float a = dot(Ray.Direction, Ray.Direction);
    float b = 2.0f * dot(Ray.Direction, RelOrigin);
    float c = dot(RelOrigin, RelOrigin) - (r * r);

    float Denom = 2.0f * a;
    float SqrtInner = b * b - 4.0f * a * c;

    if (SqrtInner < 0) return false;
    if (Denom == 0) return false;

    float RootTerm = sqrt(SqrtInner);
    if (RootTerm < IntersectionTolerance) return false;

    float tp = (-b + RootTerm) / Denom;
    float tn = (-b - RootTerm) / Denom;

    // Select closest distance (that is greater than zero)
    float t = tp;
    if (tn >= TMin && tn < tp)
    {
        t = tn;
    }

    if (t >= TMin && t <= TMax)
    {
        Hit.Distance = t;
        Hit.Point = Ray.Origin + t * Ray.Direction;
        Hit.Normal = normalize(Hit.Point - C);
        Hit.Material = Material;
        return true;
    }

    return false;
}

bool sphere::get_aabb(aabb& AABB) const
{

    AABB.Min = C - vec3{r, r, r};
    AABB.Max = C + vec3{r, r, r};
    return true;
}

bool disc::intersect(const ray &Ray, float TMin, float TMax, hit_info &Hit) const
{
    float Denominator = dot(N, Ray.Direction);
    if (fabsf(Denominator) < IntersectionTolerance) return 0;

    // Uni-directional (backface culling)
    // TODO: Actually this sort of breaks occlusion...
    if (Denominator > 0) return false;

    vec3 ToPlaneCenter = P - Ray.Origin;

    float t = dot(ToPlaneCenter, N) / Denominator;
    if (t >= TMin && t <= TMax)
    {
        Hit.Distance = t;
        Hit.Point = Ray.Origin + t * Ray.Direction;

        if (length2(Hit.Point - P) <= (r * r))
        {
            Hit.Normal = N;
            Hit.Material = Material;
            return true;
        }
    }

    return false;
}

bool disc::get_aabb(aabb &AABB) const
{
    // Compute AABB as for a sphere
    AABB.Min = P - vec3{r, r, r};
    AABB.Max = P + vec3{r, r, r};
    return true;
    /*
    // FIXME: This doesn't work...
    // Create a small box around the disc (with 0.5m thickness) using a tangent and bitangent to the normal
    const vec3& T  = normalize(perpendicular(N));
    const vec3& B = cross(N, T);
    AABB.Min = P - (T * r) - (B * r) - (N * 0.25f);
    AABB.Max = P + (T * r) + (B * r) + (N * 0.25f);
    return true;
    */
}

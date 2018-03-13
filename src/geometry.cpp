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
        Hit.Hitable = this;

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
            Hit.Hitable = this;

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

bool triangle::intersect(const ray& Ray, float TMin, float TMax, hit_info& Hit) const
{
    // From GraphicsCodex

    const vec3& E1 = V1 - V0;
    const vec3& E2 = V2 - V0;

    vec3 N = cross(E1, E2);
    normalize(&N);

    vec3 q = cross(Ray.Direction, E2);
    float a = dot(E1, q);

    // (Nearly) parallel or backfacing, or close to the limit of precision?
    if (dot(N, Ray.Direction) >= 0 || fabsf(a) <= IntersectionTolerance)
    {
        return false;
    }

    const vec3& s = (Ray.Origin - V0) / a;
    const vec3& r = cross(s, E1);

    // Barycentric coordinates
    float b[3];
    b[0] = dot(s, q);
    b[1] = dot(r, Ray.Direction);
    b[2] = 1.0f - b[0] - b[1];

    // Intersected inside triangle?
    float t = dot(E2, r);
    if ((b[0] >= 0) && (b[1] >= 0) && (b[2] >= 0) && t >= TMin && t <= TMax)
    {
        Hit.Distance = t;
        Hit.Point = Ray.Origin + (Ray.Direction * t);
        Hit.Normal = N; // TODO: Use smooth face normal using barycentric coords!

        Hit.Material = Material;
        Hit.Hitable = this;

        return true;
    }

    return false;
}

bool triangle::get_aabb(aabb& AABB) const
{
    AABB.Min = V0;
    min(&AABB.Min, V1);
    min(&AABB.Min, V2);

    AABB.Max = V0;
    max(&AABB.Max, V1);
    max(&AABB.Max, V2);

    // Expand AABB so we don't miss very AA-wise-thin triangles

    AABB.Min.x -= 0.01f;
    AABB.Min.y -= 0.01f;
    AABB.Min.z -= 0.01f;

    AABB.Max.x += 0.01f;
    AABB.Max.y += 0.01f;
    AABB.Max.z += 0.01f;

    return true;
}

#include "geometry.h"
#include "scene.h"

static const float IntersectionTolerance = 0.0001f;

bool sphere::intersect(const ray &Ray, float TMin, float TMax, hit_info& Hit, rng& Rng) const
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

        Hit.TextureCoordinate.y = Hit.Normal.y * 0.5f + 0.5f;
        Hit.TextureCoordinate.x = atan2(Hit.Normal.z, Hit.Normal.x) / tracemath::TWO_PI;

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

bool moving_sphere::intersect(const ray& Ray, float TMin, float TMax, hit_info& Hit, rng& Rng) const
{
    float TimeSlice = Rng.random_01();
    vec3 CurrentC = C + TimeSlice * Velocity;

    vec3 RelOrigin = Ray.Origin - CurrentC;

    // (quadratic formula)
    float a = dot(Ray.Direction, Ray.Direction);
    float b = 2.0f * dot(Ray.Direction, RelOrigin);
    float c = dot(RelOrigin, RelOrigin) - (r * r);

    float Denom = 2.0f * a;
    float SqrtInner = b * b - 4.0f * a * c;

    if (SqrtInner < 0) return false;
    if (Denom == 0) return false;

    float RootTerm = sqrt(SqrtInner);
    if (RootTerm < 0.0001f) return false;

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

        Hit.TextureCoordinate.y = Hit.Normal.y * 0.5f + 0.5f;
        Hit.TextureCoordinate.x = atan2(Hit.Normal.z, Hit.Normal.x) / tracemath::TWO_PI;

        Hit.Hitable = this;
        return true;
    }

    return false;
}

bool moving_sphere::get_aabb(aabb& AABB) const
{
    aabb Path[2];

    Path[0].Min = C - vec3{r, r, r};
    Path[0].Max = C + vec3{r, r, r};

    Path[1].Min = C + Velocity - vec3{r, r, r};
    Path[1].Max = C + Velocity + vec3{r, r, r};

    AABB = aabb_enclosing(Path, 2);
    return true;
}

bool disc::intersect(const ray &Ray, float TMin, float TMax, hit_info &Hit, rng& Rng) const
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

bool triangle::intersect(const ray& Ray, float TMin, float TMax, hit_info& Hit, rng& Rng) const
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

        const scene& Scene = scene::get_current();
        triangle_face Face = Scene.TriangleFaces[FaceIndex];

        // TODO: This works... What's up with the order though?
        vec3 Barycentric{b[2], b[0], b[1]};

        if (Face.HasNormals)
        {
            Hit.Normal = normalize(Barycentric[0] * Face.Normals[0] + Barycentric[1] * Face.Normals[1] + Barycentric[2] * Face.Normals[2]);
        }
        else
        {
            // Use actual/mathematical face normal
            Hit.Normal = N;
        }

        if (Face.HasUVs)
        {
            Hit.TextureCoordinate = Barycentric[0] * Face.UVs[0] + Barycentric[1] * Face.UVs[1] + Barycentric[2] * Face.UVs[2];
        }

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

    AABB.Min.x -= 0.001f;
    AABB.Min.y -= 0.001f;
    AABB.Min.z -= 0.001f;

    AABB.Max.x += 0.001f;
    AABB.Max.y += 0.001f;
    AABB.Max.z += 0.001f;

    return true;
}

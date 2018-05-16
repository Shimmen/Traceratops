#ifndef TRACERATOPS_AABB_H
#define TRACERATOPS_AABB_H

#include <algorithm>

#include "tracemath.h"

struct aabb
{
    vec3 Min{};
    vec3 Max{};

    aabb() {}
    aabb(const vec3& Min, const vec3& Max) : Min(Min), Max(Max) {}

};

static inline
aabb aabb_enclosing(const aabb *AABBs, size_t Count)
{
    assert(AABBs != nullptr && Count > 0);

    aabb Enclosing = AABBs[0];
    for (size_t i = 1; i < Count; ++i)
    {
        min(&Enclosing.Min, AABBs[i].Min);
        max(&Enclosing.Max, AABBs[i].Max);
    }
    return Enclosing;
}

static inline
bool aabb_intersection(const aabb& a, const aabb& b)
{
    return (a.Min.x <= b.Max.x && a.Max.x >= b.Min.x) &&
           (a.Min.y <= b.Max.y && a.Max.y >= b.Min.y) &&
           (a.Min.z <= b.Max.z && a.Max.z >= b.Min.z);
}

static inline
bool aabb_point_intersection(const aabb& a, const vec3& p)
{
#if 1
    return p.x >= a.Min.x && p.x <= a.Max.x &&
           p.y >= a.Min.y && p.y <= a.Max.y &&
           p.z >= a.Min.z && p.z <= a.Max.z;
#else
    constexpr float e = 0.5f;
    return (p.x + e) >= a.Min.x && (p.x - e) <= a.Max.x &&
           (p.y + e) >= a.Min.y && (p.y - e) <= a.Max.y &&
           (p.z + e) >= a.Min.z && (p.z - e) <= a.Max.z;
#endif
}


static inline
bool aabb_ray_intersection(const aabb& b, const ray& Ray, float MaxT)
{
    // From Tavian Barnes:
    // https://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans/

#define AABB_RAY_NAN_CORRECTNESS 1

    float t1 = (b.Min[0] - Ray.Origin[0]) / Ray.Direction[0];
    float t2 = (b.Max[0] - Ray.Origin[0]) / Ray.Direction[0];

    float tmin = std::min(t1, t2);
    float tmax = std::max(t1, t2);

    for (int i = 1; i < 3; ++i)
    {
        t1 = (b.Min[i] - Ray.Origin[i]) / Ray.Direction[i];
        t2 = (b.Max[i] - Ray.Origin[i]) / Ray.Direction[i];

#if AABB_RAY_NAN_CORRECTNESS == 1
        tmin = std::max(tmin, std::min(std::min(t1, t2), tmax));
        tmax = std::min(tmax, std::max(std::max(t1, t2), tmin));
#else
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
#endif
    }

    bool DidIntersectAtAll = tmax > std::max(tmin, 0.0f);

    // NOTE: We can't use a min value since AABB in the scene are allowed to intersect. So if we intersected an AABB we
    // can never know that that intersection point doesn't also intersect some other AABB so therefore we can't use MinT
    float t = tmin;
    return DidIntersectAtAll && t <= MaxT;
}

#endif // TRACERATOPS_AABB_H

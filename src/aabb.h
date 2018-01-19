#ifndef TRACERATOPS_AABB_H
#define TRACERATOPS_AABB_H

#include "tracemath.h"
#include "triangle_mesh.h"

struct aabb
{
    vec3 Min{};
    vec3 Max{};

    std::vector<const triangle_face *> ContainedTriangles{};

    //bool IsLeaf = false;
    std::vector<size_t> Children;
    //aabb *Children[2] = {};

    aabb(const vec3& Min, const vec3& Max) : Min(Min), Max(Max) {}

    //aabb(const tracemath::vec3& Min, const tracemath::vec3& Max) : Min(Min), Max(Max), IsLeaf(false), Children() {}
    //std::vector<aabb> Children;
    //void add_child(const aabb& Child) { Children.push_back(Child); }
};

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
bool aabb_ray_intersection(const aabb& b, const vec3& Direction, const vec3& Origin)
{
    // From Tavian Barnes:
    // https://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans/

#define AABB_RAY_NAN_CORRECTNESS

    float t1 = (b.Min[0] - Origin[0]) / Direction[0];
    float t2 = (b.Max[0] - Origin[0]) / Direction[0];

    float tmin = std::min(t1, t2);
    float tmax = std::max(t1, t2);

    for (int i = 1; i < 3; ++i)
    {
        t1 = (b.Min[i] - Origin[i]) / Direction[i];
        t2 = (b.Max[i] - Origin[i]) / Direction[i];

#ifdef AABB_RAY_NAN_CORRECTNESS
        tmin = std::max(tmin, std::min(std::min(t1, t2), tmax));
        tmax = std::min(tmax, std::max(std::max(t1, t2), tmin));
#else
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
#endif
    }

    return tmax > std::max(tmin, 0.0f);
}

#endif // TRACERATOPS_AABB_H

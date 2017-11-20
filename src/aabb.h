#ifndef TRACERATOPS_AABB_H
#define TRACERATOPS_AABB_H

#include "tracemath.h"
#include "triangle_mesh.h"

struct aabb
{
    tracemath::vec3 Min;
    tracemath::vec3 Max;

    std::vector<const triangle_face *> ContainedTriangles;
    std::vector<aabb> Children;

    aabb() = default;
    aabb(const tracemath::vec3& Min, const tracemath::vec3& Max) : Min(Min), Max(Max) {}

    void add_child(const aabb& Child) { Children.push_back(Child); }
};

static inline
bool aabb_intersection(const aabb &a, const aabb &b)
{
    return (a.Min.x <= b.Max.x && a.Max.x >= b.Min.x) &&
           (a.Min.y <= b.Max.y && a.Max.y >= b.Min.y) &&
           (a.Min.z <= b.Max.z && a.Max.z >= b.Min.z);
}

#endif // TRACERATOPS_AABB_H

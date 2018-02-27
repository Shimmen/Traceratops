#ifndef TRACERATOPS_GEOMETRY_H
#define TRACERATOPS_GEOMETRY_H

#include "tracemath.h"
#include "aabb.h"

struct ray
{
    vec3 Origin;
    vec3 Direction;
};

struct hit_info
{
    vec3 Point;
    vec3 Normal;
    int Material;
    float Distance;
};

struct hitable
{
    explicit hitable(int Material = 0) : Material(Material) {}

    virtual bool intersect(const ray& Ray, float TMin, float TMax, hit_info& Hit) const = 0;
    virtual bool get_aabb(aabb& AABB) const = 0;

    int Material;
};

struct sphere: public hitable
{
    sphere(vec3 C, float r, int Material) : hitable(Material), C(C), r(r) {}

    virtual bool intersect(const ray& Ray, float TMin, float TMax, hit_info& Hit) const;
    virtual bool get_aabb(aabb& AABB) const;

    vec3 C;
    float r;
};

struct disc: public hitable
{
    disc(vec3 P, vec3 N, float r, int Material) : hitable(Material), P(P), N(N), r(r) {}

    virtual bool intersect(const ray& Ray, float TMin, float TMax, hit_info& Hit) const;
    virtual bool get_aabb(aabb& AABB) const;

    vec3 P;
    vec3 N;
    float r;
};

#endif // TRACERATOPS_GEOMETRY_H

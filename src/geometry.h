#ifndef TRACERATOPS_GEOMETRY_H
#define TRACERATOPS_GEOMETRY_H

#include "tracemath.h"

struct ray
{
    vec3 Origin;
    vec3 Direction;
};

struct sphere
{
    vec3 P;
    float r;

    int Material;

    sphere(vec3 P, float r, int Material) : P(P), r(r), Material(Material) {}
};

struct plane
{
    vec3 P;
    vec3 N;

    int Material;

    plane(vec3 P, vec3 N, int Material) : P(P), N(N), Material(Material) {}
};

struct disc
{
    plane Plane;
    float r;

    disc(vec3 P, vec3 N, float r, int Material) : Plane(P, N, Material), r(r) {}
};


int sphere_intersect(const sphere& Sphere, const ray& Ray, float *t);
int plane_intersect(const plane& Plane, const ray& Ray, float *t);

#endif // TRACERATOPS_GEOMETRY_H

#ifndef TRACERATOPS_GEOMETRY_H
#define TRACERATOPS_GEOMETRY_H

#include "tracemath.h"

struct ray
{
    tracemath::vec3 Origin;
    tracemath::vec3 Direction;
};

struct sphere
{
    tracemath::vec3 P;
    float r;

    sphere(tracemath::vec3 P, float r) : P(P), r(r) {}
};

int sphere_intersect(sphere *Sphere, ray *Ray, float *t);
tracemath::vec3 sphere_normal(sphere *Sphere, tracemath::vec3 Point);

struct plane
{
    tracemath::vec3 P;
    tracemath::vec3 N;

    plane(tracemath::vec3 P, tracemath::vec3 N) : P(P), N(N) {}
};

int plane_intersect(plane *Plane, ray *Ray, float *t);

#endif // TRACERATOPS_GEOMETRY_H

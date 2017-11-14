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

    int Material;

    sphere(tracemath::vec3 P, float r, int Material) : P(P), r(r), Material(Material) {}
};

struct plane
{
    tracemath::vec3 P;
    tracemath::vec3 N;

    int Material;

    plane(tracemath::vec3 P, tracemath::vec3 N, int Material) : P(P), N(N), Material(Material) {}
};


int sphere_intersect(sphere *Sphere, ray *Ray, float *t);
int plane_intersect(plane *Plane, ray *Ray, float *t);

#endif // TRACERATOPS_GEOMETRY_H

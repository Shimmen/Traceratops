#ifndef TRACERATOPS_GEOMETRY_H
#define TRACERATOPS_GEOMETRY_H

#include "traceratops_math.h"

typedef struct ray
{
    vec3 Origin;
    vec3 Direction;
} ray;

typedef struct sphere
{
    vec3 P;
    float r;
} sphere;

int sphere_intersect(sphere *Sphere, ray *Ray, float *t);
vec3 sphere_normal(sphere *Sphere, vec3 Point);

typedef struct plane
{
    vec3 P;
    vec3 N;
} plane;

int plane_intersect(plane *Plane, ray *Ray, float *t);

#endif // TRACERATOPS_GEOMETRY_H

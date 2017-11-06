#ifndef TRACERATOPS_MATH_H
#define TRACERATOPS_MATH_H

#include <math.h>

typedef struct vec3
{
    float x;
    float y;
    float z;
} vec3;

static inline
vec3 vec3_make(float x, float y, float z)
{
    vec3 Vec = {};
    Vec.x = x;
    Vec.y = y;
    Vec.z = z;
    return Vec;
}

static inline
void vec3_negate(vec3 *out, vec3 a)
{
    out->x = -a.x;
    out->y = -a.y;
    out->z = -a.z;
}

static inline
void vec3_add(vec3 *out, vec3 a, vec3 b)
{
    out->x = a.x + b.x;
    out->y = a.y + b.y;
    out->z = a.z + b.z;
}

static inline
void vec3_sub(vec3 *out, vec3 a, vec3 b)
{
    out->x = a.x - b.x;
    out->y = a.y - b.y;
    out->z = a.z - b.z;
}

static inline
void vec3_scale(vec3 *out, vec3 a, float scale)
{
    out->x = a.x * scale;
    out->y = a.y * scale;
    out->z = a.z * scale;
}

static inline
float vec3_length2(vec3 a)
{
    return (a.x * a.x) + (a.y * a.y) + (a.z * a.z);
}

static inline
float vec3_length(vec3 a)
{
    return sqrtf(vec3_length2(a));
}

static inline
void vec3_normalize(vec3 *out, vec3 a)
{
    float len = vec3_length(a);
    vec3_scale(out, a, 1.0f / len);
}

static inline
vec3 vec3_direction(float x, float y, float z)
{
    vec3 Vec = {};
    Vec.x = x;
    Vec.y = y;
    Vec.z = z;
    vec3_normalize(&Vec, Vec);
    return Vec;
}

static inline
float vec3_dot(vec3 a, vec3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

#endif // TRACERATOPS_MATH_H

#ifndef TRACERATOPS_MATH_H
#define TRACERATOPS_MATH_H

#include <cmath>
#include <cassert>

namespace tracemath
{
    struct vec3
    {
        float x;
        float y;
        float z;

        vec3() = default;
        vec3(vec3& other) = default;
        vec3(const vec3& other) = default;
        vec3(float x, float y, float z) : x(x), y(y), z(z) {};

        // (has to be member function)
        vec3 operator-() { return vec3{-x, -y, -z}; };
    };

    static inline
    vec3 operator+(const vec3& a, const vec3& b)
    {
        return vec3{a.x + b.x, a.y + b.y, a.z + b.z};
    }

    static inline
    vec3 operator-(const vec3& a, const vec3& b)
    {
        return vec3{a.x - b.x, a.y - b.y, a.z - b.z};
    }

    static inline
    vec3 operator*(const vec3& a, float s)
    {
        return vec3{a.x * s, a.y * s, a.z * s};
    }

    static inline
    vec3 operator/(const vec3& a, float s)
    {
        return vec3{a.x / s, a.y / s, a.z / s};
    }

    static inline
    vec3 cross(const vec3& a, const vec3& b)
    {
        return vec3{
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    static inline
    float dot(const vec3& a, const vec3& b)
    {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    }

    static inline
    float length2(const vec3& a)
    {
        return (a.x * a.x) + (a.y * a.y) + (a.z * a.z);
    }

    static inline
    float length(const vec3& a)
    {
        return sqrtf(length2(a));
    }

    static inline
    vec3 normalize(const vec3& a)
    {
        float len = length(a);
        return a * (1.0f / len);
    }

    static inline
    void normalize(vec3 *a)
    {
        assert(a);
        float len = length(*a);
        a->x = a->x / len;
        a->y = a->y / len;
        a->z = a->z / len;
    }

    static inline
    vec3 make_direction(float x, float y, float z)
    {
        vec3 vec = {x, y, z};
        normalize(&vec);
        return vec;
    }

}

#endif // TRACERATOPS_MATH_H

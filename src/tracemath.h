#ifndef TRACERATOPS_MATH_H
#define TRACERATOPS_MATH_H

#include <cmath>
#include <chrono>
#include <cassert>
#include <random>

namespace tracemath
{
    ///////////////////////////////////////////////////////////////
    // constants etc.

    constexpr float PI  = 3.14159265358979323846264338327950288f;
    constexpr float TAU = 2.0f * PI;

    constexpr float TWO_PI = TAU;

    ///////////////////////////////////////////////////////////////
    // vectors

    struct vec2
    {
        float x;
        float y;

        vec2() = default;
        vec2(vec2& other) = default;
        vec2(const vec2& other) = default;
        vec2(float x, float y) : x(x), y(y) {};
    };

    struct vec3
    {
        float x;
        float y;
        float z;

        vec3() = default;
        vec3(vec3& other) = default;
        vec3(const vec3& other) = default;
        vec3(float x, float y, float z) : x(x), y(y), z(z) {};
        explicit vec3(float val) : x(val), y(val), z(val) {};

        // (has to be member function)
        vec3 operator-() const { return vec3{-x, -y, -z}; };
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
    vec3 operator*(const vec3& a, const vec3& b)
    {
        return vec3{a.x * b.x, a.y * b.y, a.z * b.z};
    }

    static inline
    vec3 operator/(const vec3& a, const vec3& b)
    {
        return vec3{a.x / b.x, a.y / b.y, a.z / b.z};
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

    static inline
    vec3 lerp(const vec3 &a, const vec3 &b, float x)
    {
        return a + (b - a) * x;
    }

    static inline
    void min(vec3 *a, const vec3& b)
    {
        a->x = fminf(a->x, b.x);
        a->y = fminf(a->y, b.y);
        a->z = fminf(a->z, b.z);
    }

    static inline
    void max(vec3 *a, const vec3& b)
    {
        a->x = fmaxf(a->x, b.x);
        a->y = fmaxf(a->y, b.y);
        a->z = fmaxf(a->z, b.z);
    }

    ///////////////////////////////////////////////////////////////
    // aabb

    struct aabb
    {
        vec3 Min;
        vec3 Max;

        aabb() = default;
        aabb(const vec3& Min, const vec3& Max) : Min(Min), Max(Max) {}
    };

    static inline
    bool aabb_intersection(const aabb &a, const aabb &b)
    {
        return (a.Min.x <= b.Max.x && a.Max.x >= b.Min.x) &&
               (a.Min.y <= b.Max.y && a.Max.y >= b.Min.y) &&
               (a.Min.z <= b.Max.z && a.Max.z >= b.Min.z);
    }

    ///////////////////////////////////////////////////////////////
    // random

    class rng
    {
    public:

        rng()
        {
            typedef std::chrono::high_resolution_clock myclock;
            myclock::time_point now = myclock::now();
            unsigned int seed = static_cast<unsigned int>(now.time_since_epoch().count());
            engine.seed(seed);
        }

        inline float random_01()
        {
            return uniform_01_dist(engine);
        }

        inline float random_neg11()
        {
            return uniform_neg11_dist(engine);
        }

    private:

        std::default_random_engine engine;
        std::uniform_real_distribution<float> uniform_01_dist{0.0f, 1.0f};
        std::uniform_real_distribution<float> uniform_neg11_dist{-1.0f, 1.0f};

    };


    ///////////////////////////////////////////////////////////////
}

#endif // TRACERATOPS_MATH_H

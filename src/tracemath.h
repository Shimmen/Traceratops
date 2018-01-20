#ifndef TRACERATOPS_MATH_H
#define TRACERATOPS_MATH_H

#include <cmath>
#include <chrono>
#include <cassert>
#include <random>
#include <thread>

///////////////////////////////////////////////////////////////
// constants etc.
namespace tracemath
{
    constexpr float PI  = 3.14159265358979323846264338327950288f;
    constexpr float TAU = 2.0f * PI;

    constexpr float TWO_PI = TAU;
}

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
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    explicit vec3(float val) : x(val), y(val), z(val) {}

    vec3 operator-() const
    {
        return vec3{-x, -y, -z};
    }

    float& operator[](int i)
    {
        assert(i >= 0 && i < 3);
        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        return x; // (shouldn't happen)
    }

    float operator[](int i) const
    {
        assert(i >= 0 && i < 3);
        if (i == 0) return x;
        if (i == 1) return y;
        if (i == 2) return z;
        return x; // (shouldn't happen)
    }

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
vec3 operator*(float s, const vec3& a)
{
    return a * s;
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

static inline
int index_of_min(const vec3& a)
{
    if (a.x < a.y && a.x < a.z) return 0;
    if (a.y < a.x && a.y < a.z) return 1;
    if (a.z < a.x && a.z < a.y) return 2;
    return 0; // (all are equal, just select one axis)
}

static inline
int index_of_max(const vec3& a)
{
    if (a.x > a.y && a.x > a.z) return 0;
    if (a.y > a.x && a.y > a.z) return 1;
    if (a.z > a.x && a.z > a.y) return 2;
    return 0; // (all are equal, just select one axis)
}

static inline
vec3 max_and_zeroes(const vec3& a)
{
    vec3 Res{};
    Res.x = (a.x > a.y && a.x > a.z) ? a.x : 0;
    Res.y = (a.y > a.x && a.y > a.z) ? a.y : 0;
    Res.z = (a.z > a.x && a.z > a.y) ? a.z : 0;

    // (all are equal, just select one axis)
    if (Res.x == 0 && Res.y == 0 && Res.z == 0)
    {
        Res.x = a.x;
    }

    return Res;
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

    explicit rng(std::thread::id ThreadId)
    {
        size_t ThreadIdHash = std::hash<std::thread::id>{}(ThreadId);
        engine.seed(static_cast<unsigned int>(ThreadIdHash));
    }

    inline float random_01()
    {
        return uniform_01_dist(engine);
    }

    inline float random_neg11()
    {
        return uniform_neg11_dist(engine);
    }

    // (on the xy-plane)
    vec3 random_in_unit_disk()
    {
        vec3 position{};
        do
        {
            position = vec3(random_neg11(), random_neg11(), 0.0f);
        } while (length2(position) >= 1.0f);
        return position;
    }

    vec3 random_in_unit_sphere()
    {
        vec3 position{};
        do
        {
            position = vec3(random_neg11(), random_neg11(), random_neg11());
        } while (length2(position) >= 1.0f);
        return position;
    }

private:

    std::default_random_engine engine;
    std::uniform_real_distribution<float> uniform_01_dist{0.0f, 1.0f};
    std::uniform_real_distribution<float> uniform_neg11_dist{-1.0f, 1.0f};

};

///////////////////////////////////////////////////////////////

#endif // TRACERATOPS_MATH_H

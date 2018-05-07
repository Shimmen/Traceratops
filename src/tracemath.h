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

static inline
float square(float x)
{
    return x * x;
}

static inline
float lerp(float a, float b, float x)
{
    return (1.0f - x) * a + x * b;
}

static inline
float clamp(float x, float a, float b)
{
    return std::max(a, std::min(x, b));
}



///////////////////////////////////////////////////////////////
// vec2

struct vec2
{
    float x;
    float y;

    vec2() = default;
    vec2(vec2& other) = default;
    vec2(const vec2& other) = default;
    vec2(float x, float y) : x(x), y(y) {};
};

static inline
vec2 operator+(const vec2& a, const vec2& b)
{
    return vec2{a.x + b.x, a.y + b.y};
}

static inline
vec2 operator*(const vec2& a, float s)
{
    return vec2{a.x * s, a.y * s};
}

static inline
vec2 operator*(float s, const vec2& a)
{
    return a * s;
}

///////////////////////////////////////////////////////////////
// vec3

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
vec3& operator+=(vec3& a, const vec3& b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

static inline
vec3& operator-=(vec3& a, const vec3& b)
{
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

static inline
vec3& operator*=(vec3& a, const vec3& b)
{
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    return a;
}

static inline
vec3& operator/=(vec3& a, const vec3& b)
{
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
    return a;
}

static inline
vec3& operator*=(vec3& a, float s)
{
    a.x *= s;
    a.y *= s;
    a.z *= s;
    return a;
}

static inline
vec3& operator/=(vec3& a, float s)
{
    a.x /= s;
    a.y /= s;
    a.z /= s;
    return a;
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
void clamp(vec3 *a, float min, float max)
{
    a->x = fminf(a->x, max);
    a->y = fminf(a->y, max);
    a->z = fminf(a->z, max);

    a->x = fmaxf(a->x, min);
    a->y = fmaxf(a->y, min);
    a->z = fmaxf(a->z, min);
}

static inline
bool is_zero(const vec3& a)
{
    constexpr float eps = std::numeric_limits<float>().epsilon();
    return std::abs(a.x) < eps && std::abs(a.y) < eps && std::abs(a.z) < eps;
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
// ray

struct ray
{
    vec3 Origin;
    vec3 Direction;
};

///////////////////////////////////////////////////////////////
// general purpose geometrical math

static bool on_same_hemisphere(const vec3& Wi, const vec3& Wo, const vec3& N)
{
    return copysign(1.0f, dot(Wo, N)) == copysign(1.0f, dot(Wi, N));
}

static vec3 reflect(const vec3& I, const vec3& N)
{
    return I - N * 2.0f * dot(N, I);
}

static bool refract(const vec3& I, const vec3& N, float NiOverNt, vec3& Refracted)
{
    // From: https://en.wikipedia.org/wiki/Snell%27s_law#Vector_form

    float c = -dot(I, N);
    float r = NiOverNt;

    float Discriminant = 1.0f - r*r * (1.0f - c*c);
    if (Discriminant > 0)
    {
        Refracted = r*I + (r*c - sqrt(Discriminant)) * N;
        return true;
    }
    else
    {
        // Total internal reflection
        return false;
    }
}

static float schlick_fresnell_base(float Cosine, float R0)
{
    return R0 + (1.0f - R0) * powf(1.0f - Cosine, 5.0f);
}

static float schlick_fresnell(float Cosine, float IndexOfRefraction)
{
    // From: https://en.wikipedia.org/wiki/Schlick%27s_approximation

    // We always assume that we go between some material and air (or actually vacuum), or the other way around
    constexpr float Air = 1.0f;

    float R0 = (Air - IndexOfRefraction) / (Air + IndexOfRefraction);
    R0 = R0 * R0;

    return schlick_fresnell_base(Cosine, R0);
}

static vec3 perpendicular(const vec3& Vector)
{
    if (std::abs(Vector.x) < std::abs(Vector.y))
    {
        return vec3{0.0f, -Vector.z, Vector.y};
    }
    else
    {
        return vec3{-Vector.z, 0.0f, Vector.x};
    }
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

    explicit rng(unsigned int seed)
    {
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

    inline int random_int_in_range(int Min, int Max)
    {
        std::uniform_int_distribution<> distribution{Min, Max};
        return distribution(engine);
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

    // NOTE: Only use on the main thread!
    static rng& global_rng()
    {
        static rng Rng{};
        return Rng;
    }

private:

    std::default_random_engine engine;
    std::uniform_real_distribution<float> uniform_01_dist{0.0f, 1.0f};
    std::uniform_real_distribution<float> uniform_neg11_dist{-1.0f, 1.0f};

};

///////////////////////////////////////////////////////////////
// pixel encoding

static uint32_t encode_rgb_to_packed_pixel_int(const vec3& Rgb)
{
    constexpr float gamma = 2.2f;
    constexpr float gamma_pow = 1.0f / gamma;

    vec3 Clamped = Rgb;
    clamp(&Clamped, 0.0f, 1.0f);

    uint32_t r = 0xFF & uint32_t(powf(Clamped.x, gamma_pow) * 255.99f);
    uint32_t g = 0xFF & uint32_t(powf(Clamped.y, gamma_pow) * 255.99f);
    uint32_t b = 0xFF & uint32_t(powf(Clamped.z, gamma_pow) * 255.99f);
    uint32_t a = 0xFF;

    return (a << 24) | (b << 16) | (g << 8) | (r << 0);
}

///////////////////////////////////////////////////////////////

#endif // TRACERATOPS_MATH_H

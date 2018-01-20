#include "material.h"

using namespace tracemath;

static bool on_same_hemisphere(const vec3& Wi, const vec3& Wo, const vec3& N)
{
    return copysign(1.0f, dot(Wo, N)) == copysign(1.0f, dot(Wi, N));
}

static vec3 reflect(const vec3& I, const vec3& N)
{
    return I - N * 2.0f * dot(N, I);
}

lambertian::lambertian(const vec3& Albedo, float Emittance)
        : Albedo{Albedo}
{
    // Use albedo for emit color
    this->EmitColor = Albedo * Emittance;
}

vec3 lambertian::brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const
{
    if (dot(Wi, Hit.Normal) <= 0.0f) return vec3{0.0f};
    if (!on_same_hemisphere(Wi, Wo, Hit.Normal)) return vec3{0.0f};
    return Albedo * (1.0f / PI);
}

bool lambertian::calculate_scattered(const ray& IncomingRay, const hit_info& Hit, rng& Rng, ray& ScatteredRay) const
{
    // TODO: Use mathematically correct cosine sampling!

    ScatteredRay.Origin = Hit.Point;
    ScatteredRay.Direction = normalize(Hit.Normal + Rng.random_in_unit_sphere());
    return true;
}

metal::metal(const vec3& Albedo, float Fuzz, float Emittance)
        : Albedo{Albedo}, Fuzz{Fuzz}
{
    if (Fuzz > 1.0f) Fuzz = 1.0f;

    // Use albedo for emit color
    this->EmitColor = Albedo * Emittance;
}

vec3 metal::brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const
{
    // TODO: Implement properly!
    return Albedo;
}

bool metal::calculate_scattered(const ray& IncomingRay, const hit_info& Hit, rng& Rng, ray& ScatteredRay) const
{
    ScatteredRay.Origin = Hit.Point;
    ScatteredRay.Direction = normalize(reflect(IncomingRay.Direction, Hit.Normal) + Rng.random_in_unit_sphere() * Fuzz);
    return dot(ScatteredRay.Direction, Hit.Normal) > 0.0f;
}

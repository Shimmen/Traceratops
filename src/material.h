#ifndef TRACERATOPS_MATERIAL_H
#define TRACERATOPS_MATERIAL_H

#include "tracemath.h"
#include "geometry.h"

struct material
{
    vec3 EmitColor{};

    material() = default;
    virtual ~material() = default;

    virtual vec3 brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const = 0;

    // Scatter the incoming ray and write the result into ScatteredRay together with the probability of that specific direction
    // being chosen (pdf), and return true if it scattered or false if it was absorbed.
    virtual bool calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const = 0;

};

struct lambertian: public material
{
    vec3 Albedo{};

    explicit lambertian(const vec3& Albedo, float Emittance = 0);

    virtual vec3 brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const;
    virtual bool calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const;

};

struct metal: public material
{
    vec3 Albedo{};
    float Fuzz; // FIXME: totally PBR (from Peter Shirley's book)

    metal(const vec3& Albedo, float Fuzz, float Emittance = 0);

    vec3 brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const;
    virtual bool calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const;

};

struct dielectric: public material
{
    float IndexOfRefraction;

    explicit dielectric(float IndexOfRefraction) : IndexOfRefraction(IndexOfRefraction) {}

    virtual vec3 brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const;
    virtual bool calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const;

};

#endif // TRACERATOPS_MATERIAL_H

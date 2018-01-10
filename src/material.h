#ifndef TRACERATOPS_MATERIAL_H
#define TRACERATOPS_MATERIAL_H

#include "tracemath.h"
#include "geometry.h"

struct hit_info
{
    tracemath::vec3 Point;
    tracemath::vec3 Normal;
    int             Material;
};

struct material
{
    float Roughness;
    float Emittance;

    tracemath::vec3 Albedo;

    material(tracemath::vec3 Albedo, float Roughness, float Emittance = 0.0f)
        : Albedo(Albedo), Roughness(Roughness), Emittance(Emittance) {}
};

struct material_new
{
    tracemath::vec3 EmitColor{};

    virtual tracemath::vec3 brdf(const tracemath::vec3& Wi, const tracemath::vec3& Wo, const hit_info& Hit, tracemath::rng& Rng) = 0;

    // Scatter the incoming ray and write the result into ScatteredRay together with the probability of that specific direction
    // being chosen (pdf), and return true if it scattered or false if it was absorbed.
    virtual bool calculate_scattered(const ray& IncomingRay, const hit_info& Hit, tracemath::rng& Rng, ray& ScatteredRay/*, float& Pdf*/) = 0;

};

struct lambertian: material_new
{
    tracemath::vec3 Albedo{};

    lambertian(const tracemath::vec3& Albedo, float Emittance = 0);

    virtual tracemath::vec3 brdf(const tracemath::vec3& Wi, const tracemath::vec3& Wo, const hit_info& Hit, tracemath::rng& Rng);
    virtual bool calculate_scattered(const ray& IncomingRay, const hit_info& Hit, tracemath::rng& Rng, ray& ScatteredRay/*, float& Pdf*/);

};

struct metal: material_new
{
    tracemath::vec3 Albedo{};
    float Fuzz; // FIXME: totally PBR (from Peter Shirley's book)

    metal(const tracemath::vec3& Albedo, float Fuzz, float Emittance = 0);

    virtual tracemath::vec3 brdf(const tracemath::vec3& Wi, const tracemath::vec3& Wo, const hit_info& Hit, tracemath::rng& Rng);
    virtual bool calculate_scattered(const ray& IncomingRay, const hit_info& Hit, tracemath::rng& Rng, ray& ScatteredRay/*, float& Pdf*/);

};




#endif // TRACERATOPS_MATERIAL_H

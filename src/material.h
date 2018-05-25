#ifndef TRACERATOPS_MATERIAL_H
#define TRACERATOPS_MATERIAL_H

#include "tracemath.h"
#include "geometry.h"
#include "texture.h"

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

    virtual vec3 brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const override;
    virtual bool calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const override;

};

struct lambertian_textured: public material
{
    const texture *DiffuseTexture;

    explicit lambertian_textured(const texture *DiffuseTexture);

    vec3 brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const override;
    bool calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const override;

};

struct metal: public material
{
    vec3 Albedo{};
    float Fuzz; // FIXME: totally PBR (from Peter Shirley's book)

    metal(const vec3& Albedo, float Fuzz, float Emittance = 0);

    vec3 brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const override;
    bool calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const override;

};

struct dielectric: public material
{
    float IndexOfRefraction;

    explicit dielectric(float IndexOfRefraction) : IndexOfRefraction(IndexOfRefraction) {}

    vec3 brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const override;
    bool calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const override;

};



struct microfacet: public material
{
    const texture *DiffuseTexture;
    const texture *RoughnessTexture;
    const texture *MetalnessTexture;

    const vec3 DiffuseConst;
    const float RoughnessConst;
    const float MetalnessConst;

    microfacet(const texture *Diffuse, const texture *Roughness, const texture *Metalness)
        : DiffuseTexture(Diffuse), RoughnessTexture(Roughness), MetalnessTexture(Metalness)
        , DiffuseConst(0.1f, 0.6f, 0.78f), RoughnessConst(0.7f), MetalnessConst(0.0f) {}

    vec3 brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const override;
    bool calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const override;

private:

    float chi_ggx(float v) const;

    float ggx_geometry_function(const vec3 &Wi, const vec3 &Wo, const vec3& N, float Roughness) const;
    float ggx_partial_geometry_function(const vec3 &V, const vec3 &Wh, const vec3& N, float Roughness) const;

    float ggx_microfacet_distibution(const vec3 &Wi, const vec3 &Wo, const vec3& N, float Roughness) const;

};

#endif // TRACERATOPS_MATERIAL_H

#include "material.h"

lambertian::lambertian(const vec3& Albedo, float Emittance)
        : Albedo{Albedo}
{
    // Use albedo for emit color
    this->EmitColor = Albedo * Emittance;
}

vec3 lambertian::brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const
{
    bool Absorb = dot(Wi, Hit.Normal) <= 0.0f;
    bool NotSameHemisphere = !on_same_hemisphere(Wi, Wo, Hit.Normal);
    if (Absorb || NotSameHemisphere) return vec3{0.0f};

    return Albedo / tracemath::PI;
}

bool lambertian::calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const
{
    ScatteredRay.Origin = Hit.Point;

    // (Cosine weighted sampling)
    ScatteredRay.Direction = normalize(Hit.Normal + Rng.random_in_unit_sphere());
    Pdf = dot(ScatteredRay.Direction, Hit.Normal) / tracemath::PI;

    return true;
}

lambertian_textured::lambertian_textured(const texture *DiffuseTexture)
        : DiffuseTexture(DiffuseTexture)
{
}

vec3 lambertian_textured::brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const
{
    bool Absorb = dot(Wi, Hit.Normal) <= 0.0f;
    bool NotSameHemisphere = !on_same_hemisphere(Wi, Wo, Hit.Normal);
    if (Absorb || NotSameHemisphere) return vec3{0.0f};

    vec2 UV = Hit.TextureCoordinate;
    vec3 Albedo = DiffuseTexture->sample_texel_linear(UV.x, UV.y);
    return Albedo / tracemath::PI;
}

bool lambertian_textured::calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const
{
    ScatteredRay.Origin = Hit.Point;

    // (Cosine weighted sampling)
    ScatteredRay.Direction = normalize(Hit.Normal + Rng.random_in_unit_sphere());
    Pdf = dot(ScatteredRay.Direction, Hit.Normal) / tracemath::PI;

    return true;
}

metal::metal(const vec3& Albedo, float Fuzz, float Emittance)
        : Albedo{Albedo}, Fuzz{Fuzz}
{
    if (Fuzz > 1.0f) this->Fuzz = 1.0f;

    // Use albedo for emit color
    this->EmitColor = Albedo * Emittance;
}

vec3 metal::brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const
{
    // Divide by angle to "cancel out" cosine term in rendering equation. Technically it's not a case of canceling out
    // anything since we use Wo and N, but for a perfectly reflecting surface (as this one) dot(Wo, N) == dot(Wi, N).
    // For full information and discussion see links:
    //   https://www.reddit.com/r/pathtracing/comments/566fs4/trying_to_understand_the_cosine_term/
    //   https://www.gamedev.net/forums/topic/657520-cosine-term-in-rendering-equation/
    return Albedo / std::max(0.001f, dot(Wo, Hit.Normal));
}

bool metal::calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const
{
    ScatteredRay.Origin = Hit.Point;
    ScatteredRay.Direction = normalize(reflect(IncomingRay.Direction, Hit.Normal) + Rng.random_in_unit_sphere() * Fuzz);
    Pdf = 1.0f; // (assume perfect mirror for now)
    return dot(ScatteredRay.Direction, Hit.Normal) > 0.0f;
}

vec3 dielectric::brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const
{
    // No loss, only refraction or reflection. Divide by angle to "cancel out" cosine term in rendering equation
    // Technically it's not a case of canceling out anything since we use Wo and N, but for a perfectly reflecting
    // surface (as this one) dot(Wo, N) == dot(Wi, N). For full information and discussion see links:
    //   https://www.reddit.com/r/pathtracing/comments/566fs4/trying_to_understand_the_cosine_term/
    //   https://www.gamedev.net/forums/topic/657520-cosine-term-in-rendering-equation/
    return vec3{1, 1, 1} / std::max(0.001f, dot(Wo, Hit.Normal));
}

bool dielectric::calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const
{
    ScatteredRay.Origin = Hit.Point;

    vec3 OutwardNormal{};
    float NiOverNt;
    float Cosine;

    if (dot(IncomingRay.Direction, Hit.Normal) > 0)
    {
        // Ray is going *out of the material*, so swap the normal to point in the same hemisphere
        OutwardNormal = -Hit.Normal;
        NiOverNt = IndexOfRefraction;

        // TODO: Which one (is better)?! They both seem very similar in terms of result.
        Cosine = IndexOfRefraction * dot(IncomingRay.Direction, Hit.Normal);
        //Cosine = dot(IncomingRay.Direction, Hit.Normal);
        //Cosine = sqrt(1 - IndexOfRefraction*IndexOfRefraction * (1 - Cosine*Cosine));
    }
    else
    {
        // We are in air going to the inside of the volume, so invert the refraction index (normal is valid)
        OutwardNormal = Hit.Normal;
        NiOverNt = 1.0f / IndexOfRefraction;
        Cosine = -dot(IncomingRay.Direction, Hit.Normal);
    }

    vec3 Refracted{};
    float ReflectProbability;

    if (refract(IncomingRay.Direction, OutwardNormal, NiOverNt, Refracted))
    {
        // Refraction is possible
        ReflectProbability = schlick_fresnell(Cosine, IndexOfRefraction);
    }
    else
    {
        // Refraction not possible -> reflect
        ReflectProbability = 1.0f;
    }

    if (ReflectProbability == 1.0f || Rng.random_01() < ReflectProbability)
    {
        ScatteredRay.Direction = reflect(IncomingRay.Direction, Hit.Normal);
    }
    else
    {
        ScatteredRay.Direction = Refracted;
    }

    Pdf = 1.0f;

    // Swap the normal for evaluating the BRDF
    Hit.Normal = OutwardNormal;

    return true;
}


vec3 microfacet::brdf(const vec3& Wi, const vec3& Wo, const hit_info& Hit, rng& Rng) const
{
    float u = Hit.TextureCoordinate.x;
    float v = Hit.TextureCoordinate.y;

    float Metalness = MetalnessTexture->sample_texel_linear(u, v).x;

    // Calculate base Fresnell from how metal the material is (yes, not totally physically based)
    float R0 = lerp(0.2f, 0.95f, Metalness);

    vec3 Wh = normalize(Wi + Wo);
    float FresnelCos = std::max(0.0f, dot(Wh, Wi));

    float F = schlick_fresnell_base(FresnelCos, R0);
    float G = shadowing_function(Wi, Wo, Hit);
    float D = cook_torrance_microfacet_distibution(Wi, Wo, Hit);
    float LdotN = std::max(0.0f, dot(Hit.Normal, Wi));
    float VdotN = std::max(0.0f, dot(Hit.Normal, Wo));
    float Epsilon = 0.05f;
    float BRDF = (F * G * D) / clamp(4.0f * (VdotN * LdotN + Epsilon), 0, 1);

    vec3 Albedo = DiffuseTexture->sample_texel_linear(u, v);
    vec3 MicrofacetPart = BRDF * Albedo;
    vec3 DiffusePart = (1.0f / tracemath::PI) * Albedo;
    return (1.0f - F) * (1.0f - Metalness) * DiffusePart + MicrofacetPart;

    //return DiffuseTexture->sample_texel_linear(u, v);
    //return vec3(F);
    //return vec3(G);
    //return vec3(D);
    //if (BRDF >= 0.9f) return vec3(1, 0, 1);
    //return vec3(BRDF);
}

bool microfacet::calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const
{
    ScatteredRay.Origin = Hit.Point;

    // TODO: Importance sample!
    // Generate a random point/direction on the hemisphere
    do
    {
        ScatteredRay.Direction = Rng.random_in_unit_sphere();
    }
    while (dot(ScatteredRay.Direction, Hit.Normal) < 0.0f);

    // The most significant part of the Cook-Torrance model is the D-term, i.e  the microfacet distribution. Therefore
    // we importance sample against it and calculate the PDF though it.
    //Pdf = cook_torrance_microfacet_distibution(ScatteredRay.Direction, -IncomingRay.Direction, Hit);

    Pdf = 1.0f;

    // Lambert importance sampling
    //ScatteredRay.Direction = normalize(Hit.Normal + Rng.random_in_unit_sphere());
    //Pdf = dot(ScatteredRay.Direction, Hit.Normal) / tracemath::PI;

    return true;
}

float microfacet::shadowing_function(const vec3& Wi, const vec3& Wo, const hit_info& Hit) const
{
    // Cook-Torrance geometry function:
    vec3 Wh = normalize(Wi + Wo);
    float OutPart = dot(Hit.Normal, Wh) * dot(Hit.Normal, Wo) / dot(Wo, Wh);
    float InPart  = dot(Hit.Normal, Wh) * dot(Hit.Normal, Wi) / dot(Wo, Wh);
    //return std::min(1.0f, std::min(2.0f * InPart, 2.0f * OutPart));

    // TODO!
    return 1.0f;
}

float microfacet::cook_torrance_microfacet_distibution(const vec3 &Wi, const vec3 &Wo, const hit_info &Hit) const
{
    float Roughness = RoughnessTexture->sample_texel_linear(Hit.TextureCoordinate.x, Hit.TextureCoordinate.y).x;

    vec3 Wh = normalize(Wi + Wo);

    // TODO: Fix this mapping to shininess, it's not very good.
    float Shininess = 1.0f - Roughness;

    return ((Shininess + 2.0f) / tracemath::TWO_PI) * powf(dot(Hit.Normal, Wh), Shininess);

    //float m_Sq = Roughness * Roughness;
    //float NdotH_Sq = std::max(0.0f, dot(Hit.Normal, Wh));
    //NdotH_Sq = NdotH_Sq * NdotH_Sq;
    //return std::exp((NdotH_Sq - 1.0f) / (m_Sq*NdotH_Sq)) / (3.14159265f * m_Sq * NdotH_Sq * NdotH_Sq);
}


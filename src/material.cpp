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

    float Metalness = /*MetalnessConst;*/MetalnessTexture->sample_texel_linear(u, v).x;
    float Roughness = /*RoughnessConst;*/RoughnessTexture->sample_texel_linear(u, v).x;
    vec3 Albedo = /*DiffuseConst;*/DiffuseTexture->sample_texel_linear(u, v);

    // Scale roughness
    //Roughness *= 7.0f;

    // Use PVP plastic as default dielectric. They don't differ too much anyway.
    // IoR source: https://refractiveindex.info/?shelf=3d&book=plastics&page=pvp
    //  ... for wavelengths r=600, g=550, b=450
    const vec3 DefaultDielectricIoR = vec3(1.5268f, 1.5299, 1.5413f);
    vec3  R0 = (1.0f - DefaultDielectricIoR) / (1.0f + DefaultDielectricIoR);
    R0 = R0 * R0;
    //R0 = vec3(0.0);

    // Use normal base fresnel for dielectrics, use albedo as base fresnel for metals
    vec3 F0 = lerp(vec3(R0), Albedo, Metalness);

    vec3 N = Hit.Normal;
    vec3 Wh = normalize(Wi + Wo);
    float FresnelCos = std::max(0.0f, dot(Wh, Wo));

    vec3  F = schlick_fresnell_base(FresnelCos, F0);
    float G = ggx_geometry_function(Wi, Wo, N, Roughness);
    float D = ggx_microfacet_distibution(Wi, Wo, N, Roughness);
    float LdotN = std::max(0.0f, dot(N, Wi));
    float VdotN = std::max(0.0f, dot(N, Wo));

    float Epsilon = 0.05f;
    vec3  BRDF = (F * G * D) / clamp(4.0f * (VdotN * LdotN + Epsilon), 0, 1);

    vec3 MicrofacetPart = BRDF;
    vec3 DiffusePart = (1.0f / tracemath::PI) * Albedo;
    return (1.0f - F) * (1.0f - Metalness) * DiffusePart + MicrofacetPart;

    //return F * DiffusePart;
    //return (vec3(1.0f) - F) * DiffusePart;
    //return vec3(F);

    //return BRDF;
    //return (1.0f - F) * (1.0f - Metalness);
    //return (1.0f - F) * DiffusePart;
    //return vec3(G);
    //return MicrofacetPart;
}

bool microfacet::calculate_scattered(const ray& IncomingRay, hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const
{
    ScatteredRay.Origin = Hit.Point;
/*
    float r1 = Rng.random_01();
    float r2 = Rng.random_01();

    vec3 N = normalize(Hit.Normal);
    float Roughness = RoughnessTexture->sample_texel_linear(Hit.TextureCoordinate).x;

    float Alpha = square(Roughness);
    float Phi = tracemath::TWO_PI * r1;
    float CosTheta = sqrt((1.0f - r2) / (1.0f + (Alpha*Alpha - 1.0f) * r2 ));
    float SinTheta = sqrt(1.0f - CosTheta * CosTheta);
    vec3 H = {
        SinTheta * cos(Phi),
        SinTheta * sin(Phi),
        CosTheta
    };
    vec3 UpVector = (abs(N.z) < 0.999f) ? vec3(0, 0, 1) : vec3(1, 0, 0);
    vec3 TangentX = normalize(cross(UpVector, N));
    vec3 TangentY = cross(N, TangentX);

    // Convert tangent to world space
    vec3 SampleMicrofacetDir = normalize(TangentX * H.x + TangentY * H.y + N * H.z);

    // Create actual Wi
    vec3 Wi = reflect(IncomingRay.Direction, SampleMicrofacetDir);
    ScatteredRay.Direction = Wi;

    // Calculate the PDF
    float Denom = (Roughness * Roughness - 1.0f) * CosTheta * CosTheta + 1.0f;
    Pdf = (Roughness * Roughness * CosTheta * SinTheta) / (tracemath::PI * Denom * Denom);

    // TODO: Convert PDF from half-vector to indicent dir?! Divide by 4*OdotH
    // TODO: Is that Wh the correct one?
    vec3 Wo = -IncomingRay.Direction;
    vec3 Wh = SampleMicrofacetDir;
    Pdf /= 4.0f * dot(Wo, Wh);

    return true;
*/


/*

    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );
    float3 H;
    H.x = SinTheta * cos( Phi );
    H.y = SinTheta * sin( Phi );
    H.z = CosTheta;
    float3 UpVector = abs(N.z) < 0.999 ? float3(0,0,1) : float3(1,0,0);
    float3 TangentX = normalize( cross( UpVector, N ) );
    float3 TangentY = cross( N, TangentX );
    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;

*/



/*
    float Theta = std::acos(std::sqrt((1.0f - r1) / (r1 * (Roughness * Roughness - 1.0f) + 1.0f)));
    //std::acos(std::sqrt((1.0f - r1) / (r1 * (Roughness * Roughness - 1.0f) + 1.0f)));
    //std::atan(Roughness * std::sqrt(r1 / (1.0f - r1)));
    float Phi = tracemath::TWO_PI * r2;

    vec3 SampleMicrofacetDir = spherical_to_vector(Theta, Phi);
    vec3 Wi = reflect(IncomingRay.Direction, SampleMicrofacetDir);

    // On correct hemisphere?
    if (Wi.y > 0.0f && dot(Wi, SampleMicrofacetDir) > 0.0f)
    {
        // Calculate the scattered direction in world space
        mat3 TBN = generate_isotropic_tbn(Hit.Normal);
        Wi = normalize(TBN * Wi);
        ScatteredRay.Direction = Wi;

        // Calculate the PDF
        float CosTheta = cos(Theta);
        float Denom = (Roughness * Roughness - 1.0f) * CosTheta * CosTheta + 1.0f;
        Pdf = (Roughness * Roughness * CosTheta * sin(Theta)) / (tracemath::PI * Denom * Denom);

        // TODO: Convert PDF from half-vector to indicent dir?! Divide by 4*OdotH
        // TODO: Is that Wh the correct one?
        vec3 Wo = -IncomingRay.Direction;
        vec3 Wh = normalize(Wo + Wi);
        Pdf /= 4.0f * dot(Wo, Wh);

        return true;
    }
    else
    {
        return false;
    }
*/

    // Generate a random point/direction on the hemisphere
    //do
    //{
    //    ScatteredRay.Direction = Rng.random_in_unit_sphere();
    //}
    //while (dot(ScatteredRay.Direction, Hit.Normal) < 0.0f);

    // The most significant part of the Cook-Torrance model is the D-term, i.e  the microfacet distribution. Therefore
    // we importance sample against it and calculate the PDF though it.
    //Pdf = cook_torrance_microfacet_distibution(ScatteredRay.Direction, -IncomingRay.Direction, Hit);


    // Lambert importance sampling
    ScatteredRay.Direction = normalize(Hit.Normal + Rng.random_in_unit_sphere());
    Pdf = dot(ScatteredRay.Direction, Hit.Normal) / tracemath::PI;
    return true;

}

float microfacet::chi_ggx(float v) const
{
    return (v > 0.0f) ? 1.0f : 0.0f;
}

float microfacet::ggx_geometry_function(const vec3 &Wi, const vec3 &Wo, const vec3& N, float Roughness) const
{
    vec3 Wh = normalize(Wi + Wo);

    float Gin  = ggx_partial_geometry_function(Wi, Wh, N, Roughness);
    float Gout = ggx_partial_geometry_function(Wo, Wh, N, Roughness);

    return Gin * Gout;
}

float microfacet::ggx_partial_geometry_function(const vec3 &V, const vec3 &Wh, const vec3& N, float Roughness) const
{
    float VdotH = clamp(dot(V, Wh), 0.0f, 1.0f);
    float VdotN = clamp(dot(V, N), 0.0f, 1.0f);
    float Chi = chi_ggx(VdotH / VdotN);
    float VdotHSq = VdotH * VdotH;
    float Tan2 = (1.0f - VdotHSq) / VdotHSq;
    return (Chi * 2.0f) / (1.0f + std::sqrt(1.0f + Roughness * Roughness * Tan2));
}

float microfacet::ggx_microfacet_distibution(const vec3 &Wi, const vec3 &Wo, const vec3& N, float Roughness) const
{
    vec3 Wh = normalize(Wi + Wo);
    float NdotH = dot(N, Wh);
    /*
    // my original:
    float NdotHSq = NdotH * NdotH;
    float AlphaSq = Roughness * Roughness;
    float Denom = NdotHSq * AlphaSq + (1.0f - NdotHSq);
    return (AlphaSq * chi_ggx(NdotH)) / (tracemath::PI * Denom * Denom);
    */

    float Alpha = Roughness;
    float CosSquared = NdotH * NdotH;
    float TanSquared = (1.0f - CosSquared) / CosSquared;
    return (1.0f / tracemath::PI) * square(Alpha / (CosSquared * (Alpha * Alpha + TanSquared)));

}


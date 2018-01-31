#include "material.h"

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

static float SchlickFresnell(float Cosine, float IndexOfRefraction)
{
    // From: https://en.wikipedia.org/wiki/Schlick%27s_approximation

    // We always assume that we go between some material and air (or actually vacuum), or the other way around
    constexpr float Air = 1.0f;

    float R0 = (Air - IndexOfRefraction) / (Air + IndexOfRefraction);
    R0 = R0 * R0;

    return R0 + (1.0f - R0) * powf(1.0f - Cosine, 5.0f);
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
    return Albedo / tracemath::PI;
}

bool lambertian::calculate_scattered(const ray& IncomingRay, const hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const
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

bool metal::calculate_scattered(const ray& IncomingRay, const hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const
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

bool dielectric::calculate_scattered(const ray& IncomingRay, const hit_info& Hit, rng& Rng, ray& ScatteredRay, float& Pdf) const
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
        ReflectProbability = SchlickFresnell(Cosine, IndexOfRefraction);
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

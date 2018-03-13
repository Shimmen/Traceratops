#include "basic_renderer.h"

basic_renderer::basic_renderer(int RaysPerPixel, int MaxRayDepth)
    : RaysPerPixel(RaysPerPixel), MaxRayDepth(MaxRayDepth)
{

}

void basic_renderer::render_scene(const scene &Scene, const camera &Camera, image &Image) const
{
    if (!Scene.is_prepared_for_rendering())
    {
        printf("Scene not prepared for rendering! Call prepare_for_rendering() on scene before rendering. Aborting!\n");
        return;
    }

    rng Rng{};

    for (uint32_t y = 0; y < Image.Height; ++y)
    {
        for (uint32_t x = 0; x < Image.Width; ++x)
        {
            vec3 AccumulatedHdrColor = vec3{};
            for (int i = 0; i < RaysPerPixel; ++i)
            {
                // (jittered UVs)
                float u = (x + 0.5f + 0.5f * Rng.random_neg11()) / Image.Width;
                float v = (y + 0.5f + 0.5f * Rng.random_neg11()) / Image.Height;
                const ray& Ray = Camera.get_ray(u, v, Rng);

                vec3 Color = trace_ray(Ray, Scene, Rng);
                AccumulatedHdrColor = AccumulatedHdrColor + Color;
            }

            AccumulatedHdrColor = AccumulatedHdrColor * (1.0f / RaysPerPixel);
            vec3 LdrColor = tone_map_hdr_to_ldr(AccumulatedHdrColor);

            uint32_t Pixel = encode_rgb_to_packed_pixel_int(LdrColor);
            Image.set_pixel(x, y, Pixel);
        }

        Image.update_window();

        float PercentDone = 100.0f * y / Image.Height;
        printf("... %f%% done ...\r", PercentDone);
        fflush(stdout);
    }

    printf("... 100%% done ...\n");
}

bool
basic_renderer::get_first_intersection(const scene& Scene, const ray& Ray, float MinT, float MaxT, hit_info *Hit) const
{
#if USE_NAIVE_INTERSECTION_TEST

    float MinDistance = INFINITY;
    hit_info CurrentHit{};

    for (int i = 0; i < Scene.Hitables.size(); ++i)
    {
        auto Hitable = Scene.Hitables[i];
        if (Hitable->intersect(Ray, MinT, MaxT, CurrentHit))
        {
            if (CurrentHit.Distance < MinDistance)
            {
                *Hit = CurrentHit;
                MinDistance = CurrentHit.Distance;
            }
        }
    }

    return MinDistance < INFINITY;

#else

    return Scene.BVHRootNode->intersect(Ray, MinT, MaxT, *Hit);

#endif
}

vec3
basic_renderer::trace_ray(ray Ray, const scene& Scene, rng& Rng) const
{
    vec3 BounceAttenuation = vec3{1, 1, 1};
    vec3 ResultColor = vec3{0, 0, 0};

    hit_info Hit{};
    for (int CurrentDepth = 0; CurrentDepth < MaxRayDepth; ++CurrentDepth)
    {
        if (get_first_intersection(Scene, Ray, 0.001f, INFINITY, &Hit))
        {
            const material& Material = Scene.get_material(Hit.Material);

            // Add direct light shining directly from a light source to the camera
            if (CurrentDepth == 0)
            {
                ResultColor = ResultColor + Material.EmitColor;
            }

            // Direct light evaluated at Hit.Position
            {
                hitable *LightSource{};
                float MaxDistance;

                // Use an offset origin and but 0 as MinT so we don't accidentally miss ourself. Usually we want to
                // avoid hitting ourself so this is therefore treated a bit like a special case.
                vec3 Origin = Hit.Point + (0.01f * Hit.Normal);

                ray LightRay = get_light_ray(Origin, Scene, Rng, &LightSource, &MaxDistance);

                // TODO: First test shadow cache!

                hit_info LightRayHit{};
                bool DidHitAnything = get_first_intersection(Scene, LightRay, 0.0f, MaxDistance, &LightRayHit);

                if (!DidHitAnything)
                {
                    // TODO: Check to make sure we're not just at a very steep gracing angle!!!
                    // Because the only situation where I get this to happen (in cornell box) is at rays that originate
                    // from y=1.98, i.e. same y-coordinate as the light source so that the ray will be perfectly
                    // orthogonal to the light source. In that case this is okay, since the cosine term would evaluate
                    // to 0 in any case. BUT, it would be good if we could notice if this isn't the case somehow. For a
                    // triangle this is easy but I don't think it's possible in the general case. Maybe we will just
                    // have to convince ourselves after a while that this actually works...

                    //printf("Light ray didn't hit anything (at pos %f %f %f), not even the light source! Something might be wrong!\n", Origin.x, Origin.y, Origin.z);
                }

                if (LightRayHit.Hitable == LightSource)
                {

                    auto& LightMaterial = Scene.get_material(LightSource->Material);
                    vec3 LightEmittance = LightMaterial.EmitColor;

                    vec3 Wi = normalize(LightRay.Direction);
                    vec3 Wo = normalize(-Ray.Direction);
                    vec3 BRDF = Material.brdf(Wi, Wo, Hit, Rng);

                    float DistanceAttenuation = 1.0f / square(LightRayHit.Distance);
                    float CosineAttenuation = std::abs(dot(LightRay.Direction, Hit.Normal)); // TODO: clamp here?!

                    vec3 DirectLight = BounceAttenuation * BRDF * CosineAttenuation * DistanceAttenuation * LightEmittance;
                    ResultColor = ResultColor + DirectLight;

                    // TODO: Apply the PDF from the get_light_ray function!
                }
                else
                {
                    // TODO: Save the hit object for the shadow cache!
                }

            }

            ray Scattered{};
            float PDF;

            if (Material.calculate_scattered(Ray, Hit, Rng, Scattered, PDF))
            {
                // Bounce the ray on this surface

                float CosineAttenuation = std::abs(dot(Scattered.Direction, Hit.Normal));
                vec3 BRDF = Material.brdf(Scattered.Direction, -Ray.Direction, Hit, Rng);
                BounceAttenuation = BounceAttenuation * BRDF * CosineAttenuation / PDF;

                Ray = Scattered;

            }
            else
            {
                // Ray absorbed, kill this path

                return vec3{0.0f};
            }

        }
        else
        {
            if (Scene.EnvironmentMap)
            {
                const vec3& Dir = Ray.Direction;

                // Convert direction to spherical coordinates
                float Theta = acosf(fmaxf(-1.0f, fminf(1.0f, Dir.y)));
                float Phi = atan2f(Dir.z, Dir.x);
                if (Phi < 0.0f) Phi += tracemath::TWO_PI;

                float u = Phi / tracemath::TWO_PI;
                float v = Theta / tracemath::PI;

                // (nearest might be okay here since we trace multiple rays per pixel)
                vec3 EnvironmentColor = Scene.EnvironmentMap->sample_texel_nearest(u, v);
                ResultColor = ResultColor + (BounceAttenuation * EnvironmentColor * Scene.EnvironmentMultiplier);
            }

            break;
        }
    }

    return ResultColor;
}

ray
basic_renderer::get_light_ray(const vec3& Origin, const scene& Scene, rng& Rng, hitable **LightSource, float *MaxDistance) const
{
    // Pick random emitting hitable (i.e. light source)
    auto& Lights = Scene.EmittingHitables;
    auto NumLights = int(Lights.size());

    if (NumLights == 0)
    {
        printf("There has to be at least one light source in the scene! Aborting!\n");
        std::exit(1);
    }

    int Index = Rng.random_int_in_range(0, NumLights - 1);
    *LightSource = Lights[Index];

    // Pick random point on light source

    // TODO: Make this work for any hitable, or something like that..?
    auto *Triangle = dynamic_cast<triangle *>(*LightSource);
    assert(Triangle);
    assert(Triangle == *LightSource);

    float u, v;
    do
    {
        u = Rng.random_01();
        v = Rng.random_01();
    } while (u + v > 1.0f);
    vec3 Point = Triangle->V0 + u * (Triangle->V1 - Triangle->V0) + v * (Triangle->V2 - Triangle->V0);
    vec3 ToPoint = Point - Origin;

    // Add some extra margin distance so we don't miss the light source
    *MaxDistance = length(ToPoint) + 0.001f;

    // TODO: Importance sample (i.e. weight by hitable size)

    ray LightRay{};
    LightRay.Direction = normalize(ToPoint);
    LightRay.Origin = Origin;
    return LightRay;
}

vec3
basic_renderer::tone_map_hdr_to_ldr(const vec3& Hdr) const
{
    // TODO: Use better tonemap!!!
    return Hdr / (Hdr + vec3(1, 1, 1));
}

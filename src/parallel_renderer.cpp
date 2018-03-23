#include "parallel_renderer.h"

#include "timer.h"

parallel_renderer::parallel_renderer(int RaysPerPixel, int MaxRayDepth)
        : RaysPerPixel(RaysPerPixel), MaxRayDepth(MaxRayDepth)
{

}

void parallel_renderer::render_tiles(const parallel_renderer *Renderer, work_context *WorkContext, rng *Rng, bool WorkLeader)
{
    size_t NumWorkOrders = WorkContext->WorkOrders.size();
    int CurrentIndex;

    while ((CurrentIndex = WorkContext->WorkOrderIndex.fetch_add(1)) < NumWorkOrders)
    {
        tile_work_order& CurrentWorkOrder = WorkContext->WorkOrders[CurrentIndex];

        int CountX = CurrentWorkOrder.MaxX - CurrentWorkOrder.MinX;
        int CountY = CurrentWorkOrder.MaxY - CurrentWorkOrder.MinY;

        for (int y = 0; y < CountY; ++y)
        {
            for (int x = 0; x < CountX; ++x)
            {
                int GlobalX = x + CurrentWorkOrder.MinX;
                int GlobalY = y + CurrentWorkOrder.MinY;

                for (int i = 0; i < CurrentWorkOrder.NumSamplesPerPixel; ++i)
                {
                    const ray& Ray = WorkContext->Camera->get_jittered_ray(GlobalX, GlobalY, WorkContext->Image->Width, WorkContext->Image->Height, *Rng);

                    vec3 Color = Renderer->trace_ray(Ray, *WorkContext->Scene, *Rng);
                    CurrentWorkOrder.AccumulatedContribution[x + y * CountX] += Color;
                }

            }
        }

        WorkContext->apply_finished_work_order(CurrentWorkOrder);

        if (WorkLeader)
        {
            float PercentDone = 100.0f * float(CurrentIndex + 1) / float(NumWorkOrders);
            float AverageSamples = 0.0f;
            const image *Image = WorkContext->generate_image_at_current_state(&AverageSamples);

            char StatusMessage[1024];
            sprintf(StatusMessage, "%f%% done (~%.1f samples per pixel)\r", PercentDone, AverageSamples);

            Image->update_window(false, StatusMessage);

            printf("%.2f%% done (~%.1f SPP)\r", PercentDone, AverageSamples);
        }
    }
}

void parallel_renderer::render_scene(const scene &Scene, const camera &Camera, image &Image) const
{
    if (!Scene.is_prepared_for_rendering())
    {
        printf("Scene not prepared for rendering! Call prepare_for_rendering() on scene before rendering. Aborting!\n");
        return;
    }

    // Default to 8 threads (i.e. assume 4 cores + hyperthreading)
    unsigned HardwareConcurrency = std::thread::hardware_concurrency();
    int NumThreadsToUse = (HardwareConcurrency != 0) ? HardwareConcurrency : 8;

    // Create thread-local RNGs
    typedef std::chrono::high_resolution_clock myclock;
    myclock::time_point Now = myclock::now();
    unsigned int BaseSeed = static_cast<unsigned int>(Now.time_since_epoch().count());

    // Setup work context
    int TileSideSize = 64;
    int SamplesPerWorkOrder = 1;
    auto WorkContext = new work_context{&Image, &Scene, &Camera, RaysPerPixel, MaxRayDepth, TileSideSize, SamplesPerWorkOrder};

    printf("-------------\n- rendering -\n-------------\n");

    // Do an initial update so that a blank/black window shows up
    Image.update_window();

    timer Timer{};
    Timer.start();

    // Spawn threads
    std::vector<std::thread> Threads{};
    for (int i = 1; i < NumThreadsToUse; ++i)
    {
        auto Rng = new rng{BaseSeed + i};
        Threads.emplace_back(parallel_renderer::render_tiles, this, WorkContext, Rng, false);
    }

    // Assign the work leader to the main thread and join all others when finished
    auto ThisThreadRng = new rng{BaseSeed};
    render_tiles(this, WorkContext, ThisThreadRng, true);
    for (auto& Thread: Threads) Thread.join();

    Timer.end();
    printf("100%% done\n");

    // Perform final image generation so that it is valid for saving etc.
    WorkContext->generate_image_at_current_state();

    printf("-------------\n--- stats ---\n-------------\n");
    printf(" - total time elapsed = %.3fs\n", Timer.get_seconds_elapsed());
    printf(" - average time elapsed per ray = %lldns\n", Timer.get_nanoseconds_elapsed_per_iteration());
    printf(" - mega paths (+light rays) / second = %.3f\n", Timer.get_mega_iterations_per_second());
    printf(" - Mrays/s = %.3f\n", Timer.get_mega_iterations_per_second() * 16.0);
    printf("-------------\n");

}

bool
parallel_renderer::get_first_intersection(const scene& Scene, const ray& Ray, float MinT, float MaxT, hit_info *Hit) const
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
parallel_renderer::trace_ray(ray Ray, const scene& Scene, rng& Rng) const
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
                float LightDistance;

                // Use an offset origin and but 0 as MinT so we don't accidentally miss ourself. Usually we want to
                // avoid hitting ourself so this is therefore treated a bit like a special case.
                vec3 Origin = Hit.Point + (0.01f * Hit.Normal);

                ray LightRay = get_light_ray(Origin, Scene, Rng, &LightSource, &LightDistance);
                hit_info LightRayHit{};

                bool DidHitAnything = get_first_intersection(Scene, LightRay, 0.0f, LightDistance + 0.001f, &LightRayHit);

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
                else if (LightRayHit.Hitable == LightSource)
                {
                    auto& LightMaterial = Scene.get_material(LightSource->Material);
                    vec3 LightEmittance = LightMaterial.EmitColor;

                    vec3 Wi = normalize(LightRay.Direction);
                    vec3 Wo = normalize(-Ray.Direction);
                    vec3 BRDF = Material.brdf(Wi, Wo, Hit, Rng);

                    float DistanceAttenuation = 1.0f / square(LightRayHit.Distance);
                    float CosineAttenuation = std::abs(dot(LightRay.Direction, Hit.Normal)); // TODO: clamp here?! relevant for refraction?

                    vec3 DirectLight = BounceAttenuation * BRDF * CosineAttenuation * DistanceAttenuation * LightEmittance;
                    ResultColor = ResultColor + DirectLight;

                    // TODO: Apply the PDF from the get_light_ray function!
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
parallel_renderer::get_light_ray(const vec3& Origin, const scene& Scene, rng& Rng, hitable **LightSource, float *MaxDistance) const
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
    *MaxDistance = length(ToPoint);

    // TODO: Importance sample (i.e. weight by hitable size)

    ray LightRay{};
    LightRay.Direction = normalize(ToPoint);
    LightRay.Origin = Origin;
    return LightRay;
}

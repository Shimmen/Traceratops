#ifndef TRACERATOPS_PARALLEL_RENDERER_H
#define TRACERATOPS_PARALLEL_RENDERER_H

#include "renderer.h"

#include <thread>
#include <atomic>
#include <mutex>

struct parallel_renderer: public renderer
{
public:

    parallel_renderer(int RaysPerPixel, int MaxRayDepth);
    virtual ~parallel_renderer() = default;

    void render_scene(const scene& Scene, const camera& Camera, image& Image) const override;

private:

    bool get_first_intersection(const scene& Scene, const ray& Ray, float MinT, float MaxT, rng& Rng, hit_info *Hit) const override;
    vec3 trace_ray(ray Ray, const scene& Scene, rng& Rng) const override;

    ray get_light_ray(const vec3& Origin, const scene& Scene, rng& Rng, hitable **LightSource, float *MaxDistance) const;

    static vec3 tone_map_hdr_to_ldr(const vec3& Hdr)
    {
        return Hdr / (Hdr + vec3(1, 1, 1));
    }

    int RaysPerPixel;
    int MaxRayDepth;

private:

    struct tile_work_order
    {
        tile_work_order() = default;
        ~tile_work_order() = default;

        // Defines tile boundaries (assert that (MaxX-MinX)*(MaxY-MinY)<=TILE_SIZE and that they are within the image bounds)
        int MinX, MaxX;
        int MinY, MaxY;

        // Accumulate all HDR samples (i.e. results from trace_ray) into here
        std::vector<vec3> AccumulatedContribution{};

        // How many samples should be taken per pixel?
        int NumSamplesPerPixel;

    };

    struct work_context
    {
        // Image to render to
        image *Image;

        const scene *Scene;
        const camera *Camera;

        // Max number of *bounces* for a path
        int MaxRayDepth;

        // All work orders to perform
        std::vector<tile_work_order> WorkOrders{};

        // The next work order index to perform
        std::atomic_int WorkOrderIndex{};

        // Shared, accumulated data and mutex object
        std::mutex SharedAccumulationDataMutex;
        std::vector<vec3> AccumulatedContribution{};
        std::vector<int> NumSamples{};

        work_context(image *Image, const scene *Scene, const camera *Camera, int NumSamplesPerPixel, int MaxRayDepth, int TileSize, int SamplesPerWorkOrder)
            : Image(Image), Scene(Scene), Camera(Camera), MaxRayDepth(MaxRayDepth)
        {
            int ImageSize = Image->Width * Image->Height;
            AccumulatedContribution.resize(ImageSize);
            NumSamples.resize(ImageSize);

            // (round up)
            int NumTilesX = (Image->Width + TileSize - 1) / TileSize;
            int NumTilesY = (Image->Height + TileSize - 1) / TileSize;
            int NumWorkOrdersPerTile = (NumSamplesPerPixel + SamplesPerWorkOrder - 1) / SamplesPerWorkOrder;

            std::random_device RandomDevice;
            std::mt19937 RandomGenerator(RandomDevice());

            for (int i = 0; i < NumWorkOrdersPerTile; ++i)
            {
                for (int TileY = 0; TileY < NumTilesY; ++TileY)
                {
                    for (int TileX = 0; TileX < NumTilesX; ++TileX)
                    {
                        int MinX = TileX * TileSize;
                        int MinY = TileY * TileSize;
                        assert(MinX < Image->Width);
                        assert(MinY < Image->Height);

                        int MaxX = std::min(MinX + TileSize, Image->Width);
                        int MaxY = std::min(MinY + TileSize, Image->Height);
                        assert(MaxX - MinX > 0);
                        assert(MaxY - MinY > 0);

                        tile_work_order WorkOrder{};
                        WorkOrder.AccumulatedContribution.resize(TileSize * TileSize);
                        WorkOrder.NumSamplesPerPixel = SamplesPerWorkOrder;
                        WorkOrder.MinX = MinX;
                        WorkOrder.MinY = MinY;
                        WorkOrder.MaxX = MaxX;
                        WorkOrder.MaxY = MaxY;

                        WorkOrders.push_back(WorkOrder);
                    }
                }

                auto ShuffleStart = WorkOrders.begin() + i * (NumTilesX * NumTilesY);
                auto ShuffleEnd = WorkOrders.begin() + (i + 1) * (NumTilesX * NumTilesY);
                std::shuffle(ShuffleStart, ShuffleEnd, RandomGenerator);
            }

        }

        void apply_finished_work_order(const tile_work_order& WorkOrder)
        {
            std::lock_guard<std::mutex> Lock(SharedAccumulationDataMutex);

            int CountX = WorkOrder.MaxX - WorkOrder.MinX;
            int CountY = WorkOrder.MaxY - WorkOrder.MinY;

            for (int y = 0; y < CountY; ++y)
            {
                for (int x = 0; x < CountX; ++x)
                {
                    int GlobalX = x + WorkOrder.MinX;
                    int GlobalY = y + WorkOrder.MinY;

                    int LocalIndex = x + y * CountX;
                    int GlobalIndex = GlobalX + GlobalY * Image->Width;

                    // Accumulate contribution per pixel
                    AccumulatedContribution[GlobalIndex] += WorkOrder.AccumulatedContribution[LocalIndex];
                    NumSamples[GlobalIndex] += WorkOrder.NumSamplesPerPixel;
                }
            }
        }

        const image *generate_image_at_current_state(float *AverageSamples = nullptr)
        {
            std::lock_guard<std::mutex> Lock(SharedAccumulationDataMutex);

            int TotalSamples = 0;

            for (int y = 0; y < Image->Height; ++y)
            {
                for (int x = 0; x < Image->Width; ++x)
                {
                    int Index = x + y * Image->Width;

                    if (NumSamples[Index] > 0)
                    {
                        TotalSamples += NumSamples[Index];

                        vec3 CurrentAverageHdr = AccumulatedContribution[Index] / NumSamples[Index];
                        vec3 CurrentLdrColor = tone_map_hdr_to_ldr(CurrentAverageHdr);
                        uint32_t CurrentPixel = encode_rgb_to_packed_pixel_int(CurrentLdrColor);
                        Image->set_pixel(x, y, CurrentPixel);
                    }
                    else
                    {
                        Image->set_pixel(x, y, 0x0);
                    }
                }
            }

            if (AverageSamples)
            {
                *AverageSamples = float(TotalSamples) / float(Image->Width * Image->Height);
            }

            return Image;
        }

    };

    static void render_tiles(const parallel_renderer *Renderer, work_context *WorkContext, rng *Rng, bool WorkLeader);

};

#endif // TRACERATOPS_PARALLEL_RENDERER_H

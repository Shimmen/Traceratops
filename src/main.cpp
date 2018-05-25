#include <string>

#include "image.h"
#include "scene.h"
#include "camera.h"
#include "debug_renderer.h"
#include "basic_renderer.h"
#include "parallel_renderer.h"

using namespace tracemath;

void setup_pbr_demo(scene& Scene)
{
    Scene.register_triangle_mesh("assets/cornell_box/", "CornellBox-Empty.obj", vec3{0.0f, 0.0f, 1.0f}, 1.7f);
    //Scene->register_triangle_mesh("assets/cerberus/", "cerberus.obj", vec3{0.9f, 1.3f, 0.2f}, 1.7f, -90.0f);

    auto DiffuseTexture = new texture{"assets/cerberus/textures/Cerberus_A.jpg"};
    auto RoughnessTexture = new texture{"assets/cerberus/textures/Cerberus_R.jpg"};
    auto MetalnessTexture = new texture{"assets/cerberus/textures/Cerberus_M.jpg"};
    int BPRMaterial = Scene.register_material(new microfacet{DiffuseTexture, RoughnessTexture, MetalnessTexture});

    Scene.add_hitable(new sphere{
            vec3{0, 1.2f, 0},
            1.0, BPRMaterial
    });
}

void setup_moving_sphere_demo(scene& Scene)
{
    Scene.register_triangle_mesh("assets/cornell_box/", "CornellBox-Empty.obj", vec3{0.0f, 0.0f, -0.75f}, 1.7f);

    int Material = Scene.register_material(new lambertian{{0.1f, 0.6f, 0.78f}});
    Scene.add_hitable(new moving_sphere{
            vec3{-1.2f, 0.3f, -1.8f}, 0.35f,
            vec3{0.10f, 0.06f, 0.17f},
            Material
    });
}

std::unique_ptr<scene> create_and_setup_scene()
{
    std::unique_ptr<scene> Scene(new scene{});

    Scene->EnvironmentMap = std::unique_ptr<texture>(new texture{"assets/environment.hdr"});
    Scene->EnvironmentMultiplier = 0.3f;

    //setup_pbr_demo(*Scene);
    setup_moving_sphere_demo(*Scene);

    return Scene;
}

int main()
{
#define QUALITY 1

#if QUALITY == 0
    image Image{1080, 1080};
    int RaysPerPixel = 256;
    int RayMaxDepth = 8;
#elif QUALITY == 1
    image Image{720, 720};
    int RaysPerPixel = 64;
    int RayMaxDepth = 8;
#elif QUALITY == 2
    image Image{288, 196};
    int RaysPerPixel = 1024;
    int RayMaxDepth = 8;
#elif QUALITY == 3
    image Image{200, 200};
    int RaysPerPixel = 16;
    int RayMaxDepth = 4;
#elif QUALITY == 4
    image Image{400, 400};
    int RaysPerPixel = 16;
    int RayMaxDepth = 4;
#endif

#if 0
    std::string ImageFileName = "traceratops_render.png";
    printf("Traceratops - rendering %dx%d image '%s'\n", Image.Width, Image.Height, ImageFileName.c_str());

    auto Scene = create_and_setup_scene();
    Scene->prepare_for_rendering();

    float ApertureSize = 0.15f;
    //camera Camera{vec3{0, 1, -2}, vec3{0.75f, 0, 1}, vec3{0, 1, 0}, Image, 90, ApertureSize};
    camera Camera{vec3{0, 1, 2.2f}, vec3{0, 1, 0}, vec3{0, 1, 0}, Image, 75, ApertureSize};

    //debug_renderer Renderer{};
    //basic_renderer Renderer{RaysPerPixel, RayMaxDepth};
    parallel_renderer Renderer{RaysPerPixel, RayMaxDepth};
    Renderer.render_scene(*Scene, Camera, Image);

    printf("Traceratops - rendering done, writing to file");
    Image.write_to_png(ImageFileName);
    Image.update_window(true);
#else

    std::string ImageFileNameBase = "traceratops_animation_";
    printf("Traceratops - rendering %dx%d image '%s'\n", Image.Width, Image.Height, ImageFileNameBase.c_str());

    auto Scene = create_and_setup_scene();
    Scene->prepare_for_rendering();

    int FramesPerSecond = 25;
    float DeltaTime = 1.0f / float(FramesPerSecond);

    int NumFrames = 25;
    for (int Frame = 0; Frame < NumFrames; Frame += 1)
    {
        printf(" - frame %d/%d\n", Frame + 1, NumFrames);

        float ApertureSize = 0.15f;
        camera Camera{vec3{0, 1, 2.2f}, vec3{0, 1, 0}, vec3{0, 1, 0}, Image, 75, ApertureSize};

        parallel_renderer Renderer{RaysPerPixel, RayMaxDepth};
        Renderer.render_scene(*Scene, Camera, Image);

        Scene->new_frame();
        Scene->prepare_for_rendering();

        printf(" - frame done, writing to file\n");

        std::string ImageFileName = ImageFileNameBase + std::to_string(Frame) + ".png";
        Image.write_to_png(ImageFileName);
        Image.update_window();
    }

#endif

    return 0;
}

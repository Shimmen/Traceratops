#include <string>

#include "image.h"
#include "scene.h"
#include "camera.h"
#include "rendering.h"

using namespace tracemath;

std::unique_ptr<scene> create_and_setup_scene()
{
    std::unique_ptr<scene> Scene(new scene{});

    Scene->EnvironmentMap = std::unique_ptr<texture>(new texture{"assets/environment.hdr"});
    Scene->EnvironmentMultiplier = 1.5f;

    //Scene->register_triangle_mesh("assets/lowpoly_tree.obj", vec3{0.1f, 0.6f, 3.0f});

    int DiffuseRedMaterial = Scene->register_material(new lambertian{vec3{1.0, 0.1, 0.1}});
    int GreenMetalMaterial = Scene->register_material(new metal{vec3{0.4, 1.0, 0.4}, 0.6f});
    int DiffuseGrayMaterial = Scene->register_material(new lambertian{vec3{0.6, 0.6, 0.6}});
    int MirrorMaterial = Scene->register_material(new metal{vec3{1, 1, 1}, 0.0});
    int LightMaterial = Scene->register_material(new lambertian{vec3{1.0, 0.7, 0.7}, 10.0});

    // Diffuse red ball
    Scene->Spheres.emplace_back(
        vec3{0.75f, 0, 1},
        1.0, DiffuseRedMaterial
    );

    // Diffuse green ball
    Scene->Spheres.emplace_back(
        vec3{1.25f, -1.0f, 0.0f},
        0.4, GreenMetalMaterial
    );

    // Mirror ball
    Scene->Spheres.emplace_back(
        vec3{-1.1f, 0.4f, 2},
        0.75, MirrorMaterial
    );

    // Light source
    Scene->Discs.emplace_back(
        vec3{+4.2f, -0.7f, 2.2f},
        make_direction(-1.0f, +0.0f, -0.5f),
        1.25, LightMaterial
    );

    // Floor plane
    Scene->Discs.emplace_back(
        vec3{0, -1.0f, 0},
        make_direction(0, 1.0f, 0),
        10.0f, DiffuseGrayMaterial
    );

    return Scene;
}

int main()
{
#define QUALITY 2

#if QUALITY == 0
    image Image{1920, 1080};
    int RaysPerPixel = 1024;
    int RayMaxDepth = 8;
#elif QUALITY == 1
    image Image{720, 480};
    int RaysPerPixel = 64;
    int RayMaxDepth = 4;
#elif QUALITY == 2
    image Image{288, 196};
    int RaysPerPixel = 1024;
    int RayMaxDepth = 4;
#elif QUALITY == 3
    image Image{288, 196};
    int RaysPerPixel = 16;
    int RayMaxDepth = 2;
#endif

    std::string ImageFileName = "traceratops_render.png";
    printf("Traceratops - rendering %dx%d image '%s'\n", Image.Width, Image.Height, ImageFileName.c_str());

    auto Scene = create_and_setup_scene();
    Scene->prepare_for_rendering();

    camera Camera{vec3{0, 1, -2}, vec3{0.75f, 0, 1}, vec3{0, 1, 0}, Image, 90};

    render_scene(*Scene, Camera, Image, RaysPerPixel, RayMaxDepth);

    printf("Traceratops - rendering done, writing to file");
    Image.write_to_png(ImageFileName);

    return 0;
}

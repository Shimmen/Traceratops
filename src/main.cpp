#include <string>

#include "image.h"
#include "scene.h"
#include "rendering.h"

using namespace tracemath;

std::unique_ptr<scene> create_and_setup_scene()
{
    std::unique_ptr<scene> Scene(new scene{});

    Scene->EnvironmentMap = std::unique_ptr<texture>(new texture{"assets/environment.hdr"});
    //Scene->set_environment_map(std::move(EnvironmentMap));
    Scene->EnvironmentMultiplier = 10.0f;

    int DiffuseRedMaterial = Scene->register_material(material{vec3{1.0, 0.1, 0.1}, 0.8});
    int DiffuseGreenMaterial = Scene->register_material(material{vec3{0.4, 1.0, 0.4}, 0.7});
    int DiffuseGrayMaterial = Scene->register_material(material{vec3{0.8, 0.8, 0.8}, 0.6});
    int MirrorMaterial = Scene->register_material(material{vec3{1, 1, 1}, 0.0});
    int LightMaterial = Scene->register_material(material{vec3{1.0, 0.8, 0.8}, 1.0, 20.0});

    // Diffuse red ball
    Scene->Spheres.emplace_back(
        vec3{0.75f, 0, 3},
        1.0, DiffuseRedMaterial
    );

    // Diffuse green ball
    Scene->Spheres.emplace_back(
        vec3{1.25f, -1.0f, 1.95f},
        0.4, DiffuseGreenMaterial
    );

    // Mirror ball
    Scene->Spheres.emplace_back(
        vec3{-1.1f, 0.4f, 3},
        0.75, MirrorMaterial
    );

    // Light source
    Scene->Spheres.emplace_back(
        vec3{0, 1.0, 1.4},
        0.35, LightMaterial
    );

    // Floor plane
    Scene->Planes.emplace_back(
        vec3{0, -1.0f, 0},
        make_direction(0, 1.0f, 0),
        DiffuseGrayMaterial
    );

    // Back plane/wall
    Scene->Planes.emplace_back(
            vec3{0, 0, 10},
            make_direction(0, 0.5f, -1.0f),
            DiffuseGrayMaterial
    );

    return Scene;
}

int main()
{
#if 1
    image Image{1920, 1080};
    int RaysPerPixel = 1024;//2048;
    int RayMaxDepth = 8;
#else
    image Image{720, 480};
    int RaysPerPixel = 64;
    int RayDepth = 4;
#endif

    std::string ImageFileName = "traceratops_render.png";
    printf("Traceratops - rendering %dx%d image '%s'\n", Image.Width, Image.Height, ImageFileName.c_str());

    auto Scene = create_and_setup_scene();
    render_scene(Scene.get(), &Image, RaysPerPixel, RayMaxDepth);

    printf("Traceratops - rendering done, writing to file");
    Image.write_to_png(ImageFileName);

    return 0;
}

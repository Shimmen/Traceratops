#include <stdio.h>
#include <string>

#include "image.h"
#include "scene.h"
#include "rendering.h"

using namespace tracemath;

scene create_and_setup_scene()
{
    scene Scene{};
    Scene.LightDirection = make_direction(3, -1, 1);

    int DiffuseRedMaterial = Scene.register_material(material{vec3{1.0, 0.1, 0.1}, 0.8});
    int DiffuseGrayMaterial = Scene.register_material(material{vec3{0.8, 0.8, 0.8}, 0.6});
    int MirrorMaterial = Scene.register_material(material{vec3{1, 1, 1}, 0.0});
    int LightMaterial = Scene.register_material(material{vec3{1.0, 0.8, 0.8}, 1.0, 50.0});

    // Diffuse ball
    Scene.Spheres.emplace_back(
        vec3{0.75f, 0, 3},
        1.0, DiffuseRedMaterial
    );

    // Mirror ball
    Scene.Spheres.emplace_back(
        vec3{-1.3f, 0, 3},
        0.75, MirrorMaterial
    );

    // Light source
    Scene.Spheres.emplace_back(
        vec3{0, 1.3, 1.4},
        0.2, LightMaterial
    );

    // Floor plane
    Scene.Planes.emplace_back(
        vec3{0, -1.0f, 0},
        make_direction(0, 1.0f, 0),
        DiffuseGrayMaterial
    );

    // Back plane/wall
    Scene.Planes.emplace_back(
            vec3{0, 0, 100},
            make_direction(0, 0, -1.0f),
            DiffuseGrayMaterial
    );

    return Scene;
}

int main()
{
#if 1
    image Image{1920, 1080};
    int RaysPerPixel = 4196;
    int RayDepth = 8;
#else
    image Image{720, 480};
    int RaysPerPixel = 128;
    int RayDepth = 4;
#endif

    std::string ImageFileName = "traceratops_render.png";
    printf("Traceratops - rendering %dx%d image '%s'\n", Image.Width, Image.Height, ImageFileName.c_str());

    scene Scene = create_and_setup_scene();
    render_scene(&Scene, &Image, RaysPerPixel, RayDepth);

    printf("Traceratops - rendering done, writing to file");
    Image.write_to_png(ImageFileName);

    return 0;
}

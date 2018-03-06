#include <string>

#include "image.h"
#include "scene.h"
#include "camera.h"
#include "basic_renderer.h"

using namespace tracemath;

std::unique_ptr<scene> create_and_setup_scene()
{
    std::unique_ptr<scene> Scene(new scene{});

    Scene->EnvironmentMap = std::unique_ptr<texture>(new texture{"assets/environment.hdr"});
    Scene->EnvironmentMultiplier = 1.5f;

    Scene->register_triangle_mesh("assets/cornell_box/CornellBox-Original.obj", vec3{0.0f, 0.0f, 0.0f});

    int DiffuseRedMaterial  = Scene->register_material(new lambertian{vec3{1.0, 0.1, 0.1}});
    int GreenMetalMaterial  = Scene->register_material(new metal{vec3{0.4, 1.0, 0.4}, 0.2f});
    int DiffuseGrayMaterial = Scene->register_material(new lambertian{vec3{0.6, 0.6, 0.6}});
    int MirrorMaterial      = Scene->register_material(new metal{vec3{1, 1, 1}, 0.0});
    int ClearGlassMaterial  = Scene->register_material(new dielectric{1.5f});
    int LightMaterial       = Scene->register_material(new lambertian{vec3{1.0, 0.7, 0.7}, 10.0});
/*
    // Diffuse red ball
    Scene->add_hitable(new sphere{
        vec3{0.75f, 0, 1},
        1.0, ClearGlassMaterial//DiffuseRedMaterial
    });

    // Diffuse green ball
    Scene->add_hitable(new sphere{
        vec3{1.25f, -1.0f, 0.0f},
        0.4, GreenMetalMaterial
    });

    // Mirror ball
    Scene->add_hitable(new sphere{
        vec3{-1.1f, 0.4f, 2},
        0.75, MirrorMaterial
    });

    // Light source
    Scene->add_hitable(new disc{
        vec3{+4.2f, -0.7f, 2.2f},
        make_direction(-1.0f, +0.0f, -0.5f),
        1.25, LightMaterial
    });

    // Floor plane
    Scene->add_hitable(new disc{
        vec3{0, -1.0f, 0},
        make_direction(0, 1.0f, 0),
        10.0f, DiffuseGrayMaterial
    });

    // Out of focus ball
    Scene->add_hitable(new sphere{
        vec3{-0.4f, 0.75f, -1.7f},
        0.20f, DiffuseRedMaterial
    });
    */
/*
    srand(0L);
    for (int i = 0; i < 100000; i++) {

        float r = float(rand()) / float(RAND_MAX);
        float g = float(rand()) / float(RAND_MAX);
        float b = float(rand()) / float(RAND_MAX);
        float e = float(rand()) / float(RAND_MAX) * 5.0f;

        int Material = Scene->register_material(new lambertian{vec3{r, g, b}, e});

        float x = float(rand()) / float(RAND_MAX) * 8.0f - 4.0f;
        float y = float(rand()) / float(RAND_MAX) * 3.0f;
        float z = float(rand()) / float(RAND_MAX) * 6.0f + 2.0f;
        float rad = float(rand()) / float(RAND_MAX) * 0.3f + 0.2f;

        Scene->add_hitable(new sphere{
            vec3{x, y, z}, rad, Material
        });
    }
*/
/*
    // Glass sphere
    Scene->add_hitable(new sphere{
        vec3{0.25f, 0.75f, -0.7f},
        0.75f, ClearGlassMaterial
    });
*/
    return Scene;
}

int main()
{
#define QUALITY 3

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
    int RayMaxDepth = 8;
#elif QUALITY == 3
    image Image{288, 196};
    int RaysPerPixel = 16;
    int RayMaxDepth = 4;
#endif

    std::string ImageFileName = "traceratops_render.png";
    printf("Traceratops - rendering %dx%d image '%s'\n", Image.Width, Image.Height, ImageFileName.c_str());

    auto Scene = create_and_setup_scene();
    Scene->prepare_for_rendering();

    float ApertureSize = 0.05f;
    //camera Camera{vec3{0, 1, -2}, vec3{0.75f, 0, 1}, vec3{0, 1, 0}, Image, 90, ApertureSize};
    camera Camera{vec3{0, 1, 1.3f}, vec3{0, 1, 0}, vec3{0, 1, 0}, Image, 90, ApertureSize};

    basic_renderer Renderer{RaysPerPixel, RayMaxDepth};
    Renderer.render_scene(*Scene, Camera, Image);

    printf("Traceratops - rendering done, writing to file");
    Image.write_to_png(ImageFileName);
    Image.update_window(true);

    return 0;
}

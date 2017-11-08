#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>

#include "image.h"
#include "scene.h"
#include "rendering.h"

using namespace tracemath;

scene create_and_setup_scene()
{
    scene Scene{};
    Scene.LightDirection = make_direction(3, -1, 1);

    Scene.Spheres.emplace_back(
        vec3{0.75f, 0, 2},
        1.0f
    );

    Scene.Spheres.emplace_back(
        vec3{-1.3f, 0, 2},
        0.75f
    );

    Scene.Planes.emplace_back(
        vec3{0, -0.5f, 0},
        make_direction(0, 1.0f, 0)
    );

    return Scene;
}

int main()
{
    // TODO: Better randomness!
    srand(time(NULL));

    std::string ImageFileName = "traceratops_render.png";
    image Image{1024, 720};
    printf("Traceratops - rendering %dx%d image '%s'\n", Image.Width, Image.Height, ImageFileName.c_str());

    scene Scene = create_and_setup_scene();

    int RaysPerPixel = 8;
    render_scene(&Scene, &Image, RaysPerPixel);

    printf("Traceratops - rendering done, writing to file");
    Image.write_to_png(ImageFileName);

    return 0;
}

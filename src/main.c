#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "image.h"
#include "scene.h"
#include "rendering.h"

scene create_and_setup_scene()
{
    scene Scene = scene_make(2, 1);

    Scene.Spheres[0].P = vec3_make(0.75f, 0, 2);
    Scene.Spheres[0].r = 1.0f;

    Scene.Spheres[1].P = vec3_make(-1.3f, 0, 2);
    Scene.Spheres[1].r = 0.75f;

    Scene.Planes[0].P = vec3_make(0, -0.5f, 0);
    Scene.Planes[0].N = vec3_direction(0, 1.0f, 0);

    Scene.LightDirection = vec3_direction(3, -1, 1);

    return Scene;
}

int main()
{
    // TODO: Better randomness!
    srand(time(NULL));

    char *ImageFileName = "traceratops_render.png";
    image Image = image_make(1024, 720);
    printf("Traceratops - rendering %dx%d image '%s'\n", Image.Width, Image.Height, ImageFileName);

    scene Scene = create_and_setup_scene();

    int RaysPerPixel = 8;
    render_scene(&Scene, &Image, RaysPerPixel);

    printf("Traceratops - rendering done, saving file");
    image_write_png(&Image, ImageFileName);

    image_release(&Image);
    scene_release(&Scene);

    return 0;
}

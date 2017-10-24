#include <stdio.h>

#include "image.h"
#include "scene.h"
#include "rendering.h"

void setup_scene(scene *Scene)
{
    // TODO: Set up the scene
}

int main()
{
    char *ImageFileName = "traceratops_render.png";
    image Image = image_make(1024, 720);
    printf("Traceratops - rendering %dx%d image '%s'\n", Image.Width, Image.Height, ImageFileName);

    scene Scene = {};
    setup_scene(&Scene);

    int RaysPerPixel = 8;
    render_scene(&Scene, &Image, RaysPerPixel);

    printf("Traceratops - rendering done, saving file");
    image_write_png(&Image, ImageFileName);
    return 0;
}

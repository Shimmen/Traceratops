#include "rendering.h"

void make_test_image(image *Image)
{
    for (uint32_t y = 0; y < Image->Height; ++y) {
        for (uint32_t x = 0; x < Image->Width; ++x) {

            uint32_t a = 0xFF000000;
            uint32_t b = ((x + y) % 0xFF) << 16;
            uint32_t g = (y % 0xFF) << 8;
            uint32_t r = (x % 0xFF) << 0;

            uint32_t PixelValue = r | g | b | a;
            image_set_pixel(Image, x, y, PixelValue);
        }
    }

    image_set_pixel(Image, 0, 0, 0xFFFFFFFF);
    image_set_pixel(Image, 0, Image->Height - 1, 0xFFFFFFFF);
    image_set_pixel(Image, Image->Width - 1, 0, 0xFFFFFFFF);
    image_set_pixel(Image, Image->Width - 1, Image->Height - 1, 0xFFFFFFFF);
}

void render_scene(scene *Scene, image *Image, int RaysPerPixel)
{
    make_test_image(Image);

/*
    for (uint32_t y = 0; y < Image->Height; ++y) {
        for (uint32_t x = 0; x < Image->Width; ++x) {

            // TODO: Perform ray tracing!

        }
    }
*/
}

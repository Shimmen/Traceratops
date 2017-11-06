#include <stdlib.h>

#include "image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

image image_make(int Width, int Height)
{
    image Image = {};

    Image.Width = Width;
    Image.Height = Height;

    size_t ImageSize = Width * Height * sizeof(uint32_t);
    Image.Pixels = malloc(ImageSize);

    return Image;
}

void image_release(image *Image)
{
    if (Image && Image->Pixels)
    {
        free(Image->Pixels);
    }
}

void image_set_pixel(image *Image, int x, int y, uint32_t Pixel)
{
    assert(x >= 0 && x < Image->Width);
    assert(y >= 0 && y < Image->Height);

    Image->Pixels[x + y * Image->Width] = Pixel;
}

void image_write_png(image *Image, const char *FileName)
{
    int NumComponents = 4;
    int Stride = Image->Width * sizeof(uint32_t);
    stbi_write_png(FileName, Image->Width, Image->Height, NumComponents, Image->Pixels, Stride);
}

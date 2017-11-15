#include "image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

image::image(int Width, int Height)
    : Width(Width), Height(Height)
{
    size_t ImageSize = Width * Height * sizeof(uint32_t);
    Pixels.resize(ImageSize);
}

void
image::set_pixel(int x, int y, uint32_t Pixel)
{
    assert(x >= 0 && x < Width);
    assert(y >= 0 && y < Height);

    Pixels[x + y * Width] = Pixel;
}

void
image::write_to_png(const std::string& FileName)
{
    assert(Width > 0 && Height > 0);

    int NumComponents = 4;
    int Stride = Width * sizeof(Pixels[0]);

    stbi_write_png(FileName.c_str(), Width, Height, NumComponents, Pixels.data(), Stride);
}

#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

texture::texture(const std::string& FileName)
{
    // We use OpenGL convention where uv (0,0) is the bottom-left corner
    stbi_set_flip_vertically_on_load(true);

    IsHdrTexture = static_cast<bool>(stbi_is_hdr(FileName.c_str()));

    if (IsHdrTexture)
    {
        PixelsHDR = stbi_loadf(FileName.c_str(), &Width, &Height, nullptr, STBI_rgb);
    }
    else
    {
        PixelsLDR = stbi_load(FileName.c_str(), &Width, &Height, nullptr, STBI_rgb);
    }

    if (!(PixelsHDR || PixelsLDR))
    {
        printf("Failed to load image '%s'!\n", FileName.c_str());
    }
}

texture::~texture()
{
    if (IsHdrTexture)
    {
        if (PixelsHDR)
        {
            stbi_image_free(PixelsHDR);
        }
    }
    else
    {
        if (PixelsLDR)
        {
            stbi_image_free(PixelsLDR);
        }
    }
}

vec3 texture::sample_texel_nearest(float u, float v) const
{
    // Assume repeat for now
    u = u - floorf(u);
    v = v - floorf(v);

    int x = static_cast<int>(floorf(u * Width));
    int y = static_cast<int>(floorf(v * Height));

    return texel_fetch(x, y);
}

vec3 texture::sample_texel_linear(float u, float v) const
{
    // Assume repeat for now
    u = u - floorf(u);
    v = v - floorf(v);

    float x = u * Width;
    float y = v * Height;

    int x0 = static_cast<int>(x);
    int y0 = static_cast<int>(y);

    float dX = x - x0;
    float dY = y - y0;

    vec3 BL = texel_fetch(x0 + 0, y0 + 0);
    vec3 BR = texel_fetch(x0 + 1, y0 + 0);
    vec3 TL = texel_fetch(x0 + 0, y0 + 1);
    vec3 TR = texel_fetch(x0 + 1, y0 + 1);

    vec3 T = (1.0f - dX) * TL + dX * TR;
    vec3 B = (1.0f - dX) * BL + dX * BR;
    vec3 Result = (1.0f - dY) * B + dY * T;

    return Result;
}

vec3
texture::texel_fetch(int x, int y) const
{
    int i = (x + y * Width) * STBI_rgb;

    if (IsHdrTexture)
    {
        return vec3{ PixelsHDR[i], PixelsHDR[i + 1], PixelsHDR[i + 2] };
    }
    else
    {
        return vec3{ PixelsLDR[i] / 255.0f, PixelsLDR[i + 1] / 255.0f, PixelsLDR[i + 2] / 255.0f };
    }
}

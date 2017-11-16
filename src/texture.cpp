#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace tracemath;

texture::texture(const std::string& FileName)
{
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

tracemath::vec3
texture::sample_texel_nearest(float u, float v) const
{
    // Assume repeat for now
    u = u - floorf(u);
    v = v - floorf(v);

    int s = static_cast<int>(floorf(u * Width));
    int t = static_cast<int>(floorf(v * Height));

    int i = (s + t * Width) * STBI_rgb;

    if (IsHdrTexture)
    {
        return vec3{ PixelsHDR[i], PixelsHDR[i + 1], PixelsHDR[i + 2] };
    }
    else
    {
        return vec3{ PixelsHDR[i] / 255.0f, PixelsHDR[i + 1] / 255.0f, PixelsHDR[i + 2] / 255.0f };
    }
}

tracemath::vec3
texture::sample_texel_linear(float u, float v) const
{
    // TODO: Implement!
    assert(false);
    return vec3{1, 0, 1};
}

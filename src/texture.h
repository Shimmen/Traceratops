#ifndef TRACERATOPS_TEXTURE_H
#define TRACERATOPS_TEXTURE_H

#include <string>

#include "tracemath.h"

class texture
{
public:

    explicit texture(const std::string& FileName);
    texture() = default;
    ~texture();


    texture(texture& Other) = delete;
    texture operator=(texture& Other) = delete;

    int get_width() const { return Width; }
    int get_height() const { return Height; }

    tracemath::vec3 sample_texel_nearest(float u, float v) const;
    tracemath::vec3 sample_texel_linear(float u, float v) const;

private:

    bool IsHdrTexture;

    uint8_t *PixelsLDR;
    float   *PixelsHDR;

    int Width;
    int Height;

};

#endif // TRACERATOPS_TEXTURE_H

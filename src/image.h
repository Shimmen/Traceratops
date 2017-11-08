#ifndef TRACERATOPS_IMAGE_H
#define TRACERATOPS_IMAGE_H

#include <cstdint>
#include <string>

class image
{
public:

    const int Width;
    const int Height;

    image(int Width, int Height);
    ~image();

    image(image& Other) = delete;
    image operator=(const image& Other) = delete;

    void set_pixel(int x, int y, uint32_t Pixel);
    void write_to_png(const std::string& FileName);

private:

    uint32_t *Pixels;

};

#endif // TRACERATOPS_IMAGE_H

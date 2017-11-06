#ifndef TRACERATOPS_IMAGE_H
#define TRACERATOPS_IMAGE_H

#include <stdint.h>

typedef struct image
{
    int Width;
    int Height;
    uint32_t *Pixels;
} image;

image image_make(int Width, int Height);
void image_release(image *Image);

void image_set_pixel(image *Image, int x, int y, uint32_t Pixel);
void image_write_png(image *Image, const char *FileName);

#endif // TRACERATOPS_IMAGE_H

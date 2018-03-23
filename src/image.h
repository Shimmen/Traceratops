#ifndef TRACERATOPS_IMAGE_H
#define TRACERATOPS_IMAGE_H

#include <cstdint>
#include <vector>
#include <string>

#include <GLFW/glfw3.h>

class image
{
public:

    const int Width;
    const int Height;

    image(int Width, int Height);
    ~image() = default;

    void set_pixel(int x, int y, uint32_t Pixel);

    void update_window(bool WaitForExit = false, const char *StatusMessage = nullptr) const;
    void write_to_png(const std::string& FileName) const;

private:

    std::vector<uint32_t> Pixels;
    GLFWwindow *Window;

};

#endif // TRACERATOPS_IMAGE_H

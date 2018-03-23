#include "image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

image::image(int Width, int Height)
    : Width(Width), Height(Height)
{
    Pixels.resize(Width * Height);

    // Setup window
    if (glfwInit())
    {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        Window = glfwCreateWindow(Width, Height, "Traceratops preview", nullptr, nullptr);
        glfwMakeContextCurrent(Window);

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, Width, Height);
    }
}

void
image::set_pixel(int x, int y, uint32_t Pixel)
{
    assert(x >= 0 && x < Width);
    assert(y >= 0 && y < Height);

    Pixels[x + y * Width] = Pixel;
}

void
image::update_window(bool WaitForExit, const char *StatusMessage) const
{
    // TODO: Seems to compensate for the retina screen..?
    glPixelZoom(2.0f, 2.0f);

    glDrawPixels(Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, Pixels.data());
    assert(glGetError() == GL_NO_ERROR);

    glfwSwapBuffers(Window);
    glfwPollEvents();

    if (StatusMessage)
    {
        glfwSetWindowTitle(Window, StatusMessage);
    }

    if (WaitForExit)
    {
        while (!glfwWindowShouldClose(Window))
        {
            glfwWaitEvents();
        }
        glfwDestroyWindow(Window);
        glfwTerminate();
    }
    else
    {
        if (glfwWindowShouldClose(Window))
        {
            glfwDestroyWindow(Window);
            glfwTerminate();
            std::exit(0);
        }
    }
}

void
image::write_to_png(const std::string& FileName) const
{
    assert(Width > 0 && Height > 0);

    int NumComponents = 4;
    int Stride = Width * sizeof(Pixels[0]);

    // Create a y-flipped image for writing to since stbi needs the first pixel to be top-left
    std::vector<uint32_t> FlippedPixels;
    FlippedPixels.resize(Width * Height);

    for (uint32_t y = 0; y < Height; ++y)
    {
        for (uint32_t x = 0; x < Width; ++x)
        {
            uint32_t FlippedY = Height - y - 1;
            FlippedPixels[x + Width * y] = Pixels[x + Width * FlippedY];
        }
    }

    stbi_write_png(FileName.c_str(), Width, Height, NumComponents, FlippedPixels.data(), Stride);
}

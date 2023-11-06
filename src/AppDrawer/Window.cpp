#include "Window.h"

#include <cstdint>
#include <cstring>
#include <string>

#include "Consts.h"

Window::Window(std::string title, uint32_t width, uint32_t height, uint32_t id) noexcept
{
    this->title = title;
    area.width = width;
    area.height = height;
    area.x = (float)WIDTH / 2 - (float)width / 2;
    area.y = (float)HEIGHT / 2 - (float)height / 2;
    this->id = id;

    auto pixelsSize = width * height * COMPONENTS;
    pixels = (uint8_t*)std::malloc(pixelsSize);
    std::memset(pixels, 0xFF, pixelsSize);
}

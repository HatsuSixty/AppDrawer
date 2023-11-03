#pragma once

#include <cstdint>
#include <iostream>

#include <raylib.h>

class Window {
public:
    std::string title;
    uint8_t* pixels;
    uint32_t id;
    Rectangle area;

    Window(std::string title, uint32_t width, uint32_t height, uint32_t id) noexcept;
};

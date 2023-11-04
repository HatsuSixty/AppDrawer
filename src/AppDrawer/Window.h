#pragma once

#include <cstdint>
#include <iostream>
#include <map>

#include <raylib.h>
#include <unordered_map>

struct WindowEvents {
    bool isPolling;
};

class Window {
public:
    std::string title;
    uint8_t* pixels;
    uint32_t id;
    Rectangle area;
    WindowEvents events;

    Window(std::string title, uint32_t width, uint32_t height, uint32_t id) noexcept;
};

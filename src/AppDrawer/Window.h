#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

#include <raylib.h>

#include "RudeDrawer.h"

struct WindowEvents {
    bool isPolling;
    std::vector<RudeDrawerEvent> events;
};

class Window {
public:
    std::string title;
    uint8_t* pixels;
    uint32_t id;
    Rectangle area;
    WindowEvents events;
    bool active;

    Window(std::string title, uint32_t width, uint32_t height, uint32_t id) noexcept(true);
};

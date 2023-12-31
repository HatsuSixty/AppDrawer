#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

#include <raylib.h>

#include "RudeDrawer.h"

struct WindowEvents {
    bool isPolling;
    bool running = true;
    std::vector<RudeDrawerEvent> events;
};

class Window {
public:
    std::string title;

    uint8_t* pixels;
    std::string pixelsShmName;
    int pixelsShmFd;
    size_t pixelsShmSize;

    uint32_t id;
    Rectangle area;
    WindowEvents events;
    bool isDragging;

    Window(std::string title, uint32_t width, uint32_t height, uint32_t id) noexcept(false);
    void sendEvent(RudeDrawerEvent event) noexcept(true);
    void destroy() noexcept(false);
};

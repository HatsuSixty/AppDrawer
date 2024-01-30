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
    std::string m_title;

    uint8_t* m_pixels;
    std::string m_pixelsShmName;
    int m_pixelsShmFd;
    size_t m_pixelsShmSize;

    uint32_t m_id;
    Rectangle m_area;
    WindowEvents m_events;
    bool m_isDragging;

    Window(std::string title, uint32_t width, uint32_t height, uint32_t id) noexcept(false);
    void sendEvent(RudeDrawerEvent event) noexcept(true);
    void destroy() noexcept(false);
};

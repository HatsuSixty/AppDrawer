#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

#include <raylib.h>

#include "RudeDrawer.h"

#include "ErrorHandling.h"

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
    int m_pixelsShmSize;

    uint32_t m_id;
    Rectangle m_area;
    WindowEvents m_events;
    bool m_isDragging;

    static Result<void*, Window*> create(std::string title, uint32_t width, uint32_t height, uint32_t id);

    void sendEvent(RudeDrawerEvent event) noexcept(true);
    Result<void*, void*> destroy();
};

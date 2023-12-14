#pragma once

#include <cstdint>
#include <string>

// Display.h - Defines the `Display` class.

class Display {
private:
    int pixelsShmFd;
    size_t pixelsShmSize;
    uint32_t windowId;
public:
    // A pointer to RGBA data.
    uint8_t* pixels;

    Display(std::string name, uint32_t width, uint32_t height, uint32_t id) noexcept(false);
    void destroy() noexcept(false);
};

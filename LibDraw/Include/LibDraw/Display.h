#pragma once

#include <cstdint>
#include <string>

// Display.h - Defines the `Display` class.

class Display {
private:
    int m_pixelsShmFd;
    size_t m_pixelsShmSize;
    uint32_t m_windowId;
public:
    // A pointer to RGBA data.
    uint8_t* m_pixels;

    Display(std::string name, uint32_t width, uint32_t height, uint32_t id) noexcept(false);
    void destroy() noexcept(false);
};

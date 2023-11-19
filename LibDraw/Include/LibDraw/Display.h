#pragma once

#include <cstdint>
#include <string>

class Display {
private:
    int pixelsShmFd;
    size_t pixelsShmSize;
    uint32_t windowId;
public:
    uint8_t* pixels;

    Display(std::string name, uint32_t width, uint32_t height, uint32_t id) noexcept(false);
    void destroy() noexcept(false);
};

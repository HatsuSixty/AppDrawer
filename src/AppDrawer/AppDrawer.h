#pragma once

#include <cstdint>
#include <vector>

#include "Window.h"

class AppDrawer {
private:
    uint32_t windowId = 1;
    int fd;

    void listener() noexcept;
    void handleClient(int clientFd) noexcept;
    void pollEvents(Window* window, int clientFd) noexcept;
public:
    std::vector<Window> windows;

    size_t findWindow(uint32_t id);
    uint32_t addWindow(std::string title, uint32_t width, uint32_t height) noexcept;
    void removeWindow(uint32_t id);
    void setWindowPolling(uint32_t id, bool polling);
    void startServer();

    ~AppDrawer() noexcept;
};

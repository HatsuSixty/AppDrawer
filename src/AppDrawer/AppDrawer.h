#pragma once

#include <cstdint>
#include <vector>

#include "Window.h"

class AppDrawer {
private:
    uint32_t windowId = 1;
    int fd;

    void listener() noexcept;
    void handleClient(int client_fd) noexcept;
    void pollEvents(Window* window) noexcept;
public:
    std::vector<Window> windows;

    uint32_t addWindow(std::string title, uint32_t width, uint32_t height) noexcept;
    void removeWindow(uint32_t id);
    void setWindowPolling(uint32_t id, bool polling);
    void startServer();

    ~AppDrawer() noexcept;
};

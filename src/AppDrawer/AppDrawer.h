#pragma once

#include <cstdint>
#include <vector>
#include <mutex>

#include "RudeDrawer.h"
#include "Window.h"

class AppDrawer {
private:
    uint32_t windowId = 1;
    int fd;

    void listener() noexcept(true);
    void handleClient(int clientFd) noexcept(true);
    void pollEvents(Window* window, int clientFd) noexcept(true);
public:
    std::mutex windowsMutex;
    std::vector<Window*> windows;

    size_t findWindow(uint32_t id) noexcept(false);
    uint32_t addWindow(std::string title, RudeDrawerVec2D dims) noexcept(false);
    void removeWindow(uint32_t id) noexcept(false);
    void setWindowPolling(uint32_t id, bool polling) noexcept(false);
    void changeActiveWindow(uint32_t id) noexcept(false);
    void startServer() noexcept(false);

    ~AppDrawer() noexcept(true);
};

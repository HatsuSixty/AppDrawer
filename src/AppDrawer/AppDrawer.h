#pragma once

#include <cstdint>
#include <vector>
#include <mutex>

#include "RudeDrawer.h"
#include "Window.h"

class Connection {
private:
    int sockfd;
public:
    Connection(int sockfd) noexcept(true);
    bool receiveOrFail(void* data, size_t n) noexcept(true);
    bool sendOrFail(void* data, size_t n) noexcept(true);
    bool sendErrOrFail(RudeDrawerErrorKind err) noexcept(true);
};

class AppDrawer {
private:
    uint32_t windowId = 1;
    int fd;

    void listener() noexcept(true);
    void handleClient(int clientFd) noexcept(true);
    void pollEvents(Window* window, Connection& conn) noexcept(true);
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

#pragma once

#include <cstdint>
#include <raylib.h>
#include <vector>
#include <mutex>
#include <unordered_set>

#include "RudeDrawer.h"
#include "Window.h"

enum ClientResult {
    CLIENT_CLOSED,
    CLIENT_OK,
    CLIENT_ERR,
};

class Client {
private:
    int m_sockfd;

public:
    Client(int sockfd) noexcept(true);
    ClientResult receiveOrFail(void* data, size_t n) noexcept(true);
    ClientResult sendOrFail(void* data, size_t n) noexcept(true);
    ClientResult sendErrOrFail(RudeDrawerErrorKind err) noexcept(true);
};

class AppDrawer {
private:
    uint32_t m_windowId = 1;
    int m_fd;
    Vector2 m_mousePos;
    Vector2 m_previousMousePos;
    std::unordered_set<uint32_t> m_windowsWithEventSockets;

    void listener() noexcept(true);
    void handleClient(int clientFd) noexcept(true);
    void pollEvents(Window* window) noexcept(true);

public:
    std::mutex m_windowsMutex;
    std::vector<Window*> m_windows;

    void setMousePosition(Vector2 mousePos);
    size_t findWindow(uint32_t id) noexcept(false);
    uint32_t addWindow(std::string title, RudeDrawerVec2D dims) noexcept(false);
    void removeWindow(uint32_t id) noexcept(false);
    void setWindowPolling(uint32_t id, bool polling) noexcept(false);
    void changeActiveWindow(uint32_t id) noexcept(false);
    void startServer() noexcept(false);

    ~AppDrawer() noexcept(true);
};

#pragma once

#include <cstdint>
#include <vector>
#include <mutex>

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

    void listener() noexcept(true);
    void handleClient(int clientFd) noexcept(true);
    void pollEvents(Window* window, Client& client) noexcept(true);
public:
    std::mutex m_windowsMutex;
    std::vector<Window*> m_windows;

    size_t findWindow(uint32_t id) noexcept(false);
    uint32_t addWindow(std::string title, RudeDrawerVec2D dims) noexcept(false);
    void removeWindow(uint32_t id) noexcept(false);
    void setWindowPolling(uint32_t id, bool polling) noexcept(false);
    void changeActiveWindow(uint32_t id) noexcept(false);
    void startServer() noexcept(false);

    ~AppDrawer() noexcept(true);
};

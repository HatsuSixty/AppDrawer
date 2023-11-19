#pragma once

#include <cstdint>
#include <string>

#include "RudeDrawer.h"
#include "Display.h"

class Draw {
private:
    int socket;

    void send(void* data, size_t n) noexcept(false);
    void recv(void* data, size_t n) noexcept(false);
public:
    void connect() noexcept(false);
    void ping() noexcept(false);
    uint32_t addWindow(std::string title, RudeDrawerVec2D dims, bool alwaysUpdating) noexcept(false);
    void removeWindow(uint32_t id) noexcept(false);
    void startPollingEventsWindow(uint32_t id) noexcept(false);
    void stopPollingEventsWindow(uint32_t id) noexcept(false);
    void sendPaintEvent(uint32_t id) noexcept(false);
    Display* getDisplay(uint32_t id, RudeDrawerVec2D dims) noexcept(false);
    RudeDrawerEvent pollEvent() noexcept(false);

    ~Draw() noexcept(true);
};

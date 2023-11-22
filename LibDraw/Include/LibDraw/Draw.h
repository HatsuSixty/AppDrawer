#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "RudeDrawer.h"
#include "Display.h"

typedef void (*DrawCallbackFunction)(void*);
struct DrawCallback {
    DrawCallbackFunction callback;
    void* parameters;
};

class Draw {
private:
    int socket;
    std::unordered_map<uint32_t, DrawCallback> callbacks;

    void send(void* data, size_t n) noexcept(false);
    void recv(void* data, size_t n) noexcept(false);
public:
    void connect() noexcept(false);
    void ping() noexcept(false);
    uint32_t addWindow(std::string title, RudeDrawerVec2D dims, bool alwaysUpdating) noexcept(false);
    void setPaintCallback(uint32_t id, DrawCallbackFunction callback, void* params) noexcept(true);
    void removePaintCallback(uint32_t id) noexcept(true);
    void removeWindow(uint32_t id) noexcept(false);
    void startPollingEventsWindow(uint32_t id) noexcept(false);
    void stopPollingEventsWindow(uint32_t id) noexcept(false);
    void sendPaintEvent(uint32_t id) noexcept(false);
    Display* getDisplay(uint32_t id, RudeDrawerVec2D dims) noexcept(false);
    RudeDrawerEvent pollEvent(uint32_t id) noexcept(false);

    ~Draw() noexcept(true);
};

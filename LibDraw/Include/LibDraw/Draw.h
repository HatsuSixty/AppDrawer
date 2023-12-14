#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "RudeDrawer.h"
#include "Display.h"

// Draw.h - Defines all LibDraw functions.

typedef void (*DrawCallbackFunction)(void*);
struct DrawCallback {
    DrawCallbackFunction callback;
    void* parameters;
    bool running = true;
    bool shouldQuit = false;
};

// This class is used for communication with the AppDrawer server.
class Draw {
private:
    int socket;
    std::unordered_map<uint32_t, DrawCallback*> callbacks;

    void send(void* data, size_t n) noexcept(false);
    void recv(void* data, size_t n) noexcept(false);
public:
    // Connects to the AppDrawer server.
    void connect() noexcept(false);
    // Makes the server print `Pong!` in its logs.
    void ping() noexcept(false);
    // Adds a window.
    uint32_t addWindow(std::string title, RudeDrawerVec2D dims, bool alwaysUpdating) noexcept(false);
    // Sets the callback that will be called everytime a window needs to be updated.
    void setPaintCallback(uint32_t id, DrawCallbackFunction callback, void* params) noexcept(true);
    // Removes the callback set by `Draw::setWindowCallback()`.
    // Not removing the callback can lead to undefined behavior.
    void removePaintCallback(uint32_t id) noexcept(true);
    // Removes a window.
    void removeWindow(uint32_t id) noexcept(false);
    // Makes the server start sending events to the client.
    void startPollingEventsWindow(uint32_t id) noexcept(false);
    // Makes the server stop sending events to the client.
    void stopPollingEventsWindow(uint32_t id) noexcept(false);
    // Makes the server send a paint event to the specified window.
    void sendPaintEvent(uint32_t id) noexcept(false);
    // Returns a `Display` instance (defined and documented in `Display.h`).
    Display* getDisplay(uint32_t id, RudeDrawerVec2D dims) noexcept(false);
    // Returns a `RudeDrawerEvent` struct (defined and documented in `RudeDrawer.h`).
    RudeDrawerEvent pollEvent(uint32_t id) noexcept(false);

    ~Draw() noexcept(true);
};

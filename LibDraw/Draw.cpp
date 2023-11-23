#include "LibDraw/Draw.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

#include "RudeDrawer.h"

void Draw::send(void* data, size_t n) noexcept(false)
{
    if (::send(socket, data, n, 0) < 0) {
        std::ostringstream error;
        error << "ERROR: could not send data to server: "
              << strerror(errno);
        throw std::runtime_error(error.str());
    }
}

void Draw::recv(void* data, size_t n) noexcept(false)
{
    auto numOfBytesRecvd = ::recv(socket, data, n, 0);
    if (numOfBytesRecvd < 0) {
        std::ostringstream error;
        error << "ERROR: could not receive data from server: "
              << strerror(errno);
        close(socket);
        throw std::runtime_error(error.str());
    }
}

#define NOTOK(resp)                                     \
    if ((resp).errorKind != RDERROR_OK) {               \
        std::ostringstream error;                       \
        error << "ERROR: " << __FUNCTION__              \
              << ": Not OK: Code " << (resp).errorKind; \
        throw std::runtime_error(error.str());          \
    }                                                   \

void Draw::connect() noexcept(false)
{
    std::cout << "[INFO] Connecting to AppDrawer server...\n";

    socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket < 0) {
        std::ostringstream error;
        error << "ERROR: could open socket: "
              << strerror(errno);
        throw std::runtime_error(error.str());
    }

    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (::connect(socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) < 0) {
        std::ostringstream error;
        error << "ERROR: could not connect to socket: "
              << strerror(errno);
        close(socket);
        throw std::runtime_error(error.str());
    }
}

void Draw::ping() noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_PING;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    NOTOK(response);
}

uint32_t Draw::addWindow(std::string title, RudeDrawerVec2D dims, bool alwaysUpdating) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_ADD_WIN;
    command.windowDims = dims;
    std::memset(command.windowTitle, 0, WINDOW_TITLE_MAX);
    std::memcpy(command.windowTitle, title.c_str(), title.size());
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    NOTOK(response);

    if (response.kind != RDRESP_WINID) {
        throw std::runtime_error("ERROR: response is not of kind `RDRESP_WINID`");
    }

    auto id = response.windowId;

    if (alwaysUpdating) {
        std::thread thread([&callbacks = callbacks, id] {
            auto shouldQuit = false;
            while (!shouldQuit) {
                auto it = callbacks.find(id);
                if (it != callbacks.end()) {
                    (it->second.callback)(it->second.parameters);
                    shouldQuit = it->second.shouldQuit;
                }
            }
            auto it = callbacks.find(id);
            if (it != callbacks.end()) {
                it->second.running = false;
            }
        });
        thread.detach();
    }

    return id;
}

void Draw::setPaintCallback(uint32_t id, DrawCallbackFunction callback, void* params) noexcept(true)
{
    callbacks[id] = (DrawCallback) {
        .callback = callback,
        .parameters = params,
    };
}

#define CALLBACK_WAIT_TIME 100

void Draw::removePaintCallback(uint32_t id) noexcept(true)
{
    auto it = callbacks.find(id);
    if (it != callbacks.end()) {
        it->second.shouldQuit = true;
    }

    auto shouldQuit = false;
    while (!shouldQuit) {
        auto it = callbacks.find(id);
        if (it != callbacks.end()) {
            shouldQuit = !it->second.running;
        }
    }
}

void Draw::removeWindow(uint32_t id) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_REMOVE_WIN;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    NOTOK(response);

    removePaintCallback(id);
}

void Draw::startPollingEventsWindow(uint32_t id) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_START_POLLING_EVENTS_WIN;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    NOTOK(response);
}

void Draw::stopPollingEventsWindow(uint32_t id) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_STOP_POLLING_EVENTS_WIN;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    NOTOK(response);
}

void Draw::sendPaintEvent(uint32_t id) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_SEND_PAINT_EVENT;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));
}

Display* Draw::getDisplay(uint32_t id, RudeDrawerVec2D dims) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_GET_DISPLAY_SHM_WIN;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    NOTOK(response);

    if (response.kind != RDRESP_SHM_NAME) {
        throw std::runtime_error("ERROR: response is not of kind `RDRESP_SHM_NAME`");
    }

    std::string shmName((char*)response.windowShmName);

    auto display = new Display(shmName, dims.x, dims.y, id);
    return display;
}

RudeDrawerEvent Draw::pollEvent(uint32_t id) noexcept(false)
{
    RudeDrawerEvent event;
    recv(&event, sizeof(RudeDrawerEvent));
    if (event.kind == RDEVENT_PAINT) {
        auto it = callbacks.find(id);
        if (it != callbacks.end()) {
            (it->second.callback)(it->second.parameters);
            RudeDrawerEvent retval;
            retval.kind = RDEVENT_NONE;
            return retval;
        }
    }
    return event;
}

Draw::~Draw() noexcept(true)
{
    std::cout << "[INFO] Closing connection\n";
    close(socket);
}

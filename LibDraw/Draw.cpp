#include "LibDraw/Draw.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

#include "RudeDrawer.h"

void Draw::send(void* data, int n) noexcept(false)
{
    if (::send(m_socket, data, n, 0) < 0) {
        std::ostringstream error;
        error << "ERROR: could not send data to server: "
              << strerror(errno);
        throw std::runtime_error(error.str());
    }
}

void Draw::recv(void* data, int n) noexcept(false)
{
    auto numOfBytesRecvd = ::recv(m_socket, data, n, 0);
    if (numOfBytesRecvd < 0) {
        std::ostringstream error;
        error << "ERROR: could not receive data from server: "
              << strerror(errno);
        close(m_socket);
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

    m_socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_socket < 0) {
        std::ostringstream error;
        error << "ERROR: could open socket: "
              << strerror(errno);
        throw std::runtime_error(error.str());
    }

    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (::connect(m_socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) < 0) {
        std::ostringstream error;
        error << "ERROR: could not connect to socket: "
              << strerror(errno);
        close(m_socket);
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
        std::thread thread([&callbacks = m_callbacks, id] {
            DrawCallback* callback = NULL;
            {
                auto it = callbacks.find(id);
                if (it != callbacks.end()) {
                    callback = it->second;
                }
            }

            auto shouldQuit = false;
            while (!shouldQuit) {
                if (callback == NULL) {
                    auto it = callbacks.find(id);
                    if (it != callbacks.end()) {
                        callback = it->second;
                    }
                } else {
                    (callback->callback)(callback->parameters);
                    shouldQuit = callback->shouldQuit;
                }
            }

            callback->running = false;
        });
        thread.detach();
    }

    return id;
}

void Draw::setPaintCallback(uint32_t id, DrawCallbackFunction callback, void* params) noexcept(true)
{
    auto paintCallback = new DrawCallback;
    paintCallback->callback = callback;
    paintCallback->parameters = params;
    m_callbacks[id] = paintCallback;
}

void Draw::removePaintCallback(uint32_t id) noexcept(true)
{
    DrawCallback* callback;
    {
        auto it = m_callbacks.find(id);
        if (it != m_callbacks.end()) {
            callback = it->second;
        } else {
            return;
        }
    }

    callback->shouldQuit = true;

    // Wait for the callback to exit
    while (callback->running) asm("nop");

    delete callback;
    m_callbacks.erase(id);
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

    std::cout << "[INFO] Connecting to event socket of window of ID `"
              << id << "`...\n";

    auto eventSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (eventSocket < 0) {
        std::ostringstream error;
        error << "ERROR: could not open socket: "
              << strerror(errno);
        throw std::runtime_error(error.str());
    }

    std::string socketPath = "/tmp/APDWindowSock" + std::to_string(id);

    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

    if (::connect(eventSocket, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) < 0) {
        std::ostringstream error;
        error << "ERROR: could not connect to socket: "
              << strerror(errno);
        close(eventSocket);
        throw std::runtime_error(error.str());
    }

    m_eventSockets[id] = eventSocket;
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

    auto it = m_eventSockets.find(id);
    if (it != m_eventSockets.end())
        close(m_eventSockets[id]);
}

void Draw::sendPaintEvent(uint32_t id) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_SEND_PAINT_EVENT;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));
}

RudeDrawerVec2D Draw::getMousePosition(uint32_t id) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_GET_MOUSE_POSITION;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    NOTOK(response);

    if (response.kind != RDRESP_MOUSE_POSITION) {
        throw std::runtime_error("ERROR: response is not of kind `RDRESP_MOUSE_POSITION`");
    }

    return response.mousePos;
}

RudeDrawerVec2D Draw::getMouseDelta() noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_GET_MOUSE_DELTA;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    NOTOK(response);

    if (response.kind != RDRESP_MOUSE_DELTA) {
        throw std::runtime_error("ERROR: response is not of kind `RDRESP_MOUSE_DELTA`");
    }

    return response.mouseDelta;
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
    auto it = m_eventSockets.find(id);
    if (it == m_eventSockets.end()) {
        std::ostringstream error;
        error << "ERROR: window of id `" << id << "` is not polling events";
        throw std::runtime_error(error.str());
    }
    auto eventSocket = it->second;

    RudeDrawerEvent event;
    auto numOfBytesRecvd = ::recv(eventSocket, &event, sizeof(RudeDrawerEvent), 0);
    if (numOfBytesRecvd < 0) {
        std::ostringstream error;
        error << "ERROR: could not receive data from server: "
              << strerror(errno);
        close(eventSocket);
        throw std::runtime_error(error.str());
    }

    if (event.kind == RDEVENT_PAINT) {
        auto it = m_callbacks.find(id);
        if (it != m_callbacks.end()) {
            (it->second->callback)(it->second->parameters);
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
    close(m_socket);
}

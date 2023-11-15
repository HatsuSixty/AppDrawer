#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "RudeDrawer.h"
#include "Util.h"

class Display {
private:
    int pixelsShmFd;
    size_t pixelsShmSize;
    uint32_t windowId;
public:
    uint8_t* pixels;

    Display(std::string name, uint32_t width, uint32_t height, uint32_t id) noexcept(false);
    void destroy() noexcept(false);
};

Display::Display(std::string name, uint32_t width, uint32_t height, uint32_t id) noexcept(false)
{
    windowId = id;

    pixelsShmFd = shm_open(name.c_str(), O_RDWR, 0666);
    if (pixelsShmFd == -1) {
        std::ostringstream error;
        error << "ERROR: could not open shared memory for window of ID `"
              << windowId << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    pixelsShmSize = width * height * COMPONENTS;

    pixels = (uint8_t*)mmap(NULL, pixelsShmSize, PROT_READ | PROT_WRITE,
                            MAP_SHARED, pixelsShmFd, 0);
    if (pixels == MAP_FAILED) {
        std::ostringstream error;
        error << "ERROR: could not mmap shared memory for window of ID `"
              << windowId << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }
}

void Display::destroy() noexcept(false)
{
    if (munmap(pixels, pixelsShmSize) == -1) {
        std::ostringstream error;
        error << "ERROR: could not munmap shared memory for window of ID `"
              << windowId << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    if (close(pixelsShmFd) == -1) {
        std::ostringstream error;
        error << "ERROR: could not close shared memory for window of ID `"
              << windowId << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }
}

class Draw {
private:
    int socket;

    void send(void* data, size_t n) noexcept(false);
    void recv(void* data, size_t n) noexcept(false);
    void notOk(RudeDrawerResponse resp) noexcept(false);
public:
    void connect() noexcept(false);
    void ping() noexcept(false);
    uint32_t addWindow(std::string title, RudeDrawerVec2D dims) noexcept(false);
    void removeWindow(uint32_t id) noexcept(false);
    void startPollingEventsWindow(uint32_t id) noexcept(false);
    void stopPollingEventsWindow(uint32_t id) noexcept(false);
    Display* getDisplay(uint32_t id, RudeDrawerVec2D dims) noexcept(false);
    RudeDrawerEvent pollEvent() noexcept(false);

    ~Draw() noexcept(true);
};

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

void Draw::notOk(RudeDrawerResponse resp) noexcept(false)
{
    if (resp.errorKind != RDERROR_OK) {
        std::ostringstream error;
        error << "ERROR: Not OK: Code " << resp.errorKind;
        throw std::runtime_error(error.str());
    }
}

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

    notOk(response);
}

uint32_t Draw::addWindow(std::string title, RudeDrawerVec2D dims) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_ADD_WIN;
    command.windowDims = dims;
    std::memset(command.windowTitle, 0, WINDOW_TITLE_MAX);
    std::memcpy(command.windowTitle, title.c_str(), title.size());
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    notOk(response);

    if (response.kind != RDRESP_WINID) {
        throw std::runtime_error("ERROR: response is not of kind `RDRESP_WINID`");
    }

    return response.windowId;
}

void Draw::removeWindow(uint32_t id) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_REMOVE_WIN;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    notOk(response);
}

void Draw::startPollingEventsWindow(uint32_t id) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_START_POLLING_EVENTS_WIN;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    notOk(response);
}

void Draw::stopPollingEventsWindow(uint32_t id) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_STOP_POLLING_EVENTS_WIN;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    notOk(response);
}

Display* Draw::getDisplay(uint32_t id, RudeDrawerVec2D dims) noexcept(false)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_GET_DISPLAY_SHM_WIN;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    notOk(response);

    if (response.kind != RDRESP_SHM_NAME) {
        throw std::runtime_error("ERROR: response is not of kind `RDRESP_SHM_NAME`");
    }

    std::string shmName((char*)response.windowShmName);

    auto display = new Display(shmName, dims.x, dims.y, id);
    return display;
}

RudeDrawerEvent Draw::pollEvent() noexcept(false)
{
    RudeDrawerEvent event;
    recv(&event, sizeof(RudeDrawerEvent));
    return event;
}

Draw::~Draw() noexcept(true)
{
    std::cout << "[INFO] Closing connection\n";
    close(socket);
}

int main() noexcept(true)
{
    Draw draw;
    TRY(draw.connect());
    TRY(draw.ping());

    RudeDrawerVec2D dims = {
        .x = 400,
        .y = 400,
    };

    uint32_t id;
    TRY(id = draw.addWindow("Test Client", dims));
    std::cout << "Window ID: " << id << "\n";

    Display* display = nullptr;
    TRY(display = draw.getDisplay(id, dims));
    for (size_t i = 0; i < dims.x*dims.y; ++i) {
        ((uint32_t*)display->pixels)[i] = 0xFF0000FF;
    }
    TRY(display->destroy());

    TRY(draw.startPollingEventsWindow(id));
    bool quit = false;
    while (!quit) {
        auto event = draw.pollEvent();
        switch (event.kind) {
        case RDEVENT_NONE:
            std::cout << "WTF None event!!??\n";
            break;
        case RDEVENT_CLOSE_WIN:
            std::cout << "Received close window event\n";
            quit = true;
            break;
        }
    }
    TRY(draw.stopPollingEventsWindow(id));

    TRY(draw.removeWindow(id));

    return 0;
}

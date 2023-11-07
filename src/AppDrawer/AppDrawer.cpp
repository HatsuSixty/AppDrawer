#include "AppDrawer.h"

#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <thread>

#include "RudeDrawer.h"
#include "Window.h"

bool fileExists(std::string const& name) noexcept
{
    struct stat buffer;
    return ::stat(name.c_str(), &buffer) == 0;
}

bool receiveOrFail(int sockfd, void* data, size_t n) noexcept
{
    int numBytesReceived = recv(sockfd, data, n, 0);
    if (numBytesReceived < 0) {
        std::cerr << "ERROR: could not receive bytes from socket: "
                  << strerror(errno) << "\n";
        return false;
    }
    else if (numBytesReceived == 0) {
        std::cout << "[INFO] Connection closed by client\n";
        return false;
    }
    return true;
}

bool sendOrFail(int fd, void* data, size_t n) noexcept
{
    if (send(fd, data, n, 0) < 0) {
        std::cerr << "ERROR: could not send data to the client: "
                  << strerror(errno) << "\n";
        return false;
    }
    return true;
}

bool sendErrOrFail(int fd, RudeDrawerErrorKind err) noexcept
{
    RudeDrawerResponse response;
    response.kind = RDRESP_EMPTY;
    response.errorKind = err;
    return sendOrFail(fd, &response, sizeof(RudeDrawerResponse));
}

void AppDrawer::listener() noexcept
{
    if (listen(fd, 20) < 0) {
        std::cerr << "ERROR: could not listen to socket: " << strerror(errno);
        goto exit;
    }
    std::cout << "[INFO] Listening to socket `" << SOCKET_PATH << "`...\n";

    struct sockaddr_un clientAddr;
    while (true) {
        socklen_t clientLen = sizeof(struct sockaddr_un);
        std::cout << "[INFO] Waiting for connection...\n";
        auto clientFd = accept(fd, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientFd < 0) {
            std::cerr << "ERROR: could not accept connection: "
                      << strerror(errno) << "\n";
            continue;
        }

        std::thread thread(&AppDrawer::handleClient, this, clientFd);
        thread.detach();
    }
exit:
    std::cout << "Exiting `listener()` thread...\n";
}

void AppDrawer::handleClient(int clientFd) noexcept
{
    RudeDrawerCommand command;
    while (true) {
        if (!receiveOrFail(clientFd, &command, sizeof(RudeDrawerCommand))) {
            goto exit;
        }

        std::cout << "[INFO] Received data\n";
        switch (command.kind) {
        case RDCMD_PING:
            std::cout << "  => Pong!\n";
            if (!sendErrOrFail(clientFd, RDERROR_OK)) continue;
            break;
        case RDCMD_ADD_WIN: {
            std::cout << "  => Adding window\n";
            std::cout << "    -> Dimensions: "
                      << command.windowWidth << "x" << command.windowHeight
                      << "\n";
            std::string title((char*)command.windowTitle);
            auto id = addWindow(title, command.windowWidth, command.windowHeight);
            std::cout << "    -> ID: " << id << "\n";

            RudeDrawerResponse response;
            response.kind = RDRESP_WINID;
            response.errorKind = RDERROR_OK;
            response.windowId = id;
            if (!sendOrFail(clientFd, &response, sizeof(RudeDrawerResponse)))
                continue;
        } break;
        case RDCMD_REMOVE_WIN:
            std::cout << "  => Removing window\n";
            std::cout << "    -> ID: " << command.windowId << "\n";
            try {
                removeWindow(command.windowId);
            } catch (std::runtime_error const& e) {
                std::cerr << e.what() << "\n";
                if (!sendErrOrFail(clientFd, RDERROR_INVALID_WINID)) continue;
                continue;
            }
            if (!sendErrOrFail(clientFd, RDERROR_OK)) continue;
            break;
        case RDCMD_START_POLLING_EVENTS_WIN: {
            std::cout << "  => Starting polling events for window\n";
            std::cout << "    -> ID: " << command.windowId << "\n";
            try {
                setWindowPolling(command.windowId, true);

                auto i = findWindow(command.windowId);
                std::thread thread(&AppDrawer::pollEvents, this, i, clientFd);
                thread.detach();
            } catch (std::runtime_error const& e) {
                std::cerr << e.what() << "\n";
                if (!sendErrOrFail(clientFd, RDERROR_INVALID_WINID)) continue;
                continue;
            }

            if (!sendErrOrFail(clientFd, RDERROR_OK)) continue;
        } break;
        case RDCMD_STOP_POLLING_EVENTS_WIN:
            std::cout << "  => Stopping polling events for window\n";
            std::cout << "    -> ID: " << command.windowId << "\n";
            try {
                setWindowPolling(command.windowId, false);
            } catch (std::runtime_error const& e) {
                std::cerr << e.what() << "\n";
                if (!sendErrOrFail(clientFd, RDERROR_INVALID_WINID)) continue;
                continue;
            }
            if (!sendErrOrFail(clientFd, RDERROR_OK)) continue;
            break;
        default:
            std::cerr << "  => ERROR: unknown command `" << command.kind << "`\n";
            if (!sendErrOrFail(clientFd, RDERROR_INVALID_COMMAND)) continue;
        }
    }

exit:
    std::cout << "Exiting `handleClient()` thread...\n";
}

void AppDrawer::pollEvents(size_t window_index, int clientFd) noexcept
{
    auto i = window_index;
    while (windows[i].events.isPolling) {
        if (!windows[i].events.events.empty()) {
            auto event = windows[i].events.events.back();
            windows[i].events.events.pop_back();

            if (!sendOrFail(clientFd, &event, sizeof(RudeDrawerEvent)))
                continue;

            std::cout << "!!! Received event: "
                      << event.kind << "\n";
        }
    }
    std::cout << "Exiting `pollEvents()` thread...\n";
}

uint32_t AppDrawer::addWindow(std::string title, uint32_t width, uint32_t height) noexcept
{
    auto id = windowId++;
    Window* window = new Window(title, width, height, id);
    windows.push_back(*window);
    return id;
}

size_t AppDrawer::findWindow(uint32_t id)
{
    for (size_t i = 0; i < windows.size(); ++i) {
        if (windows[i].id == id) {
            return i;
        }
    }
    std::ostringstream error;
    error << "ERROR: could not find window of ID `"
          << id << "`";
    throw std::runtime_error(error.str());
}

void AppDrawer::removeWindow(uint32_t id)
{
    auto i = findWindow(id);

    windows[i].events.isPolling = false;
    // Yes I know this is the best solution a human being could ever
    // come up with
    for (size_t i = 0; i < 500; i++) asm("nop");

    free(windows[i].pixels);
    windows.erase(windows.begin() + i);
}

void AppDrawer::setWindowPolling(uint32_t id, bool polling)
{
    auto i = findWindow(id);
    windows[i].events.isPolling = polling;
}

void AppDrawer::startServer()
{
    if (fileExists(SOCKET_PATH))
        if (std::remove(SOCKET_PATH) != 0) {
            std::ostringstream error;
            error << "ERROR: could not remove `"
                  << SOCKET_PATH
                  << "`: " << strerror(errno);
            throw std::runtime_error(error.str());
        }

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error("ERROR: could not open socket: %s");

    struct sockaddr_un serverAddr;
    std::memset(&serverAddr, 0, sizeof(struct sockaddr_un));
    serverAddr.sun_family = AF_UNIX;
    std::strncpy(serverAddr.sun_path, SOCKET_PATH,
        sizeof(serverAddr.sun_path) - 1);

    auto opt = SO_REUSEADDR;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(fd, (struct sockaddr*)&serverAddr,
            sizeof(struct sockaddr_un))
        < 0) {
        std::ostringstream error;
        error << "ERROR: could not bind to socket: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    std::thread thread(&AppDrawer::listener, this);
    thread.detach();
}

AppDrawer::~AppDrawer() noexcept
{
    for (auto& w : windows) {
        free(w.pixels);
    }
    close(fd);
}

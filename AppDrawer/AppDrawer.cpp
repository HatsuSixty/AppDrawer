#include "AppDrawer.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <thread>

#include "RudeDrawer.h"
#include "Window.h"

bool fileExists(std::string const& name) noexcept(true)
{
    struct stat buffer;
    return ::stat(name.c_str(), &buffer) == 0;
}

Client::Client(int sockfd) noexcept(true)
{
    this->m_sockfd = sockfd;
}

ClientResult Client::receiveOrFail(void* data, int n) noexcept(true)
{
    int numBytesReceived = recv(m_sockfd, data, n, 0);
    if (numBytesReceived < 0) {
        std::cerr << "ERROR: could not receive bytes from socket: "
                  << strerror(errno) << "\n";
        return CLIENT_ERR;
    } else if (numBytesReceived == 0) {
        std::cout << "[INFO] Connection closed by client\n";
        return CLIENT_CLOSED;
    }
    return CLIENT_OK;
}

ClientResult Client::sendOrFail(void* data, int n) noexcept(true)
{
    if (send(m_sockfd, data, n, 0) < 0) {
        std::cerr << "ERROR: could not send data to the client: "
                  << strerror(errno) << "\n";
        return CLIENT_ERR;
    }
    return CLIENT_OK;
}

ClientResult Client::sendErrOrFail(RudeDrawerErrorKind err) noexcept(true)
{
    RudeDrawerResponse response;
    response.kind = RDRESP_EMPTY;
    response.errorKind = err;
    return sendOrFail(&response, sizeof(RudeDrawerResponse));
}

void AppDrawer::listener() noexcept(true)
{
    if (listen(m_fd, 20) < 0) {
        std::cerr << "ERROR: could not listen to socket: " << strerror(errno);
        goto exit;
    }
    std::cout << "[INFO] Listening to socket `" << SOCKET_PATH << "`...\n";

    struct sockaddr_un clientAddr;
    while (true) {
        socklen_t clientLen = sizeof(struct sockaddr_un);
        std::cout << "[INFO] Waiting for connection...\n";
        auto clientFd = accept(m_fd, (struct sockaddr*)&clientAddr, &clientLen);
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

void AppDrawer::handleClient(int clientFd) noexcept(true)
{
    Client client(clientFd);

    RudeDrawerCommand command;
    while (true) {
        if (client.receiveOrFail(&command, sizeof(RudeDrawerCommand)) != CLIENT_OK) {
            goto exit;
        }

        std::cout << "[INFO] Received data\n";
        switch (command.kind) {
        case RDCMD_PING:
            std::cout << "  => Pong!\n";
            if (client.sendErrOrFail(RDERROR_OK) != CLIENT_OK)
                continue;
            break;
        case RDCMD_ADD_WIN: {
            std::cout << "  => Adding window\n";
            std::cout << "    -> Dimensions: "
                      << command.windowDims.x << "x" << command.windowDims.y
                      << "\n";
            std::string title((char*)command.windowTitle);

            auto res = addWindow(title, command.windowDims);
            if (!res.isOk()) {
                client.sendErrOrFail(RDERROR_ADD_WIN_FAILED);
                continue;
            }
            auto id = res.getValue();

            std::cout << "    -> ID: " << id << "\n";

            RudeDrawerResponse response;
            response.kind = RDRESP_WINID;
            response.errorKind = RDERROR_OK;
            response.windowId = id;
            if (client.sendOrFail(&response, sizeof(RudeDrawerResponse)) != CLIENT_OK)
                continue;
        } break;
        case RDCMD_REMOVE_WIN: {
            std::cout << "  => Removing window\n";
            std::cout << "    -> ID: " << command.windowId << "\n";

            auto res = removeWindow(command.windowId);
            if (!res.isOk()) {
                client.sendErrOrFail(RDERROR_INVALID_WINID);
                continue;
            }

            if (client.sendErrOrFail(RDERROR_OK) != CLIENT_OK)
                continue;
        } break;
        case RDCMD_START_POLLING_EVENTS_WIN: {
            std::cout << "  => Starting polling events for window\n";
            std::cout << "    -> ID: " << command.windowId << "\n";

            auto res1 = setWindowPolling(command.windowId, true);
            if (!res1.isOk()) {
                client.sendErrOrFail(RDERROR_INVALID_WINID);
                continue;
            }

            auto res2 = findWindow(command.windowId);
            if (!res2.isOk()) {
                client.sendErrOrFail(RDERROR_INVALID_WINID);
                continue;
            }
            auto i = res2.getValue();

            std::thread thread(&AppDrawer::pollEvents, this, m_windows[i]);
            thread.detach();

            auto timeout = 0;
            auto timeoutLimit = 9999999;
            while (!m_windowsWithEventSockets.contains(command.windowId)) {
                timeout += 1;
                if (timeout >= timeoutLimit) {
                    std::cerr << "ERROR: timed out when waiting for event socket to"
                                << " be available (window id `" << command.windowId << "`)\n";
                    client.sendErrOrFail(RDERROR_CANT_POLL_EVENTS);
                    break;
                }
            }

            if (timeout >= timeoutLimit)
                continue;

            if (client.sendErrOrFail(RDERROR_OK) != CLIENT_OK)
                continue;
        } break;
        case RDCMD_STOP_POLLING_EVENTS_WIN: {
            std::cout << "  => Stopping polling events for window\n";
            std::cout << "    -> ID: " << command.windowId << "\n";

            auto res = setWindowPolling(command.windowId, false);
            if (!res.isOk()) {
                client.sendErrOrFail(RDERROR_INVALID_WINID);
                continue;
            }

            if (client.sendErrOrFail(RDERROR_OK) != CLIENT_OK)
                continue;

        } break;
        case RDCMD_GET_DISPLAY_SHM_WIN: {
            std::cout << "  => Getting window display's shared memory\n";
            std::cout << "    -> ID: " << command.windowId << "\n";

            auto res = findWindow(command.windowId);
            if (!res.isOk()) {
                client.sendErrOrFail(RDERROR_INVALID_WINID);
                continue;
            }
            auto i = res.getValue();

            auto shmName = m_windows[i]->m_pixelsShmName;

            RudeDrawerResponse response;
            response.kind = RDRESP_SHM_NAME;
            response.errorKind = RDERROR_OK;

            std::memset(response.windowShmName, 0, WINDOW_SHM_NAME_MAX);
            std::memcpy(response.windowShmName, shmName.c_str(), shmName.size());

            if (client.sendOrFail(&response, sizeof(RudeDrawerResponse)) != CLIENT_OK)
                continue;
        } break;
        case RDCMD_SEND_PAINT_EVENT: {
            std::cout << "  => Sending paint event to window\n";
            std::cout << "    -> ID: " << command.windowId << "\n";

            auto res = findWindow(command.windowId);
            if (!res.isOk()) {
                client.sendErrOrFail(RDERROR_INVALID_WINID);
                continue;
            }
            auto i = res.getValue();

            RudeDrawerEvent event;
            event.kind = RDEVENT_PAINT;
            m_windows[i]->sendEvent(event);
        } break;
        case RDCMD_GET_MOUSE_POSITION: {
            std::cout << "  => Getting mouse position within window\n";
            std::cout << "    -> ID: " << command.windowId << "\n";

            auto res = findWindow(command.windowId);
            if (!res.isOk()) {
                if (client.sendErrOrFail(RDERROR_INVALID_WINID) != CLIENT_OK)
                    continue;
                continue;
            }
            auto i = res.getValue();

            RudeDrawerResponse response;
            response.kind = RDRESP_MOUSE_POSITION;
            response.errorKind = RDERROR_OK;

            auto mousePosX = m_mousePos.x - m_windows[i]->m_area.x;
            auto mousePosY = m_mousePos.y - m_windows[i]->m_area.y;
            auto outOfX = mousePosX < 0 || mousePosX > m_windows[i]->m_area.width;
            auto outOfY = mousePosY < 0 || mousePosY > m_windows[i]->m_area.height;
            if (outOfX || outOfY) {
                mousePosX = 0;
                mousePosY = 0;
            }

            response.mousePos = RudeDrawerVec2D {
                .x = (int)mousePosX,
                .y = (int)mousePosY,
            };
            if (client.sendOrFail(&response, sizeof(RudeDrawerResponse)) != CLIENT_OK)
                continue;
        } break;
        case RDCMD_GET_MOUSE_DELTA: {
            std::cout << "  => Getting mouse delta\n";

            RudeDrawerResponse response;
            response.kind = RDRESP_MOUSE_DELTA;
            response.errorKind = RDERROR_OK;

            response.mouseDelta = RudeDrawerVec2D {
                .x = (int)(m_mousePos.x - m_previousMousePos.x),
                .y = (int)(m_mousePos.y - m_previousMousePos.y),
            };
            if (client.sendOrFail(&response, sizeof(RudeDrawerResponse)) != CLIENT_OK)
                continue;
        } break;
        default:
            std::cerr << "  => ERROR: unknown command `" << command.kind << "`\n";
            if (client.sendErrOrFail(RDERROR_INVALID_COMMAND) != CLIENT_OK)
                continue;
        }
    }

exit:
    std::cout << "Exiting `handleClient()` thread...\n";
}

void AppDrawer::pollEvents(Window* window) noexcept(true)
{
    int serverEventSocket = -1;
    int clientEventSocket = -1;
    {
        std::string socketPath = "/tmp/APDWindowSock" + std::to_string(window->m_id);

        if (fileExists(socketPath)) {
            if (std::remove(socketPath.c_str()) != 0) {
                std::cerr << "ERROR: could not remove `"
                          << socketPath
                          << "`: " << strerror(errno) << "\n";
                goto exit;
            }
        }

        serverEventSocket = socket(AF_UNIX, SOCK_STREAM, 0);

        if (serverEventSocket < 0) {
            std::cerr << "ERROR: could not open socket: `"
                      << socketPath << "`\n";
            goto exit;
        }

        struct sockaddr_un serverAddr;
        std::memset(&serverAddr, 0, sizeof(struct sockaddr_un));
        serverAddr.sun_family = AF_UNIX;
        std::strncpy(serverAddr.sun_path, socketPath.c_str(),
            sizeof(serverAddr.sun_path) - 1);

        auto opt = SO_REUSEADDR;
        setsockopt(serverEventSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (bind(serverEventSocket, (struct sockaddr*)&serverAddr,
                sizeof(struct sockaddr_un))
            < 0) {
            std::cerr << "ERROR: could not bind to socket: " << strerror(errno) << "\n";
            goto exit;
        }

        if (listen(serverEventSocket, 1) < 0) {
            std::cerr << "ERROR: could not listen to socket: " << strerror(errno) << "\n";
            goto exit;
        }

        m_windowsWithEventSockets.insert(window->m_id);

        struct sockaddr_un clientAddr;
        socklen_t clientLen = sizeof(struct sockaddr_un);
        clientEventSocket = accept(serverEventSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientEventSocket < 0) {
            std::cerr << "ERROR: could not accept connection: "
                      << strerror(errno) << "\n";
            goto exit;
        }

        Client client(clientEventSocket);

        while (window->m_events.isPolling) {
            if (!window->m_events.events.empty()) {
                auto event = window->m_events.events.back();
                window->m_events.events.pop_back();

                if (client.sendOrFail(&event, sizeof(RudeDrawerEvent)) != CLIENT_OK)
                    continue;
            }
        }
    }
exit:
    close(serverEventSocket);
    close(clientEventSocket);
    if (auto iter = m_windowsWithEventSockets.find(window->m_id);
        iter != m_windowsWithEventSockets.end())
        m_windowsWithEventSockets.erase(iter);
    window->m_events.running = false;
    std::cout << "Exiting `pollEvents()` thread...\n";
}

Result<void*, uint32_t> AppDrawer::addWindow(std::string title, RudeDrawerVec2D dims) noexcept(false)
{
    std::lock_guard<std::mutex> guard(m_windowsMutex);

    auto id = m_windowId++;

    auto res = Window::create(title, dims.x, dims.y, id);
    if (!res.isOk()) {
        return Result<void*, uint32_t>::fromError(nullptr);
    }

    Window* window = res.getValue();
    m_windows.push_back(window);

    return Result<void*, uint32_t>::fromValue(id);
}

void AppDrawer::setMousePosition(Vector2 mousePos) noexcept(true)
{
    m_previousMousePos = m_mousePos;
    m_mousePos = mousePos;
}

Result<void*, int> AppDrawer::findWindow(uint32_t id) noexcept(false)
{
    for (unsigned long i = 0; i < m_windows.size(); ++i) {
        if (m_windows[i]->m_id == id) {
            return Result<void*, int>::fromValue(i);
        }
    }

    std::cerr << "ERROR: could not find window of ID `" << id << "`\n";
    return Result<void*, int>::fromError(nullptr);
}

Result<void*, void*> AppDrawer::removeWindow(uint32_t id) noexcept(false)
{
    std::lock_guard<std::mutex> guard(m_windowsMutex);

    auto res1 = findWindow(id);
    if (!res1.isOk()) {
        return Result<void*, void*>::fromError(nullptr);
    }
    auto i = res1.getValue();

    m_windows[i]->m_events.isPolling = false;
    // Wait for `pollEvents` thread to exit
    while (m_windows[i]->m_events.running)
        asm("nop");

    auto res2 = m_windows[i]->destroy();
    if (!res2.isOk()) {
        return Result<void*, void*>::fromError(nullptr);
    }

    delete m_windows[i];

    m_windows.erase(m_windows.begin() + i);

    return Result<void*, void*>::fromValue(nullptr);
}

Result<void*, void*> AppDrawer::setWindowPolling(uint32_t id, bool polling) noexcept(false)
{
    auto res = findWindow(id);
    if (!res.isOk()) {
        return Result<void*, void*>::fromError(nullptr);
    }
    auto i = res.getValue();
    m_windows[i]->m_events.isPolling = polling;

    return Result<void*, void*>::fromValue(nullptr);
}

Result<void*, void*> AppDrawer::changeActiveWindow(uint32_t id)
{
    std::lock_guard<std::mutex> guard(m_windowsMutex);

    auto res = findWindow(id);
    if (!res.isOk()) {
        return Result<void*, void*>::fromError(nullptr);
    }
    auto windowIndex = res.getValue();

    auto window = m_windows[windowIndex];
    m_windows.erase(m_windows.begin() + windowIndex);
    m_windows.push_back(window);

    return Result<void*, void*>::fromValue(nullptr);
}

AppDrawer::AppDrawer() noexcept(false)
{
    if (fileExists(SOCKET_PATH))
        if (std::remove(SOCKET_PATH) != 0) {
            std::cerr << "ERROR: could not remove `"
                      << SOCKET_PATH
                      << "`: " << strerror(errno)
                      << "\n";
            exit(1);
        }

    m_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_fd < 0) {
        std::cerr << "ERROR: could not open socket: `"
                  << SOCKET_PATH << "`\n";
        exit(1);
    }

    struct sockaddr_un serverAddr;
    std::memset(&serverAddr, 0, sizeof(struct sockaddr_un));
    serverAddr.sun_family = AF_UNIX;
    std::strncpy(serverAddr.sun_path, SOCKET_PATH,
        sizeof(serverAddr.sun_path) - 1);

    auto opt = SO_REUSEADDR;
    setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(m_fd, (struct sockaddr*)&serverAddr,
            sizeof(struct sockaddr_un))
        < 0) {
        std::cerr << "ERROR: could not bind to socket: " << strerror(errno) << "\n";
        exit(1);
    }

    std::thread thread(&AppDrawer::listener, this);
    thread.detach();
}

AppDrawer::~AppDrawer() noexcept(true)
{
    for (auto& w : m_windows) {
        w->destroy();
    }
    close(m_fd);
}

void AppDrawer::lockWindows() noexcept(true)
{
    m_windowsMutex.lock();
}

void AppDrawer::unlockWindows() noexcept(true)
{
    m_windowsMutex.unlock();
}

Window* AppDrawer::topWindow() noexcept(true)
{
    return m_windows.back();
}

int AppDrawer::windowCount() noexcept(true)
{
    return m_windows.size();
}

Window* AppDrawer::windowIndex(int index) noexcept(false)
{
    return m_windows[index];
}

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

bool file_exists(std::string const& name) noexcept
{
    struct stat buffer;
    return ::stat(name.c_str(), &buffer) == 0;
}

bool receive_or_fail(int sockfd, void* data, size_t n) noexcept
{
    int num_bytes_received = recv(sockfd, data, n, 0);
    if (num_bytes_received < 0) {
        std::cerr << "ERROR: could not receive bytes from socket: "
                  << strerror(errno) << "\n";
        return false;
    }
    else if (num_bytes_received == 0) {
        std::cout << "[INFO] Connection closed by client\n";
        return false;
    }
    return true;
}

bool send_or_fail(int fd, void* data, size_t n) noexcept
{
    if (send(fd, data, n, 0) < 0) {
        std::cerr << "ERROR: could not send data to the client: "
                  << strerror(errno);
        return false;
    }
    return true;
}

bool send_err_or_fail(int fd, RudeDrawerErrorKind err) noexcept
{
    RudeDrawerResponse response;
    response.kind = RDRESP_EMPTY;
    response.errorKind = err;
    return send_or_fail(fd, &response, sizeof(RudeDrawerResponse));
}

void AppDrawer::listener() noexcept
{
    if (listen(fd, 20) < 0) {
        std::cerr << "ERROR: could not listen to socket: " << strerror(errno);
        goto exit;
    }
    std::cout << "[INFO] Listening to socket `" << SOCKET_PATH << "`...\n";

    struct sockaddr_un client_addr;
    while (true) {
        socklen_t client_len = sizeof(struct sockaddr_un);
        std::cout << "[INFO] Waiting for connection...\n";
        auto client_fd = accept(fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "ERROR: could not accept connection: "
                      << strerror(errno) << "\n";
            continue;
        }

        std::thread thread(&AppDrawer::handleClient, this, client_fd);
        thread.detach();
    }
exit:
    std::cout << "Exiting `listener()` thread...\n";
}

void AppDrawer::handleClient(int client_fd) noexcept
{
    RudeDrawerCommand command;
    while (true) {
        if (!receive_or_fail(client_fd, &command, sizeof(RudeDrawerCommand))) {
            goto exit;
        }

        std::cout << "[INFO] Received data\n";
        switch (command.kind) {
        case RDCMD_PING:
            std::cout << "  => Pong!\n";
            if (!send_err_or_fail(client_fd, RDERROR_OK)) continue;
            break;
        case RDCMD_ADD_WINDOW: {
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
            if (!send_or_fail(client_fd, &response, sizeof(RudeDrawerResponse)))
                continue;
        } break;
        case RDCMD_REMOVE_WINDOW:
            std::cout << "  => Removing window\n";
            std::cout << "    -> ID: " << command.windowId << "\n";
            try {
                removeWindow(command.windowId);
            } catch (const std::runtime_error& e) {
                std::cerr << e.what() << "\n";
                if (!send_err_or_fail(client_fd, RDERROR_INVALID_WINID)) continue;
                continue;
            }
            if (!send_err_or_fail(client_fd, RDERROR_OK)) continue;
            break;
        default:
            std::cerr << "  => ERROR: unknown command `" << command.kind << "`\n";
            if (!send_err_or_fail(client_fd, RDERROR_INVALID_COMMAND)) continue;
        }
    }

exit:
    std::cout << "Exiting `handle_client() thread...`\n";
}

uint32_t AppDrawer::addWindow(std::string title, uint32_t width, uint32_t height) noexcept
{
    auto id = windowId++;
    Window* window = new Window(title, width, height, id);
    windows.push_back(*window);
    return id;
}

void AppDrawer::removeWindow(uint32_t id)
{
    for (size_t i = 0; i < windows.size(); ++i) {
        if (windows[i].id == id) {
            free(windows[i].pixels);
            windows.erase(windows.begin() + i);
            return;
        }
    }
    std::ostringstream error;
    error << "ERROR: could not find window of ID `"
          << id;
    throw std::runtime_error(error.str());
}

void AppDrawer::startServer()
{
    if (file_exists(SOCKET_PATH))
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

    struct sockaddr_un server_addr;
    std::memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    std::strncpy(server_addr.sun_path, SOCKET_PATH,
        sizeof(server_addr.sun_path) - 1);

    auto opt = SO_REUSEADDR;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(fd, (struct sockaddr*)&server_addr,
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

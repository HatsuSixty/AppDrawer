#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include <unistd.h>

#include "RudeDrawer.h"
#include "Util.h"

class Draw {
private:
    int socket;

    void send(void* data, size_t n);
    void recv(void* data, size_t n);
    void notOk(RudeDrawerResponse resp);
public:
    void connect();
    void ping();
    uint32_t addWindow(std::string title, uint32_t width, uint32_t height);
    void removeWindow(uint32_t id);

    ~Draw() noexcept;
};

void Draw::send(void* data, size_t n)
{
    if (::send(socket, data, n, 0) < 0) {
        std::ostringstream error;
        error << "ERROR: could not send data to server: "
              << strerror(errno);
        throw std::runtime_error(error.str());
    }
}

void Draw::recv(void* data, size_t n)
{
    auto num_of_bytes_recvd = ::recv(socket, data, n, 0);
    if (num_of_bytes_recvd < 0) {
        std::ostringstream error;
        error << "ERROR: could not receive data from server: "
              << strerror(errno);
        close(socket);
        throw std::runtime_error(error.str());
    }
}

void Draw::notOk(RudeDrawerResponse resp)
{
    if (resp.errorKind != RDERROR_OK) {
        std::ostringstream error;
        error << "ERROR: Not OK: Code " << resp.errorKind;
        throw std::runtime_error(error.str());
    }
}

void Draw::connect()
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

    if (::connect(socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) <
        0) {
        std::ostringstream error;
        error << "ERROR: could not connect to socket: "
              << strerror(errno);
        close(socket);
        throw std::runtime_error(error.str());
    }
}

void Draw::ping()
{
    RudeDrawerCommand command;
    command.kind = RDCMD_PING;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    notOk(response);
}

uint32_t Draw::addWindow(std::string title, uint32_t width, uint32_t height)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_ADD_WINDOW;
    command.windowWidth = width;
    command.windowHeight = height;
    std::memset(command.windowTitle, 0, 256*sizeof(command.windowTitle[0]));
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

void Draw::removeWindow(uint32_t id)
{
    RudeDrawerCommand command;
    command.kind = RDCMD_REMOVE_WINDOW;
    command.windowId = id;
    send(&command, sizeof(RudeDrawerCommand));

    RudeDrawerResponse response;
    recv(&response, sizeof(RudeDrawerResponse));

    notOk(response);
}

Draw::~Draw() noexcept
{
    std::cout << "[INFO] Closing connection\n";
    close(socket);
}

int main() noexcept
{
    Draw draw;
    TRY(draw.connect());
    TRY(draw.ping());

    uint32_t id;
    TRY(id = draw.addWindow("Test Client", 400, 400));

    std::cout << "Window ID: " << id << "\n";

    std::cout << "Press enter to quit the application...\n";
    std::getchar();

    TRY(draw.removeWindow(id));

    return 0;
}

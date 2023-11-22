#include "Window.h"

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Consts.h"
#include "RudeDrawer.h"

Window::Window(std::string title, uint32_t width, uint32_t height, uint32_t id) noexcept(false)
{
    this->title = title;
    area.width = width;
    area.height = height;
    area.x = (float)WIDTH / 2 - (float)width / 2;
    area.y = (float)HEIGHT / 2 - (float)height / 2;
    this->id = id;

    pixelsShmSize = width * height * COMPONENTS;
    pixelsShmName = "/APDWindow" + std::to_string(id);
    pixelsShmFd = shm_open(pixelsShmName.c_str(), O_CREAT | O_RDWR, 0666);
    if (pixelsShmFd == -1) {
        std::ostringstream error;
        error << "ERROR: could not create shared memory for window of ID `"
              << id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    if (ftruncate(pixelsShmFd, pixelsShmSize) == -1) {
        std::ostringstream error;
        error << "ERROR: could not truncate shared memory for window of ID `"
              << id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    pixels = (uint8_t*)mmap(NULL, pixelsShmSize, PROT_READ | PROT_WRITE,
                            MAP_SHARED, pixelsShmFd, 0);
    if (pixels == MAP_FAILED) {
        std::ostringstream error;
        error << "ERROR: could not mmap shared memory for window of ID `"
              << id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    std::memset(pixels, 0xFF, pixelsShmSize);
}

#define DEBUG_PAINT_EVENT false

void Window::sendEvent(RudeDrawerEvent event) noexcept(true)
{
    if (event.kind != RDEVENT_PAINT || DEBUG_PAINT_EVENT)
        std::cout << "[INFO] Sending event of ID `" << event.kind
                  << "` to window of ID `" << id << "`\n";
    if (events.isPolling) {
        events.events.push_back(event);
    } else {
        if (event.kind != RDEVENT_PAINT || DEBUG_PAINT_EVENT)
            std::cout << "[WARN] Window not polling events, not sending...\n";
    }
}

void Window::destroy() noexcept(false)
{
    if (munmap(pixels, pixelsShmSize) == -1) {
        std::ostringstream error;
        error << "ERROR: could not munmap shared memory for window of ID `"
              << id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    if (close(pixelsShmFd) == -1) {
        std::ostringstream error;
        error << "ERROR: could not close shared memory for window of ID `"
              << id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    if (shm_unlink(pixelsShmName.c_str()) == -1) {
        std::ostringstream error;
        error << "ERROR: could not unlink shared memory for window of ID `"
              << id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }
}

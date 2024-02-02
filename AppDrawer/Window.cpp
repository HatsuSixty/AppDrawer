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
    m_title = title;
    m_area.width = width;
    m_area.height = height;
    m_area.x = (float)WIDTH / 2 - (float)width / 2;
    m_area.y = (float)HEIGHT / 2 - (float)height / 2;
    m_id = id;

    m_pixelsShmSize = width * height * COMPONENTS;
    m_pixelsShmName = "/APDWindow" + std::to_string(id);
    m_pixelsShmFd = shm_open(m_pixelsShmName.c_str(), O_CREAT | O_RDWR, 0666);
    if (m_pixelsShmFd == -1) {
        std::ostringstream error;
        error << "ERROR: could not create shared memory for window of ID `"
              << id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    if (ftruncate(m_pixelsShmFd, m_pixelsShmSize) == -1) {
        std::ostringstream error;
        error << "ERROR: could not truncate shared memory for window of ID `"
              << id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    m_pixels = (uint8_t*)mmap(NULL, m_pixelsShmSize, PROT_READ | PROT_WRITE,
                            MAP_SHARED, m_pixelsShmFd, 0);
    if (m_pixels == MAP_FAILED) {
        std::ostringstream error;
        error << "ERROR: could not mmap shared memory for window of ID `"
              << id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    std::memset(m_pixels, 0xFF, m_pixelsShmSize);
}

#define DEBUG_NONLOGGED_EVENTS false

void Window::sendEvent(RudeDrawerEvent event) noexcept(true)
{
    if ((event.kind != RDEVENT_PAINT && event.kind != RDEVENT_MOUSEMOVE)
        || DEBUG_NONLOGGED_EVENTS)
        std::cout << "[INFO] Sending event of ID `" << event.kind
                  << "` to window of ID `" << m_id << "`\n";
    if (m_events.isPolling) {
        m_events.events.push_back(event);
    } else {
        if ((event.kind != RDEVENT_PAINT && event.kind != RDEVENT_MOUSEMOVE)
            || DEBUG_NONLOGGED_EVENTS)
            std::cout << "[WARN] Window not polling events, not sending...\n";
    }
}

void Window::destroy() noexcept(false)
{
    if (munmap(m_pixels, m_pixelsShmSize) == -1) {
        std::ostringstream error;
        error << "ERROR: could not munmap shared memory for window of ID `"
              << m_id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    if (close(m_pixelsShmFd) == -1) {
        std::ostringstream error;
        error << "ERROR: could not close shared memory for window of ID `"
              << m_id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    if (shm_unlink(m_pixelsShmName.c_str()) == -1) {
        std::ostringstream error;
        error << "ERROR: could not unlink shared memory for window of ID `"
              << m_id << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }
}

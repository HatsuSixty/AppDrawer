#include "Window.h"

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <stdexcept>
#include <string>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "RudeDrawer.h"

#include "ErrorHandling.h"

Result<void*, Window*> Window::create(std::string title, uint32_t width, uint32_t height, uint32_t id)
{
    Window* w = new Window();

    w->m_title = title;
    w->m_area.width = width;
    w->m_area.height = height;
    w->m_area.x = (float)GetScreenWidth() / 2 - (float)width / 2;
    w->m_area.y = (float)GetScreenHeight() / 2 - (float)height / 2;
    w->m_id = id;

    w->m_pixelsShmSize = width * height * COMPONENTS;
    w->m_pixelsShmName = "/APDWindow" + std::to_string(id);
    w->m_pixelsShmFd = shm_open(w->m_pixelsShmName.c_str(), O_CREAT | O_RDWR, 0666);
    if (w->m_pixelsShmFd == -1) {
        std::cerr << "ERROR: could not create shared memory for window of ID `"
                  << id << "`: " << strerror(errno) << "\n";
        return Result<void*, Window*>::fromError(nullptr);
    }

    if (ftruncate(w->m_pixelsShmFd, w->m_pixelsShmSize) == -1) {
        std::cerr << "ERROR: could not truncate shared memory for window of ID `"
                  << id << "`: " << strerror(errno) << "\n";
        return Result<void*, Window*>::fromError(nullptr);
    }

    w->m_pixels = (uint8_t*)mmap(nullptr, w->m_pixelsShmSize, PROT_READ | PROT_WRITE,
        MAP_SHARED, w->m_pixelsShmFd, 0);
    if (w->m_pixels == MAP_FAILED) {
        std::cerr << "ERROR: could not mmap shared memory for window of ID `"
                  << id << "`: " << strerror(errno) << "\n";
        return Result<void*, Window*>::fromError(nullptr);
    }

    std::memset(w->m_pixels, 0xFF, w->m_pixelsShmSize);

    return Result<void*, Window*>::fromValue(w);
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

Result<void*, void*> Window::destroy() noexcept(false)
{
    if (munmap(m_pixels, m_pixelsShmSize) == -1) {
        std::cerr << "ERROR: could not munmap shared memory for window of ID `"
                  << m_id << "`: " << strerror(errno) << "\n";
        return Result<void*, void*>::fromError(nullptr);
    }

    if (close(m_pixelsShmFd) == -1) {
        std::cerr << "ERROR: could not close shared memory for window of ID `"
                  << m_id << "`: " << strerror(errno) << "\n";
        return Result<void*, void*>::fromError(nullptr);
    }

    if (shm_unlink(m_pixelsShmName.c_str()) == -1) {
        std::cerr << "ERROR: could not unlink shared memory for window of ID `"
                  << m_id << "`: " << strerror(errno) << "\n";
        return Result<void*, void*>::fromError(nullptr);
    }

    return Result<void*, void*>::fromValue(nullptr);
}

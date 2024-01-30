#include "LibDraw/Display.h"

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

#include "RudeDrawer.h"

Display::Display(std::string name, uint32_t width, uint32_t height, uint32_t id) noexcept(false)
{
    m_windowId = id;

    m_pixelsShmFd = shm_open(name.c_str(), O_RDWR, 0666);
    if (m_pixelsShmFd == -1) {
        std::ostringstream error;
        error << "ERROR: could not open shared memory for window of ID `"
              << m_windowId << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    m_pixelsShmSize = width * height * COMPONENTS;

    m_pixels = (uint8_t*)mmap(NULL, m_pixelsShmSize, PROT_READ | PROT_WRITE,
                            MAP_SHARED, m_pixelsShmFd, 0);
    if (m_pixels == MAP_FAILED) {
        std::ostringstream error;
        error << "ERROR: could not mmap shared memory for window of ID `"
              << m_windowId << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }
}

void Display::destroy() noexcept(false)
{
    if (munmap(m_pixels, m_pixelsShmSize) == -1) {
        std::ostringstream error;
        error << "ERROR: could not munmap shared memory for window of ID `"
              << m_windowId << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }

    if (close(m_pixelsShmFd) == -1) {
        std::ostringstream error;
        error << "ERROR: could not close shared memory for window of ID `"
              << m_windowId << "`: " << strerror(errno);
        throw std::runtime_error(error.str());
    }
}

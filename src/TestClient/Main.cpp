#include <iostream>

#include "LibDraw/Draw.h"
#include "LibDraw/Display.h"
#include "LibDraw/Types.h"

#include "Util.h"

int main() noexcept(true)
{
    Draw draw;
    TRY(draw.connect());
    TRY(draw.ping());

    DrawVec2D dims = {
        .x = 400,
        .y = 400,
    };

    uint32_t id;
    TRY(id = draw.addWindow("Test Client", dims, true));
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
        case RDEVENT_PAINT:
            std::cout << "Paint event!!\n";
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

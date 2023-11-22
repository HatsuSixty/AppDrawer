#include "LibDraw/Display.h"
#include "LibDraw/Draw.h"
#include "LibDraw/Types.h"
#include "RudeDrawer.h"

#include <iostream>

int main()
{
    Draw draw;
    draw.connect();

    DrawVec2D dims = {
        .x = 200,
        .y = 200,
    };

    auto id = draw.addWindow("Bruh", dims, true);
    std::cout << "Window ID: " << id << "\n";

    Display* display = draw.getDisplay(id, dims);

    draw.startPollingEventsWindow(id);
    bool quit = false;
    while (!quit) {
        auto event = draw.pollEvent(id);
        switch (event.kind) {
        case RDEVENT_CLOSE_WIN:
            quit = true;
            break;
        case RDEVENT_PAINT: {
            // The client should update its window's contents at every
            // paint event
        } break;
        default:
            break;
        }
    }
    draw.stopPollingEventsWindow(id);
    draw.removeWindow(id);

    display->destroy();
    return 0;
}

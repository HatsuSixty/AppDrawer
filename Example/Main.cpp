#include "LibDraw/Display.h"
#include "LibDraw/Draw.h"
#include "LibDraw/Types.h"
#include "RudeDrawer.h"

#include <iostream>

struct CallbackParameters {
    DrawVec2D dims;
    uint8_t* pixels;
};

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

    CallbackParameters parameters = {
        .dims = dims,
        .pixels = display->pixels,
    };

    draw.setPaintCallback(id, [](void* p) {
        auto params = (CallbackParameters*)p;
        // This function is called everytime a frame needs to be updated
        // if the last parameter of `Draw::addWindow()` is `true`, this
        // function is called at every frame, else, it is called everytime
        // the client calls `Draw::sendPaintEvent()`
        for (size_t x = 0; x < params->dims.x; ++x) {
            for (size_t y = 0; y < params->dims.y; ++y) {
                // 0xFFFF0000 = blue
                ((uint32_t*)params->pixels)[x*params->dims.x+y] = 0xFFFF0000;
            }
        }
    }, &parameters);

    draw.startPollingEventsWindow(id);
    bool quit = false;
    while (!quit) {
        auto event = draw.pollEvent(id);
        switch (event.kind) {
        case RDEVENT_CLOSE_WIN:
            quit = true;
            break;
        case RDEVENT_KEYPRESS:
            std::cout << "Key pressed! " << event.key << "\n";
            break;
        default:
            break;
        }
    }
    draw.stopPollingEventsWindow(id);

    draw.removePaintCallback(id);

    draw.removeWindow(id);
    display->destroy();

    return 0;
}

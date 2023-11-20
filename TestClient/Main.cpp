#include <iostream>

#include "LibDraw/Draw.h"
#include "LibDraw/Display.h"
#include "LibDraw/Types.h"

// Disable all olive.c warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#define OLIVEC_IMPLEMENTATION
#include "olive.c"

#pragma clang diagnostic pop

#include "Util.h"

int main() noexcept(true)
{
    Draw draw;
    TRY(draw.connect());
    TRY(draw.ping());

    DrawVec2D dims = {
        .x = 500,
        .y = 400,
    };

    uint32_t id;
    TRY(id = draw.addWindow("Test Client", dims, true));
    std::cout << "Window ID: " << id << "\n";

    Display* display = nullptr;
    TRY(display = draw.getDisplay(id, dims));
    Olivec_Canvas canvas = olivec_canvas((uint32_t*)display->pixels, dims.x, dims.y, dims.x);

#define RECT_WIDTH 100
#define RECT_HEIGHT 100
#define RECT_SPD 1
    size_t rectX = dims.x / 2 - RECT_WIDTH / 2;
    size_t rectY = (dims.y / 2 - RECT_HEIGHT / 2) + 69;
    size_t rectDirX = RECT_SPD;
    size_t rectDirY = -RECT_SPD;

    TRY(draw.startPollingEventsWindow(id));
    bool quit = false;
    while (!quit) {
        auto event = draw.pollEvent();
        switch (event.kind) {
        case RDEVENT_NONE:
            std::cout << "WTF None event!!??\n";
            break;
        case RDEVENT_PAINT: {
            olivec_fill(canvas, 0xFF181818);

            if (rectX + RECT_WIDTH >= dims.x || rectX <= 0)
                rectDirX = -rectDirX;
            if (rectY + RECT_HEIGHT >= dims.y || rectY <= 0)
                rectDirY = -rectDirY;

            rectX += rectDirX;
            rectY += rectDirY;

            olivec_rect(canvas, rectX, rectY, RECT_WIDTH, RECT_HEIGHT, 0xFF00FFFF);
        } break;
        case RDEVENT_CLOSE_WIN:
            std::cout << "Received close window event\n";
            quit = true;
            break;
        }
    }
    TRY(draw.stopPollingEventsWindow(id));

    TRY(display->destroy());
    TRY(draw.removeWindow(id));

    return 0;
}

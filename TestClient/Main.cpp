#include <chrono>
#include <iostream>
#include <thread>

#include "LibDraw/Draw.h"
#include "LibDraw/Display.h"
#include "LibDraw/Types.h"
#include "RudeDrawer.h"

// Disable all olive.c warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#define OLIVEC_IMPLEMENTATION
#include "olive.c"

#pragma clang diagnostic pop

#include "Util.h"

typedef struct {
    size_t* rectX;
    size_t* rectDirX;
    size_t* rectY;
    size_t* rectDirY;
    Olivec_Canvas canvas;
    DrawVec2D dims;
} CallbackParameters;

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
    Olivec_Canvas canvas = olivec_canvas((uint32_t*)display->m_pixels, dims.x, dims.y, dims.x);

#define RECT_WIDTH 100
#define RECT_HEIGHT 100
#define RECT_SPD 1
    size_t rectX = dims.x / 2 - RECT_WIDTH / 2;
    size_t rectY = (dims.y / 2 - RECT_HEIGHT / 2) + 69;
    size_t rectDirX = RECT_SPD;
    size_t rectDirY = -RECT_SPD;

    CallbackParameters params = {
        .rectX = &rectX,
        .rectDirX = &rectDirX,
        .rectY = &rectY,
        .rectDirY = &rectDirY,
        .canvas = canvas,
        .dims = dims,
    };

    draw.setPaintCallback(id, [](void* p) {
        auto params = (CallbackParameters*)p;
        olivec_fill(params->canvas, 0xFF181818);

        if (*params->rectX + RECT_WIDTH >= params->dims.x || *params->rectX <= 0)
            *params->rectDirX = -(*params->rectDirX);
        if (*params->rectY + RECT_HEIGHT >= params->dims.y || *params->rectY <= 0)
            *params->rectDirY = -(*params->rectDirY);

        *params->rectX += *params->rectDirX;
        *params->rectY += *params->rectDirY;

        olivec_rect(params->canvas, *params->rectX, *params->rectY,
                    RECT_WIDTH, RECT_HEIGHT, 0xFF00FFFF);
    }, &params);

    TRY(draw.startPollingEventsWindow(id));
    bool quit = false;
    while (!quit) {
        auto event = draw.pollEvent(id);
        switch (event.kind) {
        case RDEVENT_NONE:
            std::cout << "WTF None event!!??\n";
            break;
        case RDEVENT_CLOSE_WIN:
            std::cout << "Received close window event\n";
            quit = true;
            break;
        case RDEVENT_KEYPRESS:
            std::cout << "Key pressed! " << event.key << "\n";
            break;
        case RDEVENT_MOUSEPRESS:
            if (event.mouseButton == RDMOUSE_LEFT) {
                auto pos = draw.getMousePosition(id);
                auto withinRectangleX = pos.x >= rectX && pos.x < (rectX + RECT_WIDTH);
                auto withinRectangleY = pos.y >= rectY && pos.y < (rectY + RECT_HEIGHT);
                if (withinRectangleX && withinRectangleY) {
                    std::cout << "Rectangle clicked!\n";
                }
            } else {
                std::cout << "Mouse button pressed! " << event.mouseButton << "\n";
            }
            break;
        default:
            break;
        }
    }
    TRY(draw.stopPollingEventsWindow(id));

    draw.removePaintCallback(id);

    TRY(display->destroy());
    TRY(draw.removeWindow(id));

    return 0;
}

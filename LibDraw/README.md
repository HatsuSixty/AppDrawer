# LibDraw

This is a library for building AppDrawer client applications.

## LibDraw application structure

Here's an example code on how LibDraw applications should be structured:
```cpp
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
```

By reading this code, you probably can mostly understand what it is doing. Here's a quick explanation on what every function is doing:
- `draw.connect()` - The first thing that should be done in order to have a functioning `Draw` instance is to call `Draw::connect()`. This function will connect with an already running AppDrawer server.
- `draw.addWindow()` - This function adds a window and returns an ID that can be used for future operations.
- `draw.getDisplay()` - This function returns a `Display` instance. To draw into the display, you should directly modify `Display::pixels`, that is a pointer to RGBA data.
- `draw.setPaintCallback()` - This function sets the callback that will be called everytime a window needs to be updated.
- `draw.startPollingEventsWindow()` - This function tells the AppDrawer server to start sending events to a window. **Not receiving these events later on leads to undefined behavior.**
- `draw.pollEvent()` - Returns a `RudeDrawerEvent` struct. See its definition in [`Include/RudeDrawer.h`](../Include/RudeDrawer.h).
- `draw.stopPollingEventsWindow()` - Tells the AppDrawer server to stop sending events to a window.
- `draw.removeWindowCallback()` - Remove the callback set by `Draw::setWindowCallback()`. Not removing the callback can lead to undefined behavior.
- `draw.removeWindow()` - Removes a window.
- `display.destroy()` - Destroy a `Display` instance.

## Error handling

LibDraw uses standard C++ error handling. To know whether a function throws or not, you can look at its signature, that should contain `noexcept(true)` or `noexcept(false)`. All LibDraw exceptions have the type of `std::runtime_error`.  
**You don't need to write error messages, you only need to catch a function's `std::runtime_error` and print the message returned by `.what()`.**

## Thread safety

All LibDraw functions (except `Draw::sendPaintEvent`) are **NOT** thread safe. This means that these functions should only be called by one thread. This happens because all AppDrawer commands have a response, and another command should only be executed after another command has received its response.  
**`SEND_PAINT_EVENT` does not receive a response after its execution, which means that it should be fine to call it at any time on another thread.**

TODO: Write the rest of the documentation lmao

#include <ranges>
#include <raylib.h>
#include <sys/signal.h>

#include "AppDrawer.h"
#include "Consts.h"
#include "RudeDrawer.h"
#include "Util.h"

#define BORDER_THICKNESS 5
#define TITLEBAR_THICKNESS 20.0f

void closeButton(AppDrawer* appdrawer, Window* window, Rectangle titleBarRect) noexcept(true)
{
    auto active = window->id == appdrawer->windows.back()->id;

    Rectangle closeButtonRect = {
        .x = titleBarRect.x,
        .y = titleBarRect.y,
        .width = TITLEBAR_THICKNESS,
        .height = TITLEBAR_THICKNESS,
    };
    DrawRectangleRec(closeButtonRect, RED);

    // Close button logic
    if (CheckCollisionPointRec(GetMousePosition(), closeButtonRect)
        && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)
        && active) {
        RudeDrawerEvent event;
        event.kind = RDEVENT_CLOSE_WIN;
        window->sendEvent(event);
    }
}

void titleBar(AppDrawer* appdrawer, Window* window) noexcept(true)
{
    auto active = window->id == appdrawer->windows.back()->id;

    Rectangle titleBarRect = {
        .x = window->area.x - BORDER_THICKNESS,
        .y = window->area.y - BORDER_THICKNESS - TITLEBAR_THICKNESS,
        .width = window->area.width + BORDER_THICKNESS * 2,
        .height = TITLEBAR_THICKNESS,
    };
    DrawRectangleRec(titleBarRect, YELLOW);

    closeButton(appdrawer, window, titleBarRect);

    DrawText(window->title.c_str(), window->area.x + TITLEBAR_THICKNESS + 1, titleBarRect.y,
        titleBarRect.height, BLUE);

    if (CheckCollisionPointRec(GetMousePosition(), titleBarRect)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            window->isDragging = true;
        }
    }

    if (window->isDragging) {
        if (active) {
            auto delta = GetMouseDelta();
            window->area.x += delta.x;
            window->area.y += delta.y;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            window->isDragging = false;
        }
    }
}

void windowBorders(Window* window) noexcept(true)
{
    auto border = window->area;
    border.width += BORDER_THICKNESS * 2;
    border.height += BORDER_THICKNESS * 2;
    border.x -= BORDER_THICKNESS;
    border.y -= BORDER_THICKNESS;
    DrawRectangleLinesEx(border, BORDER_THICKNESS, BLUE);
}

void windowDecoration(AppDrawer* appdrawer, Window* window) noexcept(true)
{
    windowBorders(window);
    titleBar(appdrawer, window);
}

int main() noexcept(true)
{
    signal(SIGPIPE, SIG_IGN);

    auto appdrawer = new AppDrawer();
    TRY(appdrawer->startServer());

    InitWindow(WIDTH, HEIGHT, "AppDrawer");

    SetTraceLogLevel(LOG_WARNING);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        Texture2D texture;
        appdrawer->windowsMutex.lock();
        for (auto w : appdrawer->windows) {
            BeginScissorMode(w->area.x, w->area.y, w->area.width, w->area.height);
            Image image = {
                .data = w->pixels,
                .width = (int)w->area.width,
                .height = (int)w->area.height,
                .mipmaps = 1,
                .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            };
            texture = LoadTextureFromImage(image);
            DrawTexture(texture, w->area.x, w->area.y, WHITE);
            EndScissorMode();

            windowDecoration(appdrawer, w);
        }
        appdrawer->windowsMutex.unlock();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !appdrawer->windows.empty()) {
            for (auto i = appdrawer->windows.size() - 1; i-- > 0;) {
                auto& w = appdrawer->windows[i];
                auto windowArea = w->area;
                windowArea.y -= BORDER_THICKNESS + TITLEBAR_THICKNESS;
                windowArea.x -= BORDER_THICKNESS;
                windowArea.width += BORDER_THICKNESS * 2;
                windowArea.height += TITLEBAR_THICKNESS + BORDER_THICKNESS * 2;
                if (CheckCollisionPointRec(GetMousePosition(), windowArea)) {
                    if (w->id != appdrawer->windows.back()->id) {
                        appdrawer->changeActiveWindow(w->id);
                    }
                    break;
                }
            }
        }

        EndDrawing();
        UnloadTexture(texture);
    }
    SetTraceLogLevel(LOG_INFO);

    CloseWindow();

    return 0;
}

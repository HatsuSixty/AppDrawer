#include <ranges>
#include <raylib.h>
#include <sys/signal.h>

#include "AppDrawer.h"
#include "Consts.h"
#include "RudeDrawer.h"
#include "Util.h"

#define BORDER_THICKNESS 5
#define TITLEBAR_THICKNESS 20.0f

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
        for (auto& w : appdrawer->windows) {
            BeginScissorMode(w->area.x, w->area.y, w->area.width, w->area.height);

            // Draw content
            Image image = {
                .data = w->pixels,
                .width = (int)w->area.width,
                .height = (int)w->area.height,
                .mipmaps = 1,
                .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            };
            texture = LoadTextureFromImage(image);
            DrawTexture(texture, w->area.x, w->area.y, WHITE);

            // Draw title
            DrawText(w->title.c_str(), w->area.x + 1, w->area.y, 30, BLUE);

            EndScissorMode();

            // Draw borders
            auto border = w->area;
            border.width += BORDER_THICKNESS * 2;
            border.height += BORDER_THICKNESS * 2;
            border.x -= BORDER_THICKNESS;
            border.y -= BORDER_THICKNESS;
            DrawRectangleLinesEx(border, BORDER_THICKNESS, BLUE);

            auto active = w->id == appdrawer->windows.back()->id;

            // Draw title bar
            Rectangle titleBarRect = {
                .x = w->area.x - BORDER_THICKNESS,
                .y = w->area.y - BORDER_THICKNESS - TITLEBAR_THICKNESS,
                .width = w->area.width + BORDER_THICKNESS * 2,
                .height = TITLEBAR_THICKNESS,
            };
            DrawRectangleRec(titleBarRect, YELLOW);

            // Title bar logic
            if (CheckCollisionPointRec(GetMousePosition(), titleBarRect)) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    w->isDragging = true;
                }
            }

            if (w->isDragging) {
                if (active) {
                    auto delta = GetMouseDelta();
                    w->area.x += delta.x;
                    w->area.y += delta.y;
                }

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    w->isDragging = false;
                }
            }

            // Draw close button
            auto closeButtonDims = 20.0f;
            Rectangle closeButtonRect = {
                .x = w->area.x - BORDER_THICKNESS,
                .y = w->area.y - BORDER_THICKNESS - closeButtonDims,
                .width = closeButtonDims,
                .height = closeButtonDims,
            };
            DrawRectangleRec(closeButtonRect, RED);

            // Close button logic
            if (CheckCollisionPointRec(GetMousePosition(), closeButtonRect)
                && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)
                && active) {
                RudeDrawerEvent event;
                event.kind = RDEVENT_CLOSE_WIN;
                w->sendEvent(event);
            }
        }
        appdrawer->windowsMutex.unlock();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !appdrawer->windows.empty()) {
            for (auto& w : std::ranges::views::reverse(appdrawer->windows)) {
                auto windowArea = w->area;
                windowArea.y -= BORDER_THICKNESS + TITLEBAR_THICKNESS;
                windowArea.x -= BORDER_THICKNESS;
                windowArea.width += BORDER_THICKNESS*2;
                windowArea.height += TITLEBAR_THICKNESS + BORDER_THICKNESS*2;
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

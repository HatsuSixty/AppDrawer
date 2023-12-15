#include <raylib.h>
#include <sys/signal.h>

#include "AppDrawer.h"
#include "Consts.h"
#include "RudeDrawer.h"
#include "Util.h"

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
            auto borderThickness = 5;
            auto border = w->area;
            border.width += borderThickness * 2;
            border.height += borderThickness * 2;
            border.x -= borderThickness;
            border.y -= borderThickness;
            DrawRectangleLinesEx(border, borderThickness, BLUE);

            // Draw title bar
            auto titleBarThickness = 20.0f;
            Rectangle titleBarRect = {
                .x = w->area.x - borderThickness,
                .y = w->area.y - borderThickness - titleBarThickness,
                .width = w->area.width + borderThickness * 2,
                .height = titleBarThickness,
            };
            DrawRectangleRec(titleBarRect, YELLOW);

            // Title bar logic
            if (CheckCollisionPointRec(GetMousePosition(), titleBarRect)) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    w->isDragging = true;
                }
            }

            if (w->isDragging) {
                auto delta = GetMouseDelta();
                w->area.x += delta.x;
                w->area.y += delta.y;

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    w->isDragging = false;
                }
            }

            // Draw close button
            auto closeButtonDims = 20.0f;
            Rectangle closeButtonRect = {
                .x = w->area.x - borderThickness,
                .y = w->area.y - borderThickness - closeButtonDims,
                .width = closeButtonDims,
                .height = closeButtonDims,
            };
            DrawRectangleRec(closeButtonRect, RED);

            // Close button logic
            if (CheckCollisionPointRec(GetMousePosition(), closeButtonRect)
                && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)
                && w->active) {
                RudeDrawerEvent event;
                event.kind = RDEVENT_CLOSE_WIN;
                w->sendEvent(event);
            }
        }
        appdrawer->windowsMutex.unlock();

        EndDrawing();
        UnloadTexture(texture);
    }
    SetTraceLogLevel(LOG_INFO);

    CloseWindow();

    return 0;
}

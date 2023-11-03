#include <raylib.h>

#include "AppDrawer.h"
#include "Consts.h"
#include "RudeDrawer.h"
#include "Util.h"

int main() noexcept
{
    auto appdrawer = new AppDrawer();
    TRY(appdrawer->startServer());

    InitWindow(WIDTH, HEIGHT, "AppDrawer");

    SetTraceLogLevel(LOG_WARNING);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        Texture2D texture;
        for (auto& w : appdrawer->windows) {
            BeginScissorMode(w.area.x, w.area.y, w.area.width, w.area.height);

            // Draw content
            Image image = {
                .data = w.pixels,
                .width = (int)w.area.width,
                .height = (int)w.area.height,
                .mipmaps = 1,
                .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            };
            texture = LoadTextureFromImage(image);
            DrawTexture(texture, w.area.x, w.area.y, WHITE);

            // Draw title
            DrawText(w.title.c_str(), w.area.x + 1, w.area.y, 30, BLUE);

            EndScissorMode();

            // Draw borders
            auto border_thickness = 5;
            auto border = w.area;
            border.width += border_thickness * 2;
            border.height += border_thickness * 2;
            border.x -= border_thickness;
            border.y -= border_thickness;
            DrawRectangleLinesEx(border, border_thickness, BLUE);
        }

        EndDrawing();
        UnloadTexture(texture);
    }
    SetTraceLogLevel(LOG_INFO);

    CloseWindow();

    return 0;
}

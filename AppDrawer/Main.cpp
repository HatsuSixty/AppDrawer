#include <ranges>
#include <raylib.h>
#include <raymath.h>
#include <string.h>
#include <sys/signal.h>

#include "AppDrawer.h"
#include "RudeDrawer.h"
#include "Util.h"

#define BORDER_THICKNESS 5
#define TITLEBAR_THICKNESS 20.0f

void closeButton(AppDrawer* appdrawer, Window* window, Rectangle titleBarRect) noexcept(true)
{
    auto active = window->m_id == appdrawer->m_windows.back()->m_id;

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
    auto active = window->m_id == appdrawer->m_windows.back()->m_id;

    Rectangle titleBarRect = {
        .x = window->m_area.x - BORDER_THICKNESS,
        .y = window->m_area.y - BORDER_THICKNESS - TITLEBAR_THICKNESS,
        .width = window->m_area.width + BORDER_THICKNESS * 2,
        .height = TITLEBAR_THICKNESS,
    };
    DrawRectangleRec(titleBarRect, YELLOW);

    closeButton(appdrawer, window, titleBarRect);

    DrawText(window->m_title.c_str(), window->m_area.x + TITLEBAR_THICKNESS + 1, titleBarRect.y,
        titleBarRect.height, BLUE);

    if (CheckCollisionPointRec(GetMousePosition(), titleBarRect)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            window->m_isDragging = true;
        }
    }

    if (window->m_isDragging) {
        if (active) {
            auto delta = GetMouseDelta();
            window->m_area.x += delta.x;
            window->m_area.y += delta.y;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            window->m_isDragging = false;
        }
    }
}

void windowBorders(Window* window) noexcept(true)
{
    auto border = window->m_area;
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

    RudeDrawerKey allKeys[] = {
        RDKEY_NULL,
        // Alphanumeric keys
        RDKEY_APOSTROPHE,
        RDKEY_COMMA,
        RDKEY_MINUS,
        RDKEY_PERIOD,
        RDKEY_SLASH,
        RDKEY_ZERO,
        RDKEY_ONE,
        RDKEY_TWO,
        RDKEY_THREE,
        RDKEY_FOUR,
        RDKEY_FIVE,
        RDKEY_SIX,
        RDKEY_SEVEN,
        RDKEY_EIGHT,
        RDKEY_NINE,
        RDKEY_SEMICOLON,
        RDKEY_EQUAL,
        RDKEY_A,
        RDKEY_B,
        RDKEY_C,
        RDKEY_D,
        RDKEY_E,
        RDKEY_F,
        RDKEY_G,
        RDKEY_H,
        RDKEY_I,
        RDKEY_J,
        RDKEY_K,
        RDKEY_L,
        RDKEY_M,
        RDKEY_N,
        RDKEY_O,
        RDKEY_P,
        RDKEY_Q,
        RDKEY_R,
        RDKEY_S,
        RDKEY_T,
        RDKEY_U,
        RDKEY_V,
        RDKEY_W,
        RDKEY_X,
        RDKEY_Y,
        RDKEY_Z,
        RDKEY_LEFT_BRACKET,
        RDKEY_BACKSLASH,
        RDKEY_RIGHT_BRACKET,
        RDKEY_GRAVE,
        // Function keys
        RDKEY_SPACE,
        RDKEY_ESCAPE,
        RDKEY_ENTER,
        RDKEY_TAB,
        RDKEY_BACKSPACE,
        RDKEY_INSERT,
        RDKEY_DELETE,
        RDKEY_RIGHT,
        RDKEY_LEFT,
        RDKEY_DOWN,
        RDKEY_UP,
        RDKEY_PAGE_UP,
        RDKEY_PAGE_DOWN,
        RDKEY_HOME,
        RDKEY_END,
        RDKEY_CAPS_LOCK,
        RDKEY_SCROLL_LOCK,
        RDKEY_NUM_LOCK,
        RDKEY_PRINT_SCREEN,
        RDKEY_PAUSE,
        RDKEY_F1,
        RDKEY_F2,
        RDKEY_F3,
        RDKEY_F4,
        RDKEY_F5,
        RDKEY_F6,
        RDKEY_F7,
        RDKEY_F8,
        RDKEY_F9,
        RDKEY_F10,
        RDKEY_F11,
        RDKEY_F12,
        RDKEY_LEFT_SHIFT,
        RDKEY_LEFT_CONTROL,
        RDKEY_LEFT_ALT,
        RDKEY_LEFT_SUPER,
        RDKEY_RIGHT_SHIFT,
        RDKEY_RIGHT_CONTROL,
        RDKEY_RIGHT_ALT,
        RDKEY_RIGHT_SUPER,
        RDKEY_KB_MENU,
        // Keypad keys
        RDKEY_KP_0,
        RDKEY_KP_1,
        RDKEY_KP_2,
        RDKEY_KP_3,
        RDKEY_KP_4,
        RDKEY_KP_5,
        RDKEY_KP_6,
        RDKEY_KP_7,
        RDKEY_KP_8,
        RDKEY_KP_9,
        RDKEY_KP_DECIMAL,
        RDKEY_KP_DIVIDE,
        RDKEY_KP_MULTIPLY,
        RDKEY_KP_SUBTRACT,
        RDKEY_KP_ADD,
        RDKEY_KP_ENTER,
        RDKEY_KP_EQUAL
    };

    auto ruid = getuid();
    auto rgid = getgid();

    InitWindow(640, 480, "AppDrawer");

    if (seteuid(ruid) < 0) {
        fprintf(stderr, "WARNING: Could not set Effective UID to the real one: %s\n", strerror(errno));
    }

    if (setegid(rgid) < 0) {
        fprintf(stderr, "WARNING: Could not set Effective GID to the real one: %s\n", strerror(errno));
    }

    AppDrawer* appdrawer;
    TRY(appdrawer = new AppDrawer());

    SetTraceLogLevel(LOG_WARNING);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(LIGHTGRAY);

        Texture2D texture;
        // Lock mutex before modifying `appdrawer->m_windows`' contents
        appdrawer->m_windowsMutex.lock();

        // Draw windows
        for (auto w : appdrawer->m_windows) {
            BeginScissorMode(w->m_area.x, w->m_area.y, w->m_area.width, w->m_area.height);
            Image image = {
                .data = w->m_pixels,
                .width = (int)w->m_area.width,
                .height = (int)w->m_area.height,
                .mipmaps = 1,
                .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            };
            texture = LoadTextureFromImage(image);
            DrawTexture(texture, w->m_area.x, w->m_area.y, WHITE);
            EndScissorMode();

            windowDecoration(appdrawer, w);
        }

        // Handle key events
        for (size_t i = 0; i < sizeof(allKeys) / sizeof(allKeys[0]) && !appdrawer->m_windows.empty(); ++i) {
            RudeDrawerEventKind eventKind;
            if (IsKeyPressed(allKeys[i]))
                eventKind = RDEVENT_KEYPRESS;
            else if (IsKeyReleased(allKeys[i]))
                eventKind = RDEVENT_KEYRELEASE;
            else
                continue;

            RudeDrawerEvent event;
            event.kind = eventKind;
            event.key = allKeys[i];
            appdrawer->m_windows.back()->sendEvent(event);
        }

        // Handle mouse events
        if (!appdrawer->m_windows.empty()) {
            RudeDrawerEvent mouseEvent;
            mouseEvent.kind = (RudeDrawerEventKind)0;
            auto mouseWheelMove = GetMouseWheelMove();
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                mouseEvent.kind = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ? RDEVENT_MOUSEPRESS : RDEVENT_MOUSERELEASE;
                mouseEvent.mouseButton = RDMOUSE_LEFT;
            } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
                mouseEvent.kind = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ? RDEVENT_MOUSEPRESS : RDEVENT_MOUSERELEASE;
                mouseEvent.mouseButton = RDMOUSE_RIGHT;
            } else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) || IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE)) {
                mouseEvent.kind = IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) ? RDEVENT_MOUSEPRESS : RDEVENT_MOUSERELEASE;
                mouseEvent.mouseButton = RDMOUSE_MIDDLE;
            } else if (mouseWheelMove > 0 || mouseWheelMove < 0) {
                mouseEvent.kind = RDEVENT_MOUSEPRESS;
                mouseEvent.mouseButton = (mouseWheelMove > 0) ? RDMOUSE_UP : RDMOUSE_DOWN;
            }

            if (mouseEvent.kind != 0) {
                appdrawer->m_windows.back()->sendEvent(mouseEvent);
            }
        }

        if (!appdrawer->m_windows.empty()) {
            if (!Vector2Equals(Vector2Zero(), GetMouseDelta())
                && CheckCollisionPointRec(GetMousePosition(), appdrawer->m_windows.back()->m_area)) {
                RudeDrawerEvent event;
                event.kind = RDEVENT_MOUSEMOVE;
                appdrawer->m_windows.back()->sendEvent(event);
            }
        }

        // Unlock mutex after modifying `appdrawer->m_windows`' contents
        appdrawer->m_windowsMutex.unlock();

        // Handle window focus
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !appdrawer->m_windows.empty()) {
            for (auto i = appdrawer->m_windows.size() - 1; i-- > 0;) {
                auto& w = appdrawer->m_windows[i];

                auto windowArea = w->m_area;
                windowArea.y -= BORDER_THICKNESS + TITLEBAR_THICKNESS;
                windowArea.x -= BORDER_THICKNESS;
                windowArea.width += BORDER_THICKNESS * 2;
                windowArea.height += TITLEBAR_THICKNESS + BORDER_THICKNESS * 2;

                if (CheckCollisionPointRec(GetMousePosition(), windowArea)) {
                    if (w->m_id != appdrawer->m_windows.back()->m_id) {
                        appdrawer->changeActiveWindow(w->m_id);
                    }
                    break;
                }
            }
        }

        appdrawer->setMousePosition(GetMousePosition());

        EndDrawing();
        UnloadTexture(texture);
    }
    SetTraceLogLevel(LOG_INFO);

    CloseWindow();

    return 0;
}

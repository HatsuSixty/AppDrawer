#pragma once

// RudeDrawer.h - Contains definitions of structures and constants used by the AppDrawer server to communicate with its clients.
// If you want to do anything with the server, start by sending a `RudeDrawerCommand` (defined and documented in this header) to it.

#include <stdint.h>
#include <stdbool.h>

#define SOCKET_PATH "/tmp/AppDrawer.sock"
// RGBA
#define COMPONENTS 4

// These are all of the command types that can be sent to AppDrawer.
// Commands can either return a `RudeDrawerResponse` struct (defined and documented in this header) or nothing.
// The client should NOT wait for a response from commands that don't return anything.
typedef enum {
    // Makes the server print "Pong!" in its logs.
    // Required arguments: None
    // Returns: `RDRESP_EMPTY`
    RDCMD_PING,
    // Adds a Window.
    // Required arguments:
    //   - `windowDims`
    //   - `windowTitle` (has to fit in `WINDOW_TITLE_MAX`)
    // Returns: `RDRESP_WINID`
    RDCMD_ADD_WIN,
    // Removes a window.
    // Required arguments:
    //   - `windowId`
    // Returns: `RDRESP_EMPTY`
    RDCMD_REMOVE_WIN,
    // Makes the server start sending events to the client.
    // The event will be a struct of the type `RudeDrawerEvent` (defined and documented in this header).
    // Required arguments:
    //   - `windowId`
    // Returns: `RDRESP_EMPTY`
    RDCMD_START_POLLING_EVENTS_WIN,
    // Makes the server stop sending events to the client.
    // Required arguments:
    //   - `windowId`
    // Returns: `RDRESP_EMPTY`
    RDCMD_STOP_POLLING_EVENTS_WIN,
    // Returns the name of a shared memory that contains the pixels of the specified window (specified by `windowId`).
    // Required arguments:
    //   - `windowId`
    // Returns: `RDRESP_SHM_NAME`
    RDCMD_GET_DISPLAY_SHM_WIN,
    // Makes the server send a paint event to the specified window (specified by `windowId`).
    // Required arguments:
    //   - `windowId`
    // Returns: None
    RDCMD_SEND_PAINT_EVENT,
    // Returns the mouse position within the specified window (specified by `windowId`).
    // Required arguments:
    //   - `windowId`
    // Returns: `RDRESP_MOUSE_POSITION`
    RDCMD_GET_MOUSE_POSITION,
} RudeDrawerCommandKind;

// This is a struct that contains two `uint32_t`s.
typedef struct {
    // x
    uint32_t x;
    // y
    uint32_t y;
} RudeDrawerVec2D;

// This is a struct that, when sent over `SOCKET_PATH`, makes the server execute a command.
#define WINDOW_TITLE_MAX 256
typedef struct {
    // The kind of command.
    // Type: `RudeDrawerCommandKind` (defined and documented in this header)
    RudeDrawerCommandKind kind;
    // The dimensions of a window.
    // Type: `RudeDrawerVec2D` (defined and documented in this header)
    RudeDrawerVec2D windowDims;
    // The title of a window.
    // Type: `char[WINDOW_TITLE_MAX]`
    char* windowTitle[WINDOW_TITLE_MAX];
    // The ID of a window.
    // Type: `uint32_t`
    uint32_t windowId;
} RudeDrawerCommand;

// These are the possible kinds of response.
typedef enum {
    // Nothing.
    RDRESP_EMPTY,
    // The ID of a window.
    RDRESP_WINID,
    // The name of a shared memory.
    RDRESP_SHM_NAME,
    // The dimensions of a window.
    RDRESP_DIMENSIONS,
    // The mouse position within a window.
    RDRESP_MOUSE_POSITION,
} RudeDrawerResponseKind;

// These are all of the possible error codes.
typedef enum {
    // Indicates an invalid command.
    RDERROR_INVALID_COMMAND,
    // Indicates an invalid window ID.
    RDERROR_INVALID_WINID,
    // Indicates that `RDCMD_ADD_WIN` failed.
    RDERROR_ADD_WIN_FAILED,
    // No error happened.
    RDERROR_OK,
} RudeDrawerErrorKind;

// This is a struct that can be returned by some commands.
#define WINDOW_SHM_NAME_MAX 256
typedef struct {
    // The kind of response.
    // Type: `RudeDrawerResponseKind` (defined and documented in this header)
    RudeDrawerResponseKind kind;
    // Contains the error code of the performed operation.
    // Type: `RudeDrawerErrorKind` (defined and documented in this header)
    RudeDrawerErrorKind errorKind;
    // The ID of a window.
    // Type: `uint32_t`
    uint32_t windowId;
    // The name of a shared memory that contains the pixels of a window.
    // Type: `char[WINDOW_SHM_NAME_MAX]`
    char windowShmName[WINDOW_SHM_NAME_MAX];
    // The dimensions of a window.
    // Type: `RudeDrawerVec2D` (defined and documented in this header)
    RudeDrawerVec2D dimensions;
    // The position of the cursor within a window.
    // Type: `RudeDrawerVec2D` (defined and documented in this header)
    RudeDrawerVec2D mousePos;
} RudeDrawerResponse;

// These are all the keyboard keys.
typedef enum {
    RDKEY_NULL            = 0,
    // Alphanumeric keys
    RDKEY_APOSTROPHE      = 39,
    RDKEY_COMMA           = 44,
    RDKEY_MINUS           = 45,
    RDKEY_PERIOD          = 46,
    RDKEY_SLASH           = 47,
    RDKEY_ZERO            = 48,
    RDKEY_ONE             = 49,
    RDKEY_TWO             = 50,
    RDKEY_THREE           = 51,
    RDKEY_FOUR            = 52,
    RDKEY_FIVE            = 53,
    RDKEY_SIX             = 54,
    RDKEY_SEVEN           = 55,
    RDKEY_EIGHT           = 56,
    RDKEY_NINE            = 57,
    RDKEY_SEMICOLON       = 59,
    RDKEY_EQUAL           = 61,
    RDKEY_A               = 65,
    RDKEY_B               = 66,
    RDKEY_C               = 67,
    RDKEY_D               = 68,
    RDKEY_E               = 69,
    RDKEY_F               = 70,
    RDKEY_G               = 71,
    RDKEY_H               = 72,
    RDKEY_I               = 73,
    RDKEY_J               = 74,
    RDKEY_K               = 75,
    RDKEY_L               = 76,
    RDKEY_M               = 77,
    RDKEY_N               = 78,
    RDKEY_O               = 79,
    RDKEY_P               = 80,
    RDKEY_Q               = 81,
    RDKEY_R               = 82,
    RDKEY_S               = 83,
    RDKEY_T               = 84,
    RDKEY_U               = 85,
    RDKEY_V               = 86,
    RDKEY_W               = 87,
    RDKEY_X               = 88,
    RDKEY_Y               = 89,
    RDKEY_Z               = 90,
    RDKEY_LEFT_BRACKET    = 91,
    RDKEY_BACKSLASH       = 92,
    RDKEY_RIGHT_BRACKET   = 93,
    RDKEY_GRAVE           = 96,
    // Function keys
    RDKEY_SPACE           = 32,
    RDKEY_ESCAPE          = 256,
    RDKEY_ENTER           = 257,
    RDKEY_TAB             = 258,
    RDKEY_BACKSPACE       = 259,
    RDKEY_INSERT          = 260,
    RDKEY_DELETE          = 261,
    RDKEY_RIGHT           = 262,
    RDKEY_LEFT            = 263,
    RDKEY_DOWN            = 264,
    RDKEY_UP              = 265,
    RDKEY_PAGE_UP         = 266,
    RDKEY_PAGE_DOWN       = 267,
    RDKEY_HOME            = 268,
    RDKEY_END             = 269,
    RDKEY_CAPS_LOCK       = 280,
    RDKEY_SCROLL_LOCK     = 281,
    RDKEY_NUM_LOCK        = 282,
    RDKEY_PRINT_SCREEN    = 283,
    RDKEY_PAUSE           = 284,
    RDKEY_F1              = 290,
    RDKEY_F2              = 291,
    RDKEY_F3              = 292,
    RDKEY_F4              = 293,
    RDKEY_F5              = 294,
    RDKEY_F6              = 295,
    RDKEY_F7              = 296,
    RDKEY_F8              = 297,
    RDKEY_F9              = 298,
    RDKEY_F10             = 299,
    RDKEY_F11             = 300,
    RDKEY_F12             = 301,
    RDKEY_LEFT_SHIFT      = 340,
    RDKEY_LEFT_CONTROL    = 341,
    RDKEY_LEFT_ALT        = 342,
    RDKEY_LEFT_SUPER      = 343,
    RDKEY_RIGHT_SHIFT     = 344,
    RDKEY_RIGHT_CONTROL   = 345,
    RDKEY_RIGHT_ALT       = 346,
    RDKEY_RIGHT_SUPER     = 347,
    RDKEY_KB_MENU         = 348,
    // Keypad keys
    RDKEY_KP_0            = 320,
    RDKEY_KP_1            = 321,
    RDKEY_KP_2            = 322,
    RDKEY_KP_3            = 323,
    RDKEY_KP_4            = 324,
    RDKEY_KP_5            = 325,
    RDKEY_KP_6            = 326,
    RDKEY_KP_7            = 327,
    RDKEY_KP_8            = 328,
    RDKEY_KP_9            = 329,
    RDKEY_KP_DECIMAL      = 330,
    RDKEY_KP_DIVIDE       = 331,
    RDKEY_KP_MULTIPLY     = 332,
    RDKEY_KP_SUBTRACT     = 333,
    RDKEY_KP_ADD          = 334,
    RDKEY_KP_ENTER        = 335,
    RDKEY_KP_EQUAL        = 336,
} RudeDrawerKey;

// These are all the mouse buttons.
typedef enum {
    RDMOUSE_UP,
    RDMOUSE_DOWN,
    RDMOUSE_LEFT,
    RDMOUSE_RIGHT,
    RDMOUSE_MIDDLE,
} RudeDrawerMouseButton;

// These are all of the possible event kinds.
typedef enum {
    // No event.
    RDEVENT_NONE,
    // The "close" button has been clicked.
    RDEVENT_CLOSE_WIN,
    // The window has to update itself.
    RDEVENT_PAINT,
    // A key has been pressed.
    RDEVENT_KEYPRESS,
    // A key has been released.
    RDEVENT_KEYRELEASE,
    // A mouse button has been pressed.
    RDEVENT_MOUSEPRESS,
    // A mouse button has been released.
    RDEVENT_MOUSERELEASE,
} RudeDrawerEventKind;

// This struct defines an event that can be sent to a client.
typedef struct {
    // The kind of event.
    // Type: `RudeDrawerEventKind` (defined and documented in this header)
    RudeDrawerEventKind kind;
    // A keyboard key.
    // Type: `RudeDrawerKey` (defined and documented in this header)
    RudeDrawerKey key;
    // A mouse button.
    // Type: `RudeDrawerMouseButton` (defined and documented in this header)
    RudeDrawerMouseButton mouseButton;
} RudeDrawerEvent;

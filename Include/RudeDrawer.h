#pragma once

// This header contains definitions of structures and constants used by the AppDrawer server to communicate with its clients.
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
} RudeDrawerResponse;

// These are all of the possible event kinds.
typedef enum {
    // No event.
    RDEVENT_NONE,
    // The "close" button has been clicked.
    RDEVENT_CLOSE_WIN,
    // The window has to update itself.
    RDEVENT_PAINT,
} RudeDrawerEventKind;

// This struct defines an event that can be sent to a client.
typedef struct {
    // The kind of event.
    // Type: `RudeDrawerEventKind` (defined and documented in this header)
    RudeDrawerEventKind kind;
} RudeDrawerEvent;

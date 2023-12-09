#pragma once

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

// A struct that contains two `uint32_t`s.
typedef struct {
    // x
    uint32_t x;
    // y
    uint32_t y;
} RudeDrawerVec2D;

#define WINDOW_TITLE_MAX 256
typedef struct {
    RudeDrawerCommandKind kind;
    RudeDrawerVec2D windowDims;
    char* windowTitle[WINDOW_TITLE_MAX];
    uint32_t windowId;
} RudeDrawerCommand;

typedef enum {
    RDRESP_EMPTY,
    RDRESP_WINID,
    RDRESP_SHM_NAME,
    RDRESP_DIMENSIONS,
} RudeDrawerResponseKind;

typedef enum {
    RDERROR_INVALID_COMMAND,
    RDERROR_INVALID_WINID,
    RDERROR_ADD_WIN_FAILED,
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
    // Type: char[WINDOW_SHM_NAME_MAX]
    char windowShmName[WINDOW_SHM_NAME_MAX];
    // The dimensions of a window.
    // Type: `RudeDrawerVec2D` (defined and documented in this header)
    RudeDrawerVec2D dimensions;
} RudeDrawerResponse;

typedef enum {
    RDEVENT_NONE,
    RDEVENT_CLOSE_WIN,
    RDEVENT_PAINT,
} RudeDrawerEventKind;

typedef struct {
    RudeDrawerEventKind kind;
} RudeDrawerEvent;

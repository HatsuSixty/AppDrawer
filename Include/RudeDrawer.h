#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SOCKET_PATH "/tmp/AppDrawer.sock"
// RGBA
#define COMPONENTS 4

// These are all of the command types that can be sent to AppDrawer.
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

typedef struct {
    uint32_t x;
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

#define WINDOW_SHM_NAME_MAX 256
typedef struct {
    RudeDrawerResponseKind kind;
    RudeDrawerErrorKind errorKind;
    uint32_t windowId;
    char windowShmName[WINDOW_SHM_NAME_MAX];
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

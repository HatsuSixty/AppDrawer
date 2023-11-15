#pragma once

#include <cstdint>

#define SOCKET_PATH "/tmp/AppDrawer.sock"
// RGBA
#define COMPONENTS 4

typedef enum {
    RDCMD_PING,
    RDCMD_ADD_WIN,
    RDCMD_REMOVE_WIN,
    RDCMD_START_POLLING_EVENTS_WIN,
    RDCMD_STOP_POLLING_EVENTS_WIN,
    RDCMD_GET_DISPLAY_SHM_WIN,
} RudeDrawerCommandKind;

#define WINDOW_TITLE_MAX 256
typedef struct {
    RudeDrawerCommandKind kind;
    uint32_t windowWidth;
    uint32_t windowHeight;
    char* windowTitle[WINDOW_TITLE_MAX];
    uint32_t windowId;
} RudeDrawerCommand;

typedef struct {
    uint32_t x;
    uint32_t y;
} RudeDrawerVec2D;

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
} RudeDrawerEventKind;

typedef struct {
    RudeDrawerEventKind kind;
} RudeDrawerEvent;

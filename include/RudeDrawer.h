#pragma once

#include <cstdint>

#define SOCKET_PATH "/tmp/AppDrawer.sock"

typedef enum {
    RDCMD_PING,
    RDCMD_ADD_WIN,
    RDCMD_REMOVE_WIN,
    RDCMD_START_POLLING_EVENTS_WIN,
    RDCMD_STOP_POLLING_EVENTS_WIN,
} RudeDrawerCommandKind;

typedef struct {
    RudeDrawerCommandKind kind;
    uint32_t windowWidth;
    uint32_t windowHeight;
    char* windowTitle[256];
    uint32_t windowId;
} RudeDrawerCommand;

typedef enum {
    RDRESP_EMPTY,
    RDRESP_WINID,
} RudeDrawerResponseKind;

typedef enum {
    RDERROR_INVALID_COMMAND,
    RDERROR_INVALID_WINID,
    RDERROR_ADD_WIN_FAILED,
    RDERROR_OK,
} RudeDrawerErrorKind;

typedef struct {
    RudeDrawerResponseKind kind;
    RudeDrawerErrorKind errorKind;
    uint32_t windowId;
} RudeDrawerResponse;

typedef enum {
    RDEVENT_NONE,
    RDEVENT_CLOSE_WIN,
} RudeDrawerEventKind;

typedef struct {
    RudeDrawerEventKind kind;
} RudeDrawerEvent;

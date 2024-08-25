#pragma once

#include <stdlib.h>
#include <string.h>

#include <xcb/xcb.h>

// workaround for xcb-xkb to be c++ compatible
#define explicit explicit_
#include <xcb/xkb.h>
#undef explicit

#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-x11.h>

#include "sy_macros.hpp"

typedef struct
{
    xcb_connection_t *conn;
    xcb_screen_t *scr;
    xcb_window_t win;
    xcb_intern_atom_reply_t *wm_delete_reply;

    // xkb stuff
    struct xkb_context *xkb_ctx;
    struct xkb_keymap *xkb_keymap;
    struct xkb_state *xkb_state;
    
    int win_width;
    int win_height;
} SyXCBInfo;

void init_window(SyXCBInfo *result, int width, int height, const char *title);
void cleanup_window(SyXCBInfo *xcb_info);

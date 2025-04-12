#include "sy_linux_window.hpp"
#include <xcb/xproto.h>

void init_window(SyXCBInfo *result, int width, int height, const char *title)
{

    result->win_width = width;
    result->win_height = height;

    // Connection
    int scr_num = 0;
    result->conn = xcb_connect(NULL, &scr_num);
    if(xcb_connection_has_error(result->conn))
    {
	xcb_disconnect(result->conn);
	SY_ERROR_OUTPUT("ERROR: Failed to create a x connection (xcb).\n");
	exit(1);
    }

    // xcb_allow_events_checked(result->conn, XCB_ALLOW_ASYNC_BOTH, XCB_CURRENT_TIME);

    // Screen
    result->scr = xcb_setup_roots_iterator(xcb_get_setup(result->conn)).data;
    if(result->scr == NULL)
    {
	xcb_disconnect(result->conn);
	SY_ERROR_OUTPUT("ERROR: Failed to create a screen.\n");
	exit(1);
    }

    // Window Creation
    result->win = xcb_generate_id(result->conn);
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[] = {result->scr->black_pixel,
	XCB_EVENT_MASK_EXPOSURE |
	XCB_EVENT_MASK_KEY_PRESS |
	XCB_EVENT_MASK_KEY_RELEASE |
	XCB_EVENT_MASK_BUTTON_PRESS |
	XCB_EVENT_MASK_BUTTON_RELEASE |
	XCB_EVENT_MASK_POINTER_MOTION |
	XCB_EVENT_MASK_POINTER_MOTION_HINT |
	XCB_EVENT_MASK_BUTTON_1_MOTION |
	XCB_EVENT_MASK_BUTTON_2_MOTION |
	XCB_EVENT_MASK_BUTTON_3_MOTION |
	XCB_EVENT_MASK_BUTTON_4_MOTION |
	XCB_EVENT_MASK_BUTTON_5_MOTION |
	XCB_EVENT_MASK_BUTTON_MOTION
    };
    xcb_create_window(result->conn, XCB_COPY_FROM_PARENT, result->win, result->scr->root, 0, 0, width, height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, result->scr->root_visual, mask, values);

    // Change Title
    xcb_change_property(result->conn, XCB_PROP_MODE_REPLACE, result->win, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);

    // Get wm exit event
    xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(result->conn, 0, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *wm_protocols_reply = xcb_intern_atom_reply(result->conn, wm_protocols_cookie, NULL);
    xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(result->conn, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
    result->wm_delete_reply = xcb_intern_atom_reply(result->conn, wm_delete_cookie, NULL);
    xcb_change_property(result->conn, XCB_PROP_MODE_REPLACE, result->win, wm_protocols_reply->atom, XCB_ATOM_ATOM, 32, 1, &result->wm_delete_reply->atom);

    // show window on screen
    xcb_map_window(result->conn, result->win);

    xcb_flush(result->conn);

    free(wm_protocols_reply);

    // xkb stuff ***DO NOT FUCK WITH***

    xcb_xkb_use_extension(result->conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

    // create xkb context
    result->xkb_ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    SY_ERROR_COND(result->xkb_ctx == NULL, "ERROR: Failed to create a xkb context.");

    // default rules for keymap
    struct xkb_rule_names names =
    {
	.rules = NULL,
	.model = NULL,
	.layout = NULL,
	.variant = NULL,
	.options = NULL
    };
 
    // create xkb keymap
    result->xkb_keymap = xkb_keymap_new_from_names(result->xkb_ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    SY_ERROR_COND(result->xkb_keymap == NULL, "ERROR: Failed to get xkb keymap.");

    // create xkb state
    result->xkb_state = xkb_state_new(result->xkb_keymap);
    SY_ERROR_COND(result->xkb_state == NULL, "ERROR: Failed to create xkb state.");

    // turn off auto repeat
    xcb_xkb_per_client_flags(result->conn, XCB_XKB_ID_USE_CORE_KBD, XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT, 1, 0, 0, 0);

    // POINTER STUFF

/*
    xcb_grab_pointer_cookie_t grab_pointer_cookie = xcb_grab_pointer(result->conn, 1, result->win,
		     XCB_EVENT_MASK_BUTTON_PRESS |
		     XCB_EVENT_MASK_BUTTON_RELEASE |
		     XCB_EVENT_MASK_POINTER_MOTION |
		     XCB_EVENT_MASK_POINTER_MOTION_HINT |
		     XCB_EVENT_MASK_BUTTON_1_MOTION |
		     XCB_EVENT_MASK_BUTTON_2_MOTION |
		     XCB_EVENT_MASK_BUTTON_3_MOTION |
		     XCB_EVENT_MASK_BUTTON_4_MOTION |
		     XCB_EVENT_MASK_BUTTON_5_MOTION |
		     XCB_EVENT_MASK_BUTTON_MOTION, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, result->win, XCB_CURSOR_NONE, XCB_CURRENT_TIME);
    xcb_grab_pointer_reply_t *grab_pointer_reply = xcb_grab_pointer_reply(result->conn, grab_pointer_cookie, NULL);
    if (grab_pointer_reply->status != XCB_GRAB_STATUS_SUCCESS)
	SY_ERROR("Failed to grab pointer %d", grab_pointer_reply->status);
    
    free(grab_pointer_reply);
*/
}

void cleanup_window(SyXCBInfo *xcb_info)
{
    // xkb
    xkb_state_unref(xcb_info->xkb_state);
    xkb_keymap_unref(xcb_info->xkb_keymap);
    xkb_context_unref(xcb_info->xkb_ctx);

    // xcb
    free(xcb_info->wm_delete_reply);
    xcb_destroy_window(xcb_info->conn, xcb_info->win);
    xcb_disconnect(xcb_info->conn);
}

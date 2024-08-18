#include "sy_linux_input.hpp"
#include "sy_linux_window.hpp"
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

void handle_event_client_message(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event);
void handle_event_expose(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event);
void handle_event_key_press(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event);
void handle_event_key_release(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event);

void poll_events(SyXCBInfo *xcb_info, SyInputInfo *input_info)
{

    input_info->window_should_close = false;
    input_info->window_resized = false;

    xcb_generic_event_t *event;
    while ( (event = xcb_poll_for_event(xcb_info->conn)) )
    {
	switch (event->response_type & ~0x80)
	{
	    case XCB_CLIENT_MESSAGE:
		handle_event_client_message(xcb_info, input_info, event);
		break;

	    case XCB_EXPOSE:
		handle_event_expose(xcb_info, input_info, event);
		break;

	    case XCB_KEY_PRESS:
		handle_event_key_press(xcb_info, input_info, event);
		break;
		
	    case XCB_KEY_RELEASE:
		handle_event_key_release(xcb_info, input_info, event);

	}

	free(event);
    }
}

void handle_event_client_message(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event)
{
    xcb_client_message_event_t *message = (xcb_client_message_event_t*)event;
    if (message->data.data32[0] == xcb_info->wm_delete_reply->atom)
    {
	input_info->window_should_close = true;
    }
}

void handle_event_expose(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event)
{
    xcb_expose_event_t *message = (xcb_expose_event_t*)event;

    xcb_get_geometry_cookie_t cookie = xcb_get_geometry(xcb_info->conn, xcb_info->win);
    xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(xcb_info->conn, cookie, NULL);
 
    // Window resize condition
    if (xcb_info->win_width != reply->width || xcb_info->win_height != reply->height)
    {
	input_info->window_resized = true;
	xcb_info->win_width = reply->width;
	xcb_info->win_height = reply->height;
    }

    free(reply);
}


#define SY_KEY_SPECIFIC_CASE(key, xkb_ver, boolean) case XKB_KEY_##xkb_ver: input_info->key = boolean; break;
#define SY_KEY_CASE(key, boolean) SY_KEY_SPECIFIC_CASE(key, key, boolean);

void handle_event_key_release(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event)
{
    xcb_key_release_event_t *message = (xcb_key_release_event_t*)event;

    xkb_keysym_t keysym = xkb_state_key_get_one_sym(xcb_info->xkb_state, message->detail);
    switch (keysym)
    {
	SY_KEY_CASE(a, false);
	SY_KEY_CASE(b, false);
	SY_KEY_CASE(c, false);
	SY_KEY_CASE(d, false);
	SY_KEY_CASE(e, false);
	SY_KEY_CASE(f, false);
	SY_KEY_CASE(g, false);
	SY_KEY_CASE(h, false);
	SY_KEY_CASE(i, false);
	SY_KEY_CASE(j, false);
	SY_KEY_CASE(k, false);
	SY_KEY_CASE(l, false);
	SY_KEY_CASE(m, false);
	SY_KEY_CASE(n, false);
	SY_KEY_CASE(o, false);
	SY_KEY_CASE(p, false);
	SY_KEY_CASE(q, false);
	SY_KEY_CASE(r, false);
	SY_KEY_CASE(s, false);
	SY_KEY_CASE(t, false);
	SY_KEY_CASE(u, false);
	SY_KEY_CASE(v, false);
	SY_KEY_CASE(w, false);
	SY_KEY_CASE(x, false);
	SY_KEY_CASE(y, false);
	SY_KEY_CASE(z, false);
	SY_KEY_SPECIFIC_CASE(zero, 0, false);
	SY_KEY_SPECIFIC_CASE(one, 1, false);
	SY_KEY_SPECIFIC_CASE(two, 2, false);
	SY_KEY_SPECIFIC_CASE(three, 3, false);
	SY_KEY_SPECIFIC_CASE(four, 4, false);
	SY_KEY_SPECIFIC_CASE(five, 5, false);
	SY_KEY_SPECIFIC_CASE(six, 6, false);
	SY_KEY_SPECIFIC_CASE(seven, 7, false);
	SY_KEY_SPECIFIC_CASE(eight, 8, false);
	SY_KEY_SPECIFIC_CASE(nine, 9, false);
	SY_KEY_CASE(space, false);
	SY_KEY_SPECIFIC_CASE(shift_left, Shift_L, false);
	SY_KEY_SPECIFIC_CASE(shift_right, Shift_R, false);
	SY_KEY_SPECIFIC_CASE(control_left, Control_L, false);
	SY_KEY_SPECIFIC_CASE(control_right, Control_R, false);
	SY_KEY_SPECIFIC_CASE(alt_left, Alt_L, false);
	SY_KEY_SPECIFIC_CASE(alt_right, Alt_R, false);
	SY_KEY_SPECIFIC_CASE(tab, Tab, false);
	SY_KEY_SPECIFIC_CASE(space, KP_Space, false);
	SY_KEY_SPECIFIC_CASE(caps_lock, Caps_Lock, false);
	SY_KEY_SPECIFIC_CASE(back_space, BackSpace, false);
	SY_KEY_SPECIFIC_CASE(enter, Return, false);
	SY_KEY_SPECIFIC_CASE(escape, Escape, false);
	SY_KEY_SPECIFIC_CASE(arrow_up, Up, false);
	SY_KEY_SPECIFIC_CASE(arrow_down, Down, false);
	SY_KEY_SPECIFIC_CASE(arrow_left, Left, false);
	SY_KEY_SPECIFIC_CASE(arrow_right, Right, false);
	SY_KEY_SPECIFIC_CASE(f1, F1, false);
	SY_KEY_SPECIFIC_CASE(f2, F2, false);
	SY_KEY_SPECIFIC_CASE(f3, F3, false);
	SY_KEY_SPECIFIC_CASE(f4, F4, false);
	SY_KEY_SPECIFIC_CASE(f5, F5, false);
	SY_KEY_SPECIFIC_CASE(f6, F6, false);
	SY_KEY_SPECIFIC_CASE(f7, F7, false);
	SY_KEY_SPECIFIC_CASE(f8, F8, false);
	SY_KEY_SPECIFIC_CASE(f9, F9, false);
	SY_KEY_SPECIFIC_CASE(f10, F10, false);
	SY_KEY_SPECIFIC_CASE(f11, F11, false);
	SY_KEY_SPECIFIC_CASE(f12, F12, false);
	SY_KEY_CASE(semicolon, false);
	SY_KEY_SPECIFIC_CASE(bracket_left, bracketleft, false);
	SY_KEY_SPECIFIC_CASE(bracket_right, bracketright, false);
	SY_KEY_CASE(comma, false);
	SY_KEY_CASE(period, false);
	SY_KEY_SPECIFIC_CASE(forward_slash, slash, false);
	SY_KEY_SPECIFIC_CASE(back_slash, backslash, false)
	SY_KEY_CASE(minus, false);
	SY_KEY_CASE(equal, false);
	SY_KEY_CASE(apostrophe, false);
	SY_KEY_CASE(grave, false);
    }

}

void handle_event_key_press(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event)
{
    xcb_key_press_event_t *message = (xcb_key_press_event_t*)event;

    xkb_keysym_t keysym = xkb_state_key_get_one_sym(xcb_info->xkb_state, message->detail);
    switch (keysym)
    {
	SY_KEY_CASE(a, true);
	SY_KEY_CASE(b, true);
	SY_KEY_CASE(c, true);
	SY_KEY_CASE(d, true);
	SY_KEY_CASE(e, true);
	SY_KEY_CASE(f, true);
	SY_KEY_CASE(g, true);
	SY_KEY_CASE(h, true);
	SY_KEY_CASE(i, true);
	SY_KEY_CASE(j, true);
	SY_KEY_CASE(k, true);
	SY_KEY_CASE(l, true);
	SY_KEY_CASE(m, true);
	SY_KEY_CASE(n, true);
	SY_KEY_CASE(o, true);
	SY_KEY_CASE(p, true);
	SY_KEY_CASE(q, true);
	SY_KEY_CASE(r, true);
	SY_KEY_CASE(s, true);
	SY_KEY_CASE(t, true);
	SY_KEY_CASE(u, true);
	SY_KEY_CASE(v, true);
	SY_KEY_CASE(w, true);
	SY_KEY_CASE(x, true);
	SY_KEY_CASE(y, true);
	SY_KEY_CASE(z, true);
	SY_KEY_SPECIFIC_CASE(zero, 0, true);
	SY_KEY_SPECIFIC_CASE(one, 1, true);
	SY_KEY_SPECIFIC_CASE(two, 2, true);
	SY_KEY_SPECIFIC_CASE(three, 3, true);
	SY_KEY_SPECIFIC_CASE(four, 4, true);
	SY_KEY_SPECIFIC_CASE(five, 5, true);
	SY_KEY_SPECIFIC_CASE(six, 6, true);
	SY_KEY_SPECIFIC_CASE(seven, 7, true);
	SY_KEY_SPECIFIC_CASE(eight, 8, true);
	SY_KEY_SPECIFIC_CASE(nine, 9, true);
	SY_KEY_CASE(space, true);
	SY_KEY_SPECIFIC_CASE(shift_left, Shift_L, true);
	SY_KEY_SPECIFIC_CASE(shift_right, Shift_R, true);
	SY_KEY_SPECIFIC_CASE(control_left, Control_L, true);
	SY_KEY_SPECIFIC_CASE(control_right, Control_R, true);
	SY_KEY_SPECIFIC_CASE(alt_left, Alt_L, true);
	SY_KEY_SPECIFIC_CASE(alt_right, Alt_R, true);
	SY_KEY_SPECIFIC_CASE(tab, Tab, true);
	SY_KEY_SPECIFIC_CASE(caps_lock, Caps_Lock, true);
	SY_KEY_SPECIFIC_CASE(back_space, BackSpace, true);
	SY_KEY_SPECIFIC_CASE(enter, Return, true);
	SY_KEY_SPECIFIC_CASE(escape, Escape, true);
	SY_KEY_SPECIFIC_CASE(arrow_up, Up, true);
	SY_KEY_SPECIFIC_CASE(arrow_down, Down, true);
	SY_KEY_SPECIFIC_CASE(arrow_left, Left, true);
	SY_KEY_SPECIFIC_CASE(arrow_right, Right, true);
	SY_KEY_SPECIFIC_CASE(f1, F1, true);
	SY_KEY_SPECIFIC_CASE(f2, F2, true);
	SY_KEY_SPECIFIC_CASE(f3, F3, true);
	SY_KEY_SPECIFIC_CASE(f4, F4, true);
	SY_KEY_SPECIFIC_CASE(f5, F5, true);
	SY_KEY_SPECIFIC_CASE(f6, F6, true);
	SY_KEY_SPECIFIC_CASE(f7, F7, true);
	SY_KEY_SPECIFIC_CASE(f8, F8, true);
	SY_KEY_SPECIFIC_CASE(f9, F9, true);
	SY_KEY_SPECIFIC_CASE(f10, F10, true);
	SY_KEY_SPECIFIC_CASE(f11, F11, true);
	SY_KEY_SPECIFIC_CASE(f12, F12, true);
	SY_KEY_CASE(semicolon, true);
	SY_KEY_CASE(comma, true);
	SY_KEY_CASE(period, true);
	SY_KEY_SPECIFIC_CASE(forward_slash, slash, true);
	SY_KEY_SPECIFIC_CASE(back_slash, backslash, true)
	SY_KEY_SPECIFIC_CASE(bracket_left, bracketleft, true);
	SY_KEY_SPECIFIC_CASE(bracket_right, bracketright, true);
	SY_KEY_CASE(minus, true);
	SY_KEY_CASE(equal, true);
	SY_KEY_CASE(apostrophe, true);
	SY_KEY_CASE(grave, true);
    }
}

#undef SY_KEY_CASE
#undef RY_KEY_SPECIFIC_CASE

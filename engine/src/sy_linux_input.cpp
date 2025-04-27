#include "sy_linux_input.hpp"
#include "sy_linux_window.hpp"

#include <math.h>

#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#include <xcb/xproto.h>
#include <xcb/xinput.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

void handle_event_input_motion(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event);
void handle_event_client_message(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event);
void handle_event_expose(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event);
void handle_event_key_press(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event);
void handle_event_key_release(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event);

void set_to_unpressed(SyKeyState *key)
{
    if (*key == SyKeyState::released)
    {
	*key = SyKeyState::unpressed;
    }
}

void clear_input_info(SyInputInfo *input_info)
{
    memset(input_info->text_buffer, 0, input_info->text_buffer_size);
    input_info->a = SyKeyState::unpressed;
    input_info->b = SyKeyState::unpressed;
    input_info->c = SyKeyState::unpressed;
    input_info->d = SyKeyState::unpressed;
    input_info->e = SyKeyState::unpressed;
    input_info->f = SyKeyState::unpressed;
    input_info->g = SyKeyState::unpressed;
    input_info->h = SyKeyState::unpressed;
    input_info->i = SyKeyState::unpressed;
    input_info->j = SyKeyState::unpressed;
    input_info->k = SyKeyState::unpressed;
    input_info->l = SyKeyState::unpressed;
    input_info->m = SyKeyState::unpressed;
    input_info->n = SyKeyState::unpressed;
    input_info->o = SyKeyState::unpressed;
    input_info->p = SyKeyState::unpressed;
    input_info->q = SyKeyState::unpressed;
    input_info->r = SyKeyState::unpressed;
    input_info->s = SyKeyState::unpressed;
    input_info->t = SyKeyState::unpressed;
    input_info->u = SyKeyState::unpressed;
    input_info->v = SyKeyState::unpressed;
    input_info->w = SyKeyState::unpressed;
    input_info->x = SyKeyState::unpressed;
    input_info->y = SyKeyState::unpressed;
    input_info->z = SyKeyState::unpressed;
    input_info->zero = SyKeyState::unpressed;
    input_info->one = SyKeyState::unpressed;
    input_info->two = SyKeyState::unpressed;
    input_info->three = SyKeyState::unpressed;
    input_info->four = SyKeyState::unpressed;
    input_info->five = SyKeyState::unpressed;
    input_info->six = SyKeyState::unpressed;
    input_info->seven = SyKeyState::unpressed;
    input_info->eight = SyKeyState::unpressed;
    input_info->nine = SyKeyState::unpressed;
    input_info->space = SyKeyState::unpressed;
    input_info->shift_left = SyKeyState::unpressed;
    input_info->shift_right = SyKeyState::unpressed;
    input_info->control_left = SyKeyState::unpressed;
    input_info->control_right = SyKeyState::unpressed;
    input_info->alt_left = SyKeyState::unpressed;
    input_info->alt_right = SyKeyState::unpressed;
    input_info->tab = SyKeyState::unpressed;
    input_info->caps_lock = SyKeyState::unpressed;
    input_info->back_space = SyKeyState::unpressed;
    input_info->enter = SyKeyState::unpressed;
    input_info->escape = SyKeyState::unpressed;
    input_info->arrow_up = SyKeyState::unpressed;
    input_info->arrow_down = SyKeyState::unpressed;
    input_info->arrow_left = SyKeyState::unpressed;
    input_info->arrow_right = SyKeyState::unpressed;
    input_info->f1 = SyKeyState::unpressed;
    input_info->f2 = SyKeyState::unpressed;
    input_info->f3 = SyKeyState::unpressed;
    input_info->f4 = SyKeyState::unpressed;
    input_info->f5 = SyKeyState::unpressed;
    input_info->f6 = SyKeyState::unpressed;
    input_info->f7 = SyKeyState::unpressed;
    input_info->f8 = SyKeyState::unpressed;
    input_info->f9 = SyKeyState::unpressed;
    input_info->f10 = SyKeyState::unpressed;
    input_info->f11 = SyKeyState::unpressed;
    input_info->f12 = SyKeyState::unpressed;
    input_info->comma = SyKeyState::unpressed;
    input_info->period = SyKeyState::unpressed;
    input_info->forward_slash = SyKeyState::unpressed;
    input_info->semicolon = SyKeyState::unpressed;
    input_info->single_quote = SyKeyState::unpressed;
    input_info->bracket_left = SyKeyState::unpressed;
    input_info->bracket_right = SyKeyState::unpressed;
    input_info->back_slash = SyKeyState::unpressed;
    input_info->minus = SyKeyState::unpressed;
    input_info->equal = SyKeyState::unpressed;
    input_info->apostrophe = SyKeyState::unpressed;
    input_info->grave = SyKeyState::unpressed;
}

void poll_events(SyXCBInfo *xcb_info, SyInputInfo *input_info)
{
    memset(input_info->text_buffer, 0, sizeof(input_info->text_buffer[0]) * input_info->text_buffer_size);
    set_to_unpressed(&input_info->a);
    set_to_unpressed(&input_info->b);
    set_to_unpressed(&input_info->c);
    set_to_unpressed(&input_info->d);
    set_to_unpressed(&input_info->e);
    set_to_unpressed(&input_info->f);
    set_to_unpressed(&input_info->g);
    set_to_unpressed(&input_info->h);
    set_to_unpressed(&input_info->i);
    set_to_unpressed(&input_info->j);
    set_to_unpressed(&input_info->k);
    set_to_unpressed(&input_info->l);
    set_to_unpressed(&input_info->m);
    set_to_unpressed(&input_info->n);
    set_to_unpressed(&input_info->o);
    set_to_unpressed(&input_info->p);
    set_to_unpressed(&input_info->q);
    set_to_unpressed(&input_info->r);
    set_to_unpressed(&input_info->s);
    set_to_unpressed(&input_info->t);
    set_to_unpressed(&input_info->u);
    set_to_unpressed(&input_info->v);
    set_to_unpressed(&input_info->w);
    set_to_unpressed(&input_info->x);
    set_to_unpressed(&input_info->y);
    set_to_unpressed(&input_info->z);
    set_to_unpressed(&input_info->zero);
    set_to_unpressed(&input_info->one);
    set_to_unpressed(&input_info->two);
    set_to_unpressed(&input_info->three);
    set_to_unpressed(&input_info->four);
    set_to_unpressed(&input_info->five);
    set_to_unpressed(&input_info->six);
    set_to_unpressed(&input_info->seven);
    set_to_unpressed(&input_info->eight);
    set_to_unpressed(&input_info->nine);
    set_to_unpressed(&input_info->space);
    set_to_unpressed(&input_info->shift_left);
    set_to_unpressed(&input_info->shift_right);
    set_to_unpressed(&input_info->control_left);
    set_to_unpressed(&input_info->control_right);
    set_to_unpressed(&input_info->alt_left);
    set_to_unpressed(&input_info->alt_right);
    set_to_unpressed(&input_info->tab);
    set_to_unpressed(&input_info->caps_lock);
    set_to_unpressed(&input_info->back_space);
    set_to_unpressed(&input_info->enter);
    set_to_unpressed(&input_info->escape);
    set_to_unpressed(&input_info->arrow_up);
    set_to_unpressed(&input_info->arrow_down);
    set_to_unpressed(&input_info->arrow_left);
    set_to_unpressed(&input_info->arrow_right);
    set_to_unpressed(&input_info->f1);
    set_to_unpressed(&input_info->f2);
    set_to_unpressed(&input_info->f3);
    set_to_unpressed(&input_info->f4);
    set_to_unpressed(&input_info->f5);
    set_to_unpressed(&input_info->f6);
    set_to_unpressed(&input_info->f7);
    set_to_unpressed(&input_info->f8);
    set_to_unpressed(&input_info->f9);
    set_to_unpressed(&input_info->f10);
    set_to_unpressed(&input_info->f11);
    set_to_unpressed(&input_info->f12);
    set_to_unpressed(&input_info->comma);
    set_to_unpressed(&input_info->period);
    set_to_unpressed(&input_info->forward_slash);
    set_to_unpressed(&input_info->semicolon);
    set_to_unpressed(&input_info->single_quote);
    set_to_unpressed(&input_info->bracket_left);
    set_to_unpressed(&input_info->bracket_right);
    set_to_unpressed(&input_info->back_slash);
    set_to_unpressed(&input_info->minus);
    set_to_unpressed(&input_info->equal);
    set_to_unpressed(&input_info->apostrophe);
    set_to_unpressed(&input_info->grave);
    
    input_info->window_should_close = false;
    input_info->window_resized = false;

    input_info->mouse_dx = 0;
    input_info->mouse_dy = 0;

    xcb_generic_event_t *event;
    while ( (event = xcb_poll_for_event(xcb_info->conn)) )
    {
	
	if (event->response_type == 0)
	{
	    SY_ERROR_OUTPUT("XCB Error recieved");
	    free(event);
	    continue;
	}

	switch (XCB_EVENT_RESPONSE_TYPE(event))
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
		break;

	    case XCB_BUTTON_PRESS:
		break;

	    case XCB_BUTTON_RELEASE:
		break;

	    case XCB_GE_GENERIC:
	    {
		xcb_ge_generic_event_t *ge_event = (xcb_ge_generic_event_t*)event;
		if (ge_event->extension == xcb_info->xi_opcode)
		{
		    switch (ge_event->event_type)
		    {
			case XCB_INPUT_MOTION:

			    handle_event_input_motion(xcb_info, input_info, event);
			    break;
		    }
		}
		break;
	    }

	    default:
		SY_OUTPUT_DEBUG("xcb event loop: unknown event %d", XCB_EVENT_RESPONSE_TYPE(event));
		break;

	}

	free(event);
    }


    if (input_info->mouse_dx != 0 || input_info->mouse_dy != 0)
    {
	xcb_warp_pointer_checked(xcb_info->conn, XCB_NONE, xcb_info->win, 0, 0, 0, 0, input_info->window_width/2, input_info->window_height/2);
	xcb_flush(xcb_info->conn);

	// get the position through here to avoid small bits of drift due to imprecision casting float to int
	xcb_input_xi_query_pointer_cookie_t cookie = xcb_input_xi_query_pointer(xcb_info->conn, xcb_info->win, xcb_info->master_pointer_id);
	xcb_generic_error_t *err = NULL;
	xcb_input_xi_query_pointer_reply_t *reply = xcb_input_xi_query_pointer_reply(xcb_info->conn, cookie, &err);

	SY_ERROR_COND(err != NULL, "Failed to query pointer. error code %d major %d minor %d", err->error_code, err->major_code, err->minor_code);

	input_info->mouse_x = (float)reply->win_x / 65536.0;
	input_info->mouse_y = (float)reply->win_y / 65536.0;

	free(reply);

    }

}

void handle_event_input_motion(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event)
{
    xcb_input_motion_event_t *message = (xcb_input_motion_event_t*)event;
    
    float x_pos = (float)message->event_x / 65536.0;
    float y_pos = (float)message->event_y / 65536.0;

    input_info->mouse_dx += x_pos - input_info->mouse_x;
    input_info->mouse_dy += y_pos - input_info->mouse_y;

    input_info->mouse_x = x_pos;
    input_info->mouse_y = y_pos;
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
	input_info->window_width = reply->width;
	input_info->window_height = reply->height;
    }

    free(reply);
}

#define SY_KEY_SPECIFIC_CASE(key, xkb_ver, boolean)\
    case XKB_KEY_##xkb_ver:			   \
    {						   \
	input_info->key = boolean;		   \
	xkb_state_update_key(xcb_info->xkb_state, message->detail, (boolean == SyKeyState::released)? XKB_KEY_UP : XKB_KEY_DOWN); \
									\
	if (boolean == SyKeyState::pressed)				\
	{								\
	    char buf[5];						\
	    xkb_keysym_to_utf8(keysym, buf, 5); \
	    strncat(input_info->text_buffer, buf, input_info->text_buffer_size - strlen(input_info->text_buffer) - 1); \
									\
	}								\
	break;								\
    }

#define SY_KEY_CASE(key, boolean) SY_KEY_SPECIFIC_CASE(key, key, boolean);

void handle_event_key_release(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event)
{
    xcb_key_release_event_t *message = (xcb_key_release_event_t*)event;

    xkb_keysym_t keysym = xkb_state_key_get_one_sym(xcb_info->xkb_state, message->detail);
    switch (xkb_keysym_to_lower(keysym))
    {
	SY_KEY_CASE(a, SyKeyState::released);
	SY_KEY_CASE(b, SyKeyState::released);
	SY_KEY_CASE(c, SyKeyState::released);
	SY_KEY_CASE(d, SyKeyState::released);
	SY_KEY_CASE(e, SyKeyState::released);
	SY_KEY_CASE(f, SyKeyState::released);
	SY_KEY_CASE(g, SyKeyState::released);
	SY_KEY_CASE(h, SyKeyState::released);
	SY_KEY_CASE(i, SyKeyState::released);
	SY_KEY_CASE(j, SyKeyState::released);
	SY_KEY_CASE(k, SyKeyState::released);
	SY_KEY_CASE(l, SyKeyState::released);
	SY_KEY_CASE(m, SyKeyState::released);
	SY_KEY_CASE(n, SyKeyState::released);
	SY_KEY_CASE(o, SyKeyState::released);
	SY_KEY_CASE(p, SyKeyState::released);
	SY_KEY_CASE(q, SyKeyState::released);
	SY_KEY_CASE(r, SyKeyState::released);
	SY_KEY_CASE(s, SyKeyState::released);
	SY_KEY_CASE(t, SyKeyState::released);
	SY_KEY_CASE(u, SyKeyState::released);
	SY_KEY_CASE(v, SyKeyState::released);
	SY_KEY_CASE(w, SyKeyState::released);
	SY_KEY_CASE(x, SyKeyState::released);
	SY_KEY_CASE(y, SyKeyState::released);
	SY_KEY_CASE(z, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(zero, 0, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(one, 1, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(two, 2, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(three, 3, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(four, 4, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(five, 5, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(six, 6, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(seven, 7, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(eight, 8, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(nine, 9, SyKeyState::released);
	SY_KEY_CASE(space, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(shift_left, Shift_L, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(shift_right, Shift_R, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(control_left, Control_L, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(control_right, Control_R, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(alt_left, Alt_L, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(alt_right, Alt_R, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(tab, Tab, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(space, KP_Space, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(caps_lock, Caps_Lock, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(back_space, BackSpace, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(enter, Return, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(escape, Escape, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(arrow_up, Up, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(arrow_down, Down, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(arrow_left, Left, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(arrow_right, Right, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f1, F1, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f2, F2, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f3, F3, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f4, F4, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f5, F5, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f6, F6, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f7, F7, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f8, F8, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f9, F9, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f10, F10, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f11, F11, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(f12, F12, SyKeyState::released);
	SY_KEY_CASE(semicolon, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(bracket_left, bracketleft, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(bracket_right, bracketright, SyKeyState::released);
	SY_KEY_CASE(comma, SyKeyState::released);
	SY_KEY_CASE(period, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(forward_slash, slash, SyKeyState::released);
	SY_KEY_SPECIFIC_CASE(back_slash, backslash, SyKeyState::released)
	SY_KEY_CASE(minus, SyKeyState::released);
	SY_KEY_CASE(equal, SyKeyState::released);
	SY_KEY_CASE(apostrophe, SyKeyState::released);
	SY_KEY_CASE(grave, SyKeyState::released);
    }

}

void handle_event_key_press(SyXCBInfo *xcb_info, SyInputInfo *input_info, xcb_generic_event_t *event)
{
    xcb_key_press_event_t *message = (xcb_key_press_event_t*)event;

    xkb_keysym_t keysym = xkb_state_key_get_one_sym(xcb_info->xkb_state, message->detail);
    switch (xkb_keysym_to_lower(keysym))
    {
	SY_KEY_CASE(a, SyKeyState::pressed);
	SY_KEY_CASE(b, SyKeyState::pressed);
	SY_KEY_CASE(c, SyKeyState::pressed);
	SY_KEY_CASE(d, SyKeyState::pressed);
	SY_KEY_CASE(e, SyKeyState::pressed);
	SY_KEY_CASE(f, SyKeyState::pressed);
	SY_KEY_CASE(g, SyKeyState::pressed);
	SY_KEY_CASE(h, SyKeyState::pressed);
	SY_KEY_CASE(i, SyKeyState::pressed);
	SY_KEY_CASE(j, SyKeyState::pressed);
	SY_KEY_CASE(k, SyKeyState::pressed);
	SY_KEY_CASE(l, SyKeyState::pressed);
	SY_KEY_CASE(m, SyKeyState::pressed);
	SY_KEY_CASE(n, SyKeyState::pressed);
	SY_KEY_CASE(o, SyKeyState::pressed);
	SY_KEY_CASE(p, SyKeyState::pressed);
	SY_KEY_CASE(q, SyKeyState::pressed);
	SY_KEY_CASE(r, SyKeyState::pressed);
	SY_KEY_CASE(s, SyKeyState::pressed);
	SY_KEY_CASE(t, SyKeyState::pressed);
	SY_KEY_CASE(u, SyKeyState::pressed);
	SY_KEY_CASE(v, SyKeyState::pressed);
	SY_KEY_CASE(w, SyKeyState::pressed);
	SY_KEY_CASE(x, SyKeyState::pressed);
	SY_KEY_CASE(y, SyKeyState::pressed);
	SY_KEY_CASE(z, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(zero, 0, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(one, 1, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(two, 2, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(three, 3, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(four, 4, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(five, 5, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(six, 6, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(seven, 7, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(eight, 8, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(nine, 9, SyKeyState::pressed);
	SY_KEY_CASE(space, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(shift_left, Shift_L, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(shift_right, Shift_R, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(control_left, Control_L, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(control_right, Control_R, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(alt_left, Alt_L, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(alt_right, Alt_R, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(tab, Tab, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(caps_lock, Caps_Lock, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(back_space, BackSpace, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(enter, Return, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(escape, Escape, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(arrow_up, Up, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(arrow_down, Down, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(arrow_left, Left, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(arrow_right, Right, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f1, F1, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f2, F2, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f3, F3, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f4, F4, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f5, F5, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f6, F6, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f7, F7, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f8, F8, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f9, F9, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f10, F10, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f11, F11, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(f12, F12, SyKeyState::pressed);
	SY_KEY_CASE(semicolon, SyKeyState::pressed);
	SY_KEY_CASE(comma, SyKeyState::pressed);
	SY_KEY_CASE(period, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(forward_slash, slash, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(back_slash, backslash, SyKeyState::pressed)
	SY_KEY_SPECIFIC_CASE(bracket_left, bracketleft, SyKeyState::pressed);
	SY_KEY_SPECIFIC_CASE(bracket_right, bracketright, SyKeyState::pressed);
	SY_KEY_CASE(minus, SyKeyState::pressed);
	SY_KEY_CASE(equal, SyKeyState::pressed);
	SY_KEY_CASE(apostrophe, SyKeyState::pressed);
	SY_KEY_CASE(grave, SyKeyState::pressed);
    }
}

#undef SY_KEY_CASE
#undef RY_KEY_SPECIFIC_CASE

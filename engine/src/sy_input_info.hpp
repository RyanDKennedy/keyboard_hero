#pragma once

#include <stddef.h>

enum class SyKeyState
{
    unpressed,
    pressed,
    released,
};

struct SyInputInfo
{
    static const size_t text_buffer_size = 32;
    char text_buffer[text_buffer_size];
    float mouse_x;
    float mouse_y;
    float mouse_dx;
    float mouse_dy;
    int window_width;
    int window_height;
    bool window_should_close;
    bool window_resized;
    SyKeyState a;
    SyKeyState b;
    SyKeyState c;
    SyKeyState d;
    SyKeyState e;
    SyKeyState f;
    SyKeyState g;
    SyKeyState h;
    SyKeyState i;
    SyKeyState j;
    SyKeyState k;
    SyKeyState l;
    SyKeyState m;
    SyKeyState n;
    SyKeyState o;
    SyKeyState p;
    SyKeyState q;
    SyKeyState r;
    SyKeyState s;
    SyKeyState t;
    SyKeyState u;
    SyKeyState v;
    SyKeyState w;
    SyKeyState x;
    SyKeyState y;
    SyKeyState z;
    SyKeyState zero;
    SyKeyState one;
    SyKeyState two;
    SyKeyState three;
    SyKeyState four;
    SyKeyState five;
    SyKeyState six;
    SyKeyState seven;
    SyKeyState eight;
    SyKeyState nine;
    SyKeyState space;
    SyKeyState shift_left;
    SyKeyState shift_right;
    SyKeyState control_left;
    SyKeyState control_right;
    SyKeyState alt_left;
    SyKeyState alt_right;
    SyKeyState tab;
    SyKeyState caps_lock;
    SyKeyState back_space;
    SyKeyState enter;
    SyKeyState escape;
    SyKeyState arrow_up;
    SyKeyState arrow_down;
    SyKeyState arrow_left;
    SyKeyState arrow_right;
    SyKeyState f1;
    SyKeyState f2;
    SyKeyState f3;
    SyKeyState f4;
    SyKeyState f5;
    SyKeyState f6;
    SyKeyState f7;
    SyKeyState f8;
    SyKeyState f9;
    SyKeyState f10;
    SyKeyState f11;
    SyKeyState f12;
    SyKeyState comma;
    SyKeyState period;
    SyKeyState forward_slash;
    SyKeyState semicolon;
    SyKeyState single_quote;
    SyKeyState bracket_left;
    SyKeyState bracket_right;
    SyKeyState back_slash;
    SyKeyState minus;
    SyKeyState equal;
    SyKeyState apostrophe;
    SyKeyState grave;
};

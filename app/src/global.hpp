#pragma once

#include "sy_ecs.hpp"
#include "glm_include.hpp"

struct Global
{
    SyEntityHandle player;

    static const size_t buttons_amt = 3;
    SyEntityHandle buttons[buttons_amt];
    size_t selected_btn;

    float yaw;
    float pitch;
};

inline Global *g_state;

#pragma once

#include "sy_ecs.hpp"

struct Global
{
    SyEntityHandle player;
    SyEntityHandle entity_square;

    float yaw;
    float pitch;
};

inline Global *g_state;

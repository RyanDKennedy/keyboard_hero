#pragma once

#include "sy_ecs.hpp"

struct Global
{
    SyEntityHandle player;
    SyEntityHandle entity_square;
};

inline Global *g_state;

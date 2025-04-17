#pragma once

#include "sy_ecs.hpp"
#include "glm_include.hpp"

struct PositionAnimation
{
    glm::vec3 start;
    glm::vec3 destination;
    float duration;
    float time_done;
    size_t transform_index;
    bool running;
};

struct Global
{
    SyEntityHandle player;
    SyEntityHandle entity_square;

    float yaw;
    float pitch;

    PositionAnimation animation;
};

inline Global *g_state;

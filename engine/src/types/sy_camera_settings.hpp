#pragma once

#include "sy_ecs.hpp"

struct SyCameraSettings
{
    float fov;
    float near_plane;
    float far_plane;
    SyEntityHandle active_camera;
};

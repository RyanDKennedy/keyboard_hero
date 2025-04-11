#pragma once

#include "sy_ecs.hpp"

struct SyCameraSettings
{
    float fov;
    float aspect_ratio;
    float near_plane;
    float far_plane;
    SyEntityHandle active_camera;
};

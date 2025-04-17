#pragma once

#include "sy_ecs.hpp"
#include "sy_app_info.hpp"
#include "glm_include.hpp"

#include "global.hpp"

glm::vec3 make_rgb_from_255(float r, float g, float b);

void orthographic_movement(SyAppInfo *app_info, float camera_speed, float zoom_speed, float move_speed);
void perspective_movement(SyAppInfo *app_info, float camera_speed, float move_speed);

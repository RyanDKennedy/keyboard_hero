#pragma once

#include "sy_render_settings.hpp"
#include "sy_render_info.hpp"
#include "sy_pipeline.hpp"
#include "sy_input_info.hpp"
#include "sy_ecs.hpp"
#include "types/sy_camera_settings.hpp"

void sy_render_draw(SyRenderInfo *render_info, SyInputInfo *input_info, SyEcs *ecs, SyCameraSettings *camera_settings);



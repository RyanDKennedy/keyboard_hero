#pragma once

#include "sy_render_settings.hpp"

#include "sy_render_info.hpp"

void sy_render_create_swapchain(SyRenderInfo *render_info, int window_width, int window_height);

void sy_render_destroy_swapchain(SyRenderInfo *render_info);

#pragma once

#include "sy_render_info.hpp"


void sy_render_create_resources(SyRenderInfo *render_info, int win_width, int win_height);
void sy_render_destroy_resources(SyRenderInfo *render_info);
void sy_render_create_descriptor_set_layouts(SyRenderInfo *render_info);
VkRenderPass sy_render_create_simple_render_pass(SyRenderInfo *render_info);
void sy_render_create_swapchain_framebuffers(SyRenderInfo *render_info);

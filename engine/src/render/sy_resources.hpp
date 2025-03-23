#pragma once

#include "sy_render_info.hpp"

void sy_render_create_descriptor_set_layouts(SyRenderInfo *render_info);
VkRenderPass sy_render_create_simple_render_pass(SyRenderInfo *render_info);
void sy_render_create_swapchain_framebuffers(SyRenderInfo *render_info);
void sy_render_create_command_pool(SyRenderInfo *render_info);
void sy_render_create_command_buffers(SyRenderInfo *render_info);
void sy_render_create_sync_objects(SyRenderInfo *render_info);
size_t sy_render_create_descriptor_set(SyRenderInfo *render_info, size_t uniform_size, void *data, VkDescriptorSetLayout descriptor_layout, uint32_t layout_binding);

#pragma once

#include "sy_render_info.hpp"

SyRenderImage sy_render_create_texture_image(SyRenderInfo *render_info, void *data, VkExtent2D extent, VkFormat format, VkImageUsageFlags usage);
void sy_render_transition_image(SyRenderInfo *render_info, SyRenderImage *image, VkImageLayout old_layout, VkImageLayout new_layout);
SyRenderImage sy_render_create_image(SyRenderInfo *render_info, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags image_usage);
void sy_render_create_depth_resources(SyRenderInfo *render_info);
void sy_render_create_descriptor_set_layouts(SyRenderInfo *render_info);
VkRenderPass sy_render_create_simple_render_pass(SyRenderInfo *render_info);
void sy_render_create_framebuffers(SyRenderInfo *render_info);
void sy_render_create_command_pool(SyRenderInfo *render_info);
void sy_render_create_command_buffers(SyRenderInfo *render_info);
void sy_render_create_sync_objects(SyRenderInfo *render_info);
// size_t sy_render_create_descriptor_set(SyRenderInfo *render_info, size_t uniform_size, void *data, VkDescriptorSetLayout descriptor_layout, uint32_t layout_binding);
// void sy_render_create_descriptor_pool(SyRenderInfo *render_info);
void sy_render_create_pipelines(SyRenderInfo *render_info);
void sy_render_create_allocator(SyRenderInfo *render_info);

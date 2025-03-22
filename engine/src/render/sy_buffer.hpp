#pragma once

#include "sy_render_settings.hpp"

#include "sy_render_info.hpp"

void sy_render_copy_buffer(SyRenderInfo *render_info, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

void sy_render_create_vertex_buffer(SyRenderInfo *render_info, size_t vertex_amt, size_t vertex_size, void *vertices, VkBuffer *out_buffer, VmaAllocation *out_buffer_alloc);

void sy_render_create_index_buffer(SyRenderInfo *render_info, size_t index_amt, uint32_t *indices, VkBuffer *out_buffer, VmaAllocation *out_buffer_alloc);

#pragma once

#include "sy_render_settings.hpp"

#include "sy_render_info.hpp"

void sy_create_buffer(SyRenderInfo *render_info, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *buffer_memory);



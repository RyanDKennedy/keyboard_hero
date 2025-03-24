#pragma once

#include <stdint.h>

#include <vulkan/vulkan_core.h>
#include "vk_mem_alloc.h"

#include "sy_render_defines.hpp"

struct SyDescriptorSetDataGroup
{
    VkDescriptorSet descriptor_set[SY_RENDER_MAX_FRAMES_IN_FLIGHT];
    
    size_t buffer_size[SY_RENDER_MAX_FRAMES_IN_FLIGHT];
    VkBuffer uniform_buffer[SY_RENDER_MAX_FRAMES_IN_FLIGHT];
    VmaAllocation uniform_buffer_allocation[SY_RENDER_MAX_FRAMES_IN_FLIGHT];
};

#pragma once

#include <stdint.h>

#include <vulkan/vulkan_core.h>
#include "vk_mem_alloc.h"

struct SyDescriptorSetDataGroup
{
    VkDescriptorSet descriptor_set[2];
    
    size_t buffer_size[2];
    VkBuffer uniform_buffer[2];
    VmaAllocation uniform_buffer_allocation[2];
};

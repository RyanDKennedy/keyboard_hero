#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

struct SyMesh
{
    VkBuffer vertex_buffer;
    VmaAllocation vertex_buffer_alloc;

    VkBuffer index_buffer;
    VmaAllocation index_buffer_alloc;
    uint32_t index_amt;
};


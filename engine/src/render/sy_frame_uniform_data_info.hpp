#pragma once

#include <vector>
#include "vk_mem_alloc.h"
#include "render/sy_descriptor_allocator.hpp"

struct SyUniformAllocation
{
    VmaAllocation allocation;
    VkBuffer buffer;
};

struct SyFrameUniformDataInfo
{
    SyDescriptorAllocator descriptor_allocator;
    std::vector<SyUniformAllocation> allocations;
};

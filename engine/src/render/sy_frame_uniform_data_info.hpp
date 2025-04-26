#pragma once

#include <vector>
#include "vk_mem_alloc.h"
#include "render/sy_descriptor_allocator.hpp"

struct SyBufferAllocation
{
    VmaAllocation allocation;
    VkBuffer buffer;
};

struct SyFrameUniformDataInfo
{
    SyDescriptorAllocator descriptor_allocator;
    std::vector<SyBufferAllocation> allocations;
    
};

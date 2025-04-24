#pragma once

#include <vulkan/vulkan_core.h>
#include "vk_mem_alloc.h"

struct SyRenderImage
{
    VmaAllocation alloc;
    VkImage image;
    VkImageView image_view;
};

void sy_render_destroy_render_image(VkDevice logical_device, VmaAllocator vma_allocator, SyRenderImage *render_image);

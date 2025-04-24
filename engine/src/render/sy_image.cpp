#include "sy_image.hpp"

void sy_render_destroy_render_image(VkDevice logical_device, VmaAllocator vma_allocator, SyRenderImage *render_image)
{
    vkDestroyImageView(logical_device, render_image->image_view, NULL);
    vmaDestroyImage(vma_allocator, render_image->image, render_image->alloc);
}

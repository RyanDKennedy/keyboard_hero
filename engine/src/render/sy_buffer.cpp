#include "sy_buffer.hpp"
#include "render/sy_render_info.hpp"

#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);

void sy_render_create_vertex_buffer(SyRenderInfo *render_info, size_t vertices_size, uint8_t *vertices, VkBuffer *out_buffer, VmaAllocation *out_buffer_alloc)
{
    VkBuffer result;
    VmaAllocation result_alloc;

    VkDeviceSize buffer_size = vertices_size;

    // Create Staging Buffer
    VkBuffer staging_buffer;
    VmaAllocation staging_buffer_alloc;
    {
	VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	buffer_info.size = buffer_size;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	
	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
	alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	
	vmaCreateBuffer(render_info->vma_allocator, &buffer_info, &alloc_info, &staging_buffer, &staging_buffer_alloc, nullptr);
    }

    // Copy the vertices into the staging buffer
    vmaCopyMemoryToAllocation(render_info->vma_allocator, vertices, staging_buffer_alloc, 0, buffer_size);

    // Create Vertex Buffer
    {
	VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	buffer_info.size = buffer_size;
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	
	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	
	vmaCreateBuffer(render_info->vma_allocator, &buffer_info, &alloc_info, &result, &result_alloc, nullptr);
    }

    // Copy Staging Buffer to Vertex Buffer
    sy_render_copy_buffer(render_info, staging_buffer, result, buffer_size);

    // Destroy Staging Buffer
    vmaDestroyBuffer(render_info->vma_allocator, staging_buffer, staging_buffer_alloc);

    *out_buffer = result;
    *out_buffer_alloc = result_alloc;
}

void sy_render_create_index_buffer(SyRenderInfo *render_info, size_t index_amt, uint32_t *indices, VkBuffer *out_buffer, VmaAllocation *out_buffer_alloc)
{
    VkDeviceSize buffer_size = index_amt * sizeof(uint32_t);
    
    // Create Staging Buffer
    VkBuffer staging_buffer;
    VmaAllocation staging_buffer_alloc;
    {
	VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	buffer_info.size = buffer_size;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	
	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
	alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	
	vmaCreateBuffer(render_info->vma_allocator, &buffer_info, &alloc_info, &staging_buffer, &staging_buffer_alloc, nullptr);
    }

    // Copy data into Staging Buffer
    vmaCopyMemoryToAllocation(render_info->vma_allocator, indices, staging_buffer_alloc, 0, buffer_size);

    VkBuffer result;
    VmaAllocation result_alloc;

    // Create Index Buffer
    {
	VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	buffer_info.size = buffer_size;
	buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
	
	vmaCreateBuffer(render_info->vma_allocator, &buffer_info, &alloc_info, &result, &result_alloc, nullptr);
    }

    
    // Copy from Staging Buffer to Index Buffer
    sy_render_copy_buffer(render_info, staging_buffer, result, buffer_size);

    // Destroy Staging Buffer
    vmaDestroyBuffer(render_info->vma_allocator, staging_buffer, staging_buffer_alloc);

    *out_buffer = result;
    *out_buffer_alloc = result_alloc;
}

void sy_render_copy_buffer(SyRenderInfo *render_info, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
{
    // Create Command Buffer

    VkCommandBufferAllocateInfo command_buffer_allocate_info;
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.pNext = NULL;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;
    command_buffer_allocate_info.commandPool = render_info->command_pool;

    VkCommandBuffer command_buffer;
    SY_ERROR_COND(vkAllocateCommandBuffers(render_info->logical_device, &command_buffer_allocate_info, &command_buffer) != VK_SUCCESS, "RENDER: Failed to allocate command buffer for buffer copy.");


    // Record Command Buffer

    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = NULL;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = NULL;

    SY_ERROR_COND(vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS, "RENDER: Failed to start command buffer for buffer copy.");

    VkBufferCopy copy_region;
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);
    
    SY_ERROR_COND(vkEndCommandBuffer(command_buffer) != VK_SUCCESS, "RENDER: Failed to end command buffer for buffer copy.");

    // Submit Buffer

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    SY_ERROR_COND(vkQueueSubmit(render_info->graphics_queue, 1, &submit_info, NULL) != VK_SUCCESS, "RENDER: Failed to submit command buffer for buffer copy.");

    vkQueueWaitIdle(render_info->graphics_queue);

    // Cleanup

    vkFreeCommandBuffers(render_info->logical_device, render_info->command_pool, 1, &command_buffer);
}

uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    // Find the index 
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
    {
	if (type_filter & (1 << i) &&
	    (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
	{
	    return i;
	}
    }
    SY_ERROR("RENDER: Failed to find a correct memory type.");
}

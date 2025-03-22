#include "sy_buffer.hpp"

#include <stdlib.h>
#include <string.h>

uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);

void sy_render_create_vertex_buffer(SyRenderInfo *render_info, size_t vertex_amt, size_t vertex_size, void *vertices, VkBuffer *out_buffer, VkDeviceMemory *out_buffer_memory)
{
    VkBuffer result;
    VkDeviceMemory result_memory;

    VkDeviceSize buffer_size = vertex_amt * vertex_size;

    // Create Staging Buffer
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    sy_render_create_buffer(render_info, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

    // Copy the vertices into the staging buffer
    void *data;
    SY_ERROR_COND(vkMapMemory(render_info->logical_device, staging_buffer_memory, 0, buffer_size, 0, &data) != VK_SUCCESS, "RENDER: Failed to map vertex buffer memory.");
    memcpy(data, vertices, (size_t)buffer_size);
    vkUnmapMemory(render_info->logical_device, staging_buffer_memory);

    // Create Vertex Buffer
    sy_render_create_buffer(render_info, buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &result, &result_memory);

    // Copy Staging Buffer to Vertex Buffer
    sy_render_copy_buffer(render_info, staging_buffer, result, buffer_size);

    // Destroy Staging Buffer
    vkDestroyBuffer(render_info->logical_device, staging_buffer, NULL);
    vkFreeMemory(render_info->logical_device, staging_buffer_memory, NULL);

    *out_buffer = result;
    *out_buffer_memory = result_memory;
}

void sy_render_create_index_buffer(SyRenderInfo *render_info, size_t index_amt, uint32_t *indices, VkBuffer *out_buffer, VkDeviceMemory *out_buffer_memory)
{
    VkDeviceSize buffer_size = index_amt * sizeof(uint32_t);
    
    // Create Staging Buffer
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    sy_render_create_buffer(render_info, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer, &staging_buffer_memory);

    // Copy data into Staging Buffer
    void *data;
    SY_ERROR_COND(vkMapMemory(render_info->logical_device, staging_buffer_memory, 0, buffer_size, 0, &data) != VK_SUCCESS, "RENDER: Failed to map vertex buffer memory.");
    memcpy(data, indices, (size_t)buffer_size);
    vkUnmapMemory(render_info->logical_device, staging_buffer_memory);

    VkBuffer result;
    VkDeviceMemory result_memory;

    // Create Index Buffer
    sy_render_create_buffer(render_info, buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &result, &result_memory);
    
    // Copy from Staging Buffer to Index Buffer
    sy_render_copy_buffer(render_info, staging_buffer, result, buffer_size);

    // Destroy Staging Buffer
    vkDestroyBuffer(render_info->logical_device, staging_buffer, NULL);
    vkFreeMemory(render_info->logical_device, staging_buffer_memory, NULL);

    *out_buffer = result;
    *out_buffer_memory = result_memory;
}


void sy_render_create_buffer(SyRenderInfo *render_info, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *buffer_memory)
{
    VkBufferCreateInfo buffer_create_info;
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = NULL;
    buffer_create_info.flags = 0;
    buffer_create_info.size = size;
    buffer_create_info.usage = usage;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create_info.queueFamilyIndexCount = 0;
    buffer_create_info.pQueueFamilyIndices = NULL; // don't have to do this because of sharing mode

    SY_ERROR_COND(vkCreateBuffer(render_info->logical_device, &buffer_create_info, NULL, buffer) != VK_SUCCESS, "RENDER: Failed to create buffer.");

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(render_info->logical_device, *buffer, &memory_requirements);

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = find_memory_type(render_info->physical_device, memory_requirements.memoryTypeBits, properties);

    SY_ERROR_COND(vkAllocateMemory(render_info->logical_device, &allocate_info, NULL, buffer_memory) != VK_SUCCESS, "RENDER: Failed to allocate memory for buffer.");

    SY_ERROR_COND(vkBindBufferMemory(render_info->logical_device, *buffer, *buffer_memory, 0) != VK_SUCCESS, "RENDER: Failed to bind buffer memory to buffer");



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

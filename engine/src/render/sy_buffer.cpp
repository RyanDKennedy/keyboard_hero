#include "sy_buffer.hpp"

#include <stdlib.h>

uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);

void sy_create_buffer(SyRenderInfo *render_info, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer *buffer, VkDeviceMemory *buffer_memory)
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

    SY_ERROR_COND(vkCreateBuffer(render_info->logical_device, &buffer_create_info, NULL, buffer) != VK_SUCCESS,
		  "PIPELINE: Failed to create buffer.");

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(render_info->logical_device, *buffer, &memory_requirements);

    VkMemoryAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = find_memory_type(render_info->physical_device, memory_requirements.memoryTypeBits, properties);

    SY_ERROR_COND(vkAllocateMemory(render_info->logical_device, &allocate_info, NULL, buffer_memory) != VK_SUCCESS, "PIPELINE: Failed to allocate memory for buffer.");

    SY_ERROR_COND(vkBindBufferMemory(render_info->logical_device, *buffer, *buffer_memory, 0) != VK_SUCCESS, "PIPELINE: Failed to bind buffer memory to buffer");



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

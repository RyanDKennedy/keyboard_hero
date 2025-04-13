#include "sy_logical_device.hpp"

#include <stdlib.h>
#include <vulkan/vulkan_core.h>

void sy_render_create_logical_device(SyRenderInfo *render_info)
{
    VkResult ok = VK_SUCCESS;

    // Fill queue_create_infos 

    /* NOTE: There is a possibility that the graphics_family and the present_family are the same and in that case we only want to
     *       attempt to create one family instead of two, so we must create a collection of unique queue families.
     */
    
    const uint32_t amt_of_unique_queue_families = (render_info->graphics_queue_family_index == render_info->present_queue_family_index)? 1 : 2;
    VkDeviceQueueCreateInfo *queue_create_infos = (VkDeviceQueueCreateInfo*)calloc(amt_of_unique_queue_families, sizeof(VkDeviceQueueCreateInfo));
    uint32_t queue_families[] = {render_info->graphics_queue_family_index, render_info->present_queue_family_index};
    
    float queue_priority[] = {1.0f};
    for (size_t i = 0; i < amt_of_unique_queue_families; ++i)
    {
	queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_infos[i].flags = 0;
	queue_create_infos[i].pNext = NULL;
	queue_create_infos[i].queueFamilyIndex = queue_families[i];
	queue_create_infos[i].queueCount = 1;
	queue_create_infos[i].pQueuePriorities = queue_priority;
    }

    // Create VkDeviceCreateInfo

    VkPhysicalDeviceFeatures device_features = {0};

    VkDeviceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.queueCreateInfoCount = amt_of_unique_queue_families;
    create_info.pQueueCreateInfos = queue_create_infos;
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = sy_g_render_vulkan_device_extension_amt;
    create_info.ppEnabledExtensionNames = sy_g_render_vulkan_device_extensions;
    
    if (sy_g_render_use_validation_layers)
    { // This is for backwards compatibility
	create_info.enabledLayerCount = sy_g_render_vulkan_layer_amt;
	create_info.ppEnabledLayerNames = sy_g_render_validation_layers;
    }
    else
    { // This is the modern way as there are no longer device layers.
	create_info.enabledLayerCount = 0;
	create_info.ppEnabledLayerNames = NULL;
    }

    SY_ERROR_COND(vkCreateDevice(render_info->physical_device, &create_info, NULL, &render_info->logical_device) != VK_SUCCESS, "RENDER: Failed to create logical device.");

    free(queue_create_infos);

    vkGetDeviceQueue(render_info->logical_device, render_info->graphics_queue_family_index, 0, &render_info->graphics_queue);
    vkGetDeviceQueue(render_info->logical_device, render_info->present_queue_family_index, 0, &render_info->present_queue);
}

#include "sy_physical_device.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool is_device_suitable(VkSurfaceKHR surface, VkPhysicalDevice device, uint32_t *out_graphics_family, uint32_t *out_present_family);

void sy_render_create_physical_device(SyRenderInfo *render_info)
{
    VkResult ok = VK_SUCCESS;
    
    uint32_t device_count = 0;
    ok = vkEnumeratePhysicalDevices(render_info->instance, &device_count, NULL);
    SY_ERROR_COND(ok != VK_SUCCESS, "RENDER: vkEnumeratePhysicalDevices(...) failed.");
    SY_ERROR_COND(device_count == 0, "RENDER: There are no physical devices recognized by vulkan.");

    VkPhysicalDevice *devices = (VkPhysicalDevice*)calloc(device_count, sizeof(VkPhysicalDevice));
    ok = vkEnumeratePhysicalDevices(render_info->instance, &device_count, devices);
    SY_ERROR_COND(ok != VK_SUCCESS, "RENDER: vkEnumeratePhysicalDevices(...) failed.");

    render_info->physical_device = VK_NULL_HANDLE;
    uint32_t present_family_index = -1;
    uint32_t graphics_family_index = -1;

    for (int i = 0; i < device_count; ++i)
    {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(devices[i], &props);
	SY_OUTPUT_INFO("POSSIBLE device %s", props.deviceName);
    }

    // Choose device
    for (int i = 0; i < device_count; ++i)
    {
	if (is_device_suitable(render_info->surface, devices[i], &graphics_family_index, &present_family_index))
	{
	    render_info->physical_device = devices[i];
	    break;
	}
    }

    free(devices);

    SY_ERROR_COND(render_info->physical_device == VK_NULL_HANDLE, "RENDER: Failed to find a suitable GPU.");
    render_info->graphics_queue_family_index = graphics_family_index;
    render_info->present_queue_family_index = present_family_index;
}

/* Checks for
   1. Has graphics and present queue famlies.
   2. Device supports the device swapchain extension
   3. The swap chain is adequate. (Has a present mode and has a color space.)
 */
bool is_device_suitable(VkSurfaceKHR surface, VkPhysicalDevice device, uint32_t *out_graphics_family, uint32_t *out_present_family)
{
    VkResult ok = VK_SUCCESS;

    bool graphics_family_exists = false;
    bool present_family_exists = false;
    uint32_t graphics_family;
    uint32_t present_family;

    { // Get queue family indices
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

	VkQueueFamilyProperties *queue_family_properties = (VkQueueFamilyProperties*)calloc(queue_family_count, sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_properties);

	for (int i = 0; i < queue_family_count; ++i)
	{
	    VkBool32 present_support = false;
	    ok = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
	    SY_ERROR_COND(ok != VK_SUCCESS, "RENDER: vkGetPhysicalDeviceSurfaceSupportKHR(...) failed.");	    	    

	    if (present_support)
	    {
		present_family_exists = true;
		present_family = i;
	    }
	    
	    if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
	    {
		graphics_family_exists = true;
		graphics_family = i;
	    }
	}

	free(queue_family_properties);
    }

    if (graphics_family_exists && present_family_exists == false)
    {
	return false;
    }

    bool extensions_supported = true;
    
    { // Checks if the devices extensions are supported
	uint32_t extension_count = 0;
	ok = vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
	SY_ERROR_COND(ok != VK_SUCCESS, "RENDER: vkEnumerateDeviceExtensionProperties(...) failed.");

	VkExtensionProperties *extension_properties = (VkExtensionProperties*)calloc(extension_count, sizeof(VkExtensionProperties));
	ok = vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, extension_properties);
	SY_ERROR_COND(ok != VK_SUCCESS, "RENDER: vkGetPhysicalDeviceQueueFamilyProperties(...) failed.");

	// make sure that each of the required extensions is listed in extension_properties.

	for (int i = 0; i < sy_g_render_vulkan_device_extension_amt; ++i)
	{ // For each required extension go through the entire list of available extensions
	    bool this_extension_supported = false;

	    for (int j = 0; j < extension_count; ++j)
	    { // Go until match or none left
		if (strcmp(sy_g_render_vulkan_device_extensions[i], extension_properties[j].extensionName) == 0)
		{
		    this_extension_supported = true;
		    break;
		}
	    }

	    // Check if the prior for loop found a match or not
	    if (this_extension_supported == false)
	    { 
		extensions_supported = false;
		break;
	    }
	}

	free(extension_properties);
    }

    if (extensions_supported == false)
    {
	return false;
    }

    bool swap_chain_adequate = false;

    { // Checks if the swap chain is adequate
	VkSurfaceCapabilitiesKHR capabilities;
	uint32_t format_amt = 0;
	VkSurfaceFormatKHR *formats;
	uint32_t present_mode_amt = 0;
	VkPresentModeKHR *present_modes;

	// Getting Capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

	// Gettings Formats
	ok = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_amt, NULL);
	SY_ERROR_COND(ok != VK_SUCCESS, "RENDER: vkGetPhysicalDeviceSurfaceFormatsKHR(...) failed.");
	if (format_amt != 0)
	{
	    formats = (VkSurfaceFormatKHR*)calloc(format_amt, sizeof(VkSurfaceFormatKHR));
	    ok = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_amt, formats);
	    SY_ERROR_COND(ok != VK_SUCCESS, "RENDER: vkGetPhysicalDeviceSurfaceFormatsKHR(...) failed.");	    
	    free(formats);
	}

	// Getting Present Modes
	ok = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_amt, NULL);
	SY_ERROR_COND(ok != VK_SUCCESS, "RENDER: vkGetPhysicalDeviceSurfacePresentModesKHR(...) failed.");	
	if (present_mode_amt != 0)
	{
	    present_modes = (VkPresentModeKHR*)calloc(present_mode_amt, sizeof(VkPresentModeKHR));
	    ok = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_amt, present_modes);
	    SY_ERROR_COND(ok != VK_SUCCESS, "RENDER: vkGetPhysicalDeviceSurfacePresentModesKHR(...) failed.");	
	    free(present_modes);
	}

	// Determining if it is adequate based on the data gathered
	swap_chain_adequate = (format_amt != 0) && (present_mode_amt != 0);

    }

    // This is only added because in the future we always need to find the queue family indices, so instead
    // I just store them instead of having to constantly recalculate them
    *out_graphics_family = graphics_family;
    *out_present_family = present_family;

    // equivalent to "return swap_chain_adequate && extensions_supported && graphics_family_exists && present_family_exists;"
    return swap_chain_adequate;
}


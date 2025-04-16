#include "sy_swapchain.hpp"
#include "render/sy_resources.hpp"

#include <stdlib.h>

void sy_render_create_swapchain(SyRenderInfo *render_info, int window_width, int window_height)
{
    VkResult ok = VK_SUCCESS;
    
    // Getting/Choosing swap chain extent
    VkExtent2D extent;
    VkSurfaceCapabilitiesKHR capabilities;
    {
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(render_info->physical_device, render_info->surface, &capabilities);

	if (capabilities.currentExtent.width != __UINT32_MAX__)
	{
	    extent = capabilities.currentExtent;
	}
	else
	{
	    extent.height = window_width;
	    extent.width = window_height;

	    if (extent.width > capabilities.maxImageExtent.width)
		extent.width = capabilities.maxImageExtent.width;

	    if (extent.width < capabilities.minImageExtent.width)
		extent.width = capabilities.minImageExtent.width;

	    if (extent.height > capabilities.maxImageExtent.height)
		extent.height = capabilities.maxImageExtent.height;

	    if (extent.height < capabilities.minImageExtent.height)
		extent.height = capabilities.minImageExtent.height;
	}
    }

    /* NOTE: When we got the formats and present modes in the physical device creation stage we checked if there were any
     *       formats and present modes, so we don't need to check here because if it passed the physical device creation stage
     *       then it must have available formats and present modes.
     */

    VkSurfaceFormatKHR surface_format;
    { // Gettings/Choosing surface format

	// Filling format_amt and formats
	uint32_t format_amt;
	VkSurfaceFormatKHR *formats;
	ok = vkGetPhysicalDeviceSurfaceFormatsKHR(render_info->physical_device, render_info->surface, &format_amt, NULL);
	SY_ERROR_COND(ok != VK_SUCCESS, "RENDERER: vkGetPhysicalDeviceSurfaceFormatsKHR(...) failed.");
	formats = (VkSurfaceFormatKHR*)calloc(format_amt, sizeof(VkSurfaceFormatKHR));
	ok = vkGetPhysicalDeviceSurfaceFormatsKHR(render_info->physical_device, render_info->surface, &format_amt, formats);
	SY_ERROR_COND(ok != VK_SUCCESS, "RENDERER: vkGetPhysicalDeviceSurfaceFormatsKHR(...) failed.");	    
	
	// Assigning a value from the formats we just got
	surface_format = formats[0]; // Default value
	for (uint32_t i = 0; i < format_amt; ++i)
	{
	    // Ideal value
	    if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
		formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
	    {
		surface_format = formats[i];
		break;
	    }
	}
	
	free(formats);
    }
    
    VkPresentModeKHR present_mode;
    { // Getting/Choosing present mode

	// Filling present_mode_amt and present_modes
	uint32_t present_mode_amt;
	VkPresentModeKHR *present_modes;
	ok = vkGetPhysicalDeviceSurfacePresentModesKHR(render_info->physical_device, render_info->surface, &present_mode_amt, NULL);
	SY_ERROR_COND(ok != VK_SUCCESS, "RENDERER: vkGetPhysicalDeviceSurfacePresentModesKHR(...) failed.");	
	present_modes = (VkPresentModeKHR*)calloc(present_mode_amt, sizeof(VkPresentModeKHR));
	ok = vkGetPhysicalDeviceSurfacePresentModesKHR(render_info->physical_device, render_info->surface, &present_mode_amt, present_modes);
	SY_ERROR_COND(ok != VK_SUCCESS, "RENDERER: vkGetPhysicalDeviceSurfacePresentModesKHR(...) failed.");	
	
	/* Modes
	 * + FIFO_KHR - has a FIFO queue of submitted images and gets one each refresh
	 * + MAILBOX_KHR - like FIFO except that the already queued images are replaced by newer ones (preferred)
	 */

	// Assigning a value from the present modes we just got
	present_mode = VK_PRESENT_MODE_FIFO_KHR; // default value
	for (uint32_t i = 0; i < present_mode_amt;  ++i)
	{
	    // Ideal value
	    if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
	    {
		present_mode = present_modes[i];
		break;
	    }

	}
	
	free(present_modes);
    }

    uint32_t image_count = capabilities.minImageCount + 1;
    // if maxImageCount == 0 then there is no max, as many images as you want
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
    {
	image_count = capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.presentMode = present_mode;
    create_info.surface = render_info->surface;
    create_info.imageArrayLayers = 1; // only 1 unless you are doing stereoscopic rendering (movie theater 3D effect)
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    uint32_t queue_family_indices[] = {render_info->graphics_queue_family_index, render_info->present_queue_family_index};
    if (render_info->graphics_queue_family_index != render_info->present_queue_family_index)
    {
	create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	create_info.queueFamilyIndexCount = 2;
	create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 0; // this field is only required if image sharing mode is concurrent
	create_info.pQueueFamilyIndices = NULL;
    }

    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.clipped = VK_TRUE; // don't render fragments that are obscured by other windows
    create_info.oldSwapchain = VK_NULL_HANDLE;

    SY_ERROR_COND(vkCreateSwapchainKHR(render_info->logical_device, &create_info, NULL, &render_info->swapchain) != VK_SUCCESS,
		  "RENDER: Failed to create a swap chain.");

    // Get the images
    ok = vkGetSwapchainImagesKHR(render_info->logical_device, render_info->swapchain, &render_info->swapchain_images_amt, NULL);
    SY_ERROR_COND(ok != VK_SUCCESS, "RENDER: vkGetSwapchainImagesKHR(...) failed");

    render_info->swapchain_images = (VkImage*)calloc(render_info->swapchain_images_amt, sizeof(VkImage)); // This will be freed inside cleanup inside main.c
    ok = vkGetSwapchainImagesKHR(render_info->logical_device, render_info->swapchain, &render_info->swapchain_images_amt, render_info->swapchain_images);
    SY_ERROR_COND(ok != VK_SUCCESS, "RENDER: vkGetSwapchainImagesKHR(...) failed");

    // We will need this data later
    render_info->swapchain_image_extent = extent;
    render_info->swapchain_image_format = surface_format.format;


    // Create image views
    render_info->swapchain_image_views_amt = render_info->swapchain_images_amt;
    render_info->swapchain_image_views = (VkImageView*)calloc(render_info->swapchain_image_views_amt, sizeof(VkImageView));

    for (uint32_t i = 0; i < render_info->swapchain_image_views_amt; ++i)
    {
	VkImageViewCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.image = render_info->swapchain_images[i];
	create_info.format = render_info->swapchain_image_format;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	create_info.subresourceRange.baseMipLevel = 0;
	create_info.subresourceRange.levelCount = 1;
	create_info.subresourceRange.baseArrayLayer = 0;
	create_info.subresourceRange.layerCount = 1;
	
	SY_ERROR_COND(vkCreateImageView(render_info->logical_device, &create_info, NULL, &render_info->swapchain_image_views[i]) != VK_SUCCESS, "RENDER: Failed to create image view.");
    }

    sy_render_create_depth_resources(render_info);
}

void sy_render_destroy_swapchain(SyRenderInfo *render_info)
{

    for (size_t i = 0; i < render_info->swapchain_image_views_amt; ++i)
    {
	vkDestroyImageView(render_info->logical_device, render_info->swapchain_image_views[i], NULL);

	vmaDestroyImage(render_info->vma_allocator, render_info->depth_images[i], render_info->depth_image_allocations[i]);
	vkDestroyImageView(render_info->logical_device, render_info->depth_image_views[i], NULL);
    }

    free(render_info->swapchain_image_views);
    free(render_info->swapchain_images);

    free(render_info->depth_images);
    free(render_info->depth_image_views);
    free(render_info->depth_image_allocations);

    vkDestroySwapchainKHR(render_info->logical_device, render_info->swapchain, NULL);
}

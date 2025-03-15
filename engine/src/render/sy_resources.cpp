#include "sy_resources.hpp"

#include "sy_physical_device.hpp"
#include "sy_logical_device.hpp"
#include "sy_swapchain.hpp"

void sy_render_create_resources(SyRenderInfo *render_info, int win_width, int win_height)
{
    sy_render_create_physical_device(render_info);
    {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(render_info->physical_device, &props);
	SY_OUTPUT_INFO("using device %s", props.deviceName);
    }

    sy_render_create_logical_device(render_info);

    sy_render_create_swapchain(render_info, win_width, win_height);

}

void sy_render_destroy_resources(SyRenderInfo *render_info)
{
    sy_render_destroy_swapchain(render_info);
    vkDestroyDevice(render_info->logical_device, NULL);    
}

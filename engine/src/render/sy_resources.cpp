#include "sy_resources.hpp"

#include "sy_physical_device.hpp"
#include "sy_logical_device.hpp"
#include "sy_swapchain.hpp"

/*
p   create_render_pass(vk_info);
p   create_descriptor_set_layout(vk_info);
p   create_pipeline(vk_info);
p   create_framebuffers(vk_info);
r   create_command_pool(vk_info);
r   create_vertex_buffer(vk_info);
r   create_index_buffer(vk_info);
p   create_uniform_buffers(vk_info);
p   create_descriptor_pool(vk_info);
p   create_descriptor_sets(vk_info);
r   create_command_buffers(vk_info);
r   create_sync_objects(vk_info);

PIPELINE

renderpass                  by engine
sync objects                by engine
descriptor set/pool/layout  by engine
framebuffers                by engine
uniform                     by user / render system

RENDERER
command pool     by engine
command buffer   by engine
vertex buffer    by user / render system
index buffer     by user / render system


 */

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

    SY_OUTPUT_INFO("Created render resources.");
}

void sy_render_destroy_resources(SyRenderInfo *render_info)
{
    sy_render_destroy_swapchain(render_info);
    vkDestroyDevice(render_info->logical_device, NULL);    

    SY_OUTPUT_INFO("Destroyed render resources.");
}

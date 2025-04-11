#include "sy_render_info.hpp"

#include "sy_resources.hpp"

#include <stdlib.h>
#include <vulkan/vulkan_core.h>

#include "render/sy_render_defines.hpp"
#include "render/sy_physical_device.hpp"
#include "render/sy_logical_device.hpp"
#include "render/sy_swapchain.hpp"

#include "sy_pipeline.hpp"
#include "sy_macros.hpp"

void sy_render_info_init(SyRenderInfo *render_info, int win_width, int win_height)
{
    render_info->current_frame = 0;
    sy_render_create_physical_device(render_info);
    sy_render_create_logical_device(render_info);
    sy_render_create_swapchain(render_info, win_width, win_height);
    sy_render_create_command_pool(render_info);
    render_info->render_pass = sy_render_create_simple_render_pass(render_info);
    sy_render_create_swapchain_framebuffers(render_info);
    sy_render_create_command_buffers(render_info);
    sy_render_create_sync_objects(render_info);
    sy_render_create_descriptor_set_layouts(render_info);
    sy_render_create_allocator(render_info);
    sy_render_create_pipelines(render_info);

    render_info->frame_uniform_data = new SyFrameUniformData[SY_RENDER_MAX_FRAMES_IN_FLIGHT];
    for (int i = 0; i < SY_RENDER_MAX_FRAMES_IN_FLIGHT; ++i)
    {
	render_info->frame_uniform_data[i].descriptor_allocator.init_pool(render_info->logical_device, 20, 1);
    }
}

void sy_render_info_deinit(SyRenderInfo *render_info)
{
    vkDeviceWaitIdle(render_info->logical_device);

    for (int i = 0; i < SY_RENDER_MAX_FRAMES_IN_FLIGHT; ++i)
    {
	for (SyUniformAllocation &uniform_allocation : render_info->frame_uniform_data[i].allocations)
	{
	    vmaDestroyBuffer(render_info->vma_allocator, uniform_allocation.buffer, uniform_allocation.allocation);
	}
	render_info->frame_uniform_data[i].descriptor_allocator.destroy_pool(render_info->logical_device);
    }
    delete[] render_info->frame_uniform_data;

    vkDestroyPipeline(render_info->logical_device, render_info->single_color_pipeline, NULL);

    vkDestroyPipelineLayout(render_info->logical_device, render_info->single_color_pipeline_layout, NULL);

    vmaDestroyAllocator(render_info->vma_allocator);

    vkDestroyDescriptorSetLayout(render_info->logical_device, render_info->frame_descriptor_set_layout, NULL);
    vkDestroyDescriptorSetLayout(render_info->logical_device, render_info->material_descriptor_set_layout, NULL);
    vkDestroyDescriptorSetLayout(render_info->logical_device, render_info->object_descriptor_set_layout, NULL);

    free(render_info->command_buffers);

    for (int i = 0; i < SY_RENDER_MAX_FRAMES_IN_FLIGHT; ++i)
    {
	vkDestroySemaphore(render_info->logical_device, render_info->image_available_semaphores[i], NULL);
	vkDestroySemaphore(render_info->logical_device, render_info->render_finished_semaphores[i], NULL);
	vkDestroyFence(render_info->logical_device, render_info->in_flight_fences[i], NULL);
    }
    free(render_info->image_available_semaphores);
    free(render_info->render_finished_semaphores);
    free(render_info->in_flight_fences);
    
    for (int i = 0; i < render_info->swapchain_framebuffers_amt; ++i)
    {
	vkDestroyFramebuffer(render_info->logical_device, render_info->swapchain_framebuffers[i], NULL);
    }
    free(render_info->swapchain_framebuffers);

    vkDestroyRenderPass(render_info->logical_device, render_info->render_pass, NULL);

    vkDestroyCommandPool(render_info->logical_device, render_info->command_pool, NULL); // command buffers are freed when command pool is freed

    sy_render_destroy_swapchain(render_info);
    vkDestroyDevice(render_info->logical_device, NULL);
}

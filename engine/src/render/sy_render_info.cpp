#include "sy_render_info.hpp"

#include "sy_resources.hpp"

#include <stdlib.h>
#include <vulkan/vulkan_core.h>

#include "render/sy_render_defines.hpp"
#include "render/sy_physical_device.hpp"
#include "render/sy_logical_device.hpp"
#include "render/sy_swapchain.hpp"
#include "render/sy_buffer.hpp"
#include "render/sy_fonts.hpp"

#include "sy_pipeline.hpp"
#include "sy_macros.hpp"

void sy_render_info_init(SyRenderInfo *render_info, int win_width, int win_height)
{
    render_info->current_frame = 0;
    sy_render_create_physical_device(render_info);
    sy_render_create_logical_device(render_info);
    sy_render_create_allocator(render_info);
    sy_render_create_swapchain(render_info, win_width, win_height);
    sy_render_create_command_pool(render_info);
    render_info->render_pass = sy_render_create_simple_render_pass(render_info);
    sy_render_create_framebuffers(render_info);
    sy_render_create_command_buffers(render_info);
    sy_render_create_sync_objects(render_info);
    sy_render_create_descriptor_set_layouts(render_info);
    sy_render_create_pipelines(render_info);

    render_info->frame_uniform_data = new SyFrameUniformDataInfo[SY_RENDER_MAX_FRAMES_IN_FLIGHT];
    for (int i = 0; i < SY_RENDER_MAX_FRAMES_IN_FLIGHT; ++i)
    {
	render_info->frame_uniform_data[i].descriptor_allocator.init_pool(render_info->logical_device, 1500, 1);
    }

    // Test

    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.pNext = NULL;
    sampler_create_info.flags = 0;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.unnormalizedCoordinates = VK_TRUE;
    sampler_create_info.anisotropyEnable = VK_FALSE;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    
    SY_ERROR_COND(vkCreateSampler(render_info->logical_device, &sampler_create_info, NULL, &render_info->font_sampler) != VK_SUCCESS, "RENDER - Failed to create nearest sampler.");

    {
	float vertex_data[] =
	    {
		0.0f, 0.0f, // 0 top left
		1.0f, 0.0f,  // 1 top right
		0.0f, 1.0f,  // 2 bottom left
		1.0f, 1.0f    // 3 bottom right
	    };

	sy_render_create_vertex_buffer(render_info, SY_ARRLEN(vertex_data) * sizeof(vertex_data[0]), (uint8_t*)vertex_data, &render_info->quad_buffer.buffer, &render_info->quad_buffer.allocation);
    }

}

void sy_render_info_deinit(SyRenderInfo *render_info)
{
    vkDeviceWaitIdle(render_info->logical_device);

    vmaDestroyBuffer(render_info->vma_allocator, render_info->quad_buffer.buffer, render_info->quad_buffer.allocation);

    vkDestroySampler(render_info->logical_device, render_info->font_sampler, NULL);

    for (int i = 0; i < SY_RENDER_MAX_FRAMES_IN_FLIGHT; ++i)
    {
	for (SyBufferAllocation &uniform_allocation : render_info->frame_uniform_data[i].allocations)
	{
	    vmaDestroyBuffer(render_info->vma_allocator, uniform_allocation.buffer, uniform_allocation.allocation);
	}
	render_info->frame_uniform_data[i].descriptor_allocator.destroy_pool(render_info->logical_device);
    }
    delete[] render_info->frame_uniform_data;

    vkDestroyPipeline(render_info->logical_device, render_info->single_color_pipeline, NULL);
    vkDestroyPipelineLayout(render_info->logical_device, render_info->single_color_pipeline_layout, NULL);

    vkDestroyPipeline(render_info->logical_device, render_info->text_pipeline, NULL);
    vkDestroyPipelineLayout(render_info->logical_device, render_info->text_pipeline_layout, NULL);

    vkDestroyDescriptorSetLayout(render_info->logical_device, render_info->frame_descriptor_set_layout, NULL);
    vkDestroyDescriptorSetLayout(render_info->logical_device, render_info->material_descriptor_set_layout, NULL);
    vkDestroyDescriptorSetLayout(render_info->logical_device, render_info->object_descriptor_set_layout, NULL);

    vkDestroyDescriptorSetLayout(render_info->logical_device, render_info->character_map_descriptor_set_layout, NULL);
    vkDestroyDescriptorSetLayout(render_info->logical_device, render_info->character_information_descriptor_set_layout, NULL);
    vkDestroyDescriptorSetLayout(render_info->logical_device, render_info->text_buffer_descriptor_set_layout, NULL);    

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
    
    for (uint32_t i = 0; i < render_info->swapchain_framebuffers_amt; ++i)
    {
	vkDestroyFramebuffer(render_info->logical_device, render_info->swapchain_framebuffers[i], NULL);
    }
    free(render_info->swapchain_framebuffers);

    vkDestroyRenderPass(render_info->logical_device, render_info->render_pass, NULL);

    vkDestroyCommandPool(render_info->logical_device, render_info->command_pool, NULL); // command buffers are freed when command pool is freed

    sy_render_destroy_swapchain(render_info);

    vmaDestroyAllocator(render_info->vma_allocator);

    vkDestroyDevice(render_info->logical_device, NULL);
}

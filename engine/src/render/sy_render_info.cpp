#include "sy_render_info.hpp"

#include "sy_resources.hpp"

#include <stdlib.h>
#include <vulkan/vulkan_core.h>

#include "render/sy_render_defines.hpp"
#include "render/sy_physical_device.hpp"
#include "render/sy_logical_device.hpp"
#include "render/sy_swapchain.hpp"
#include "render/sy_buffer.hpp"

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
	render_info->frame_uniform_data[i].descriptor_allocator.init_pool(render_info->logical_device, 20, 1);
    }

    // Test


    //checkerboard image
    uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
    uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
    uint32_t pixels[16*16]; //for 16x16 checkerboard texture
    for (int x = 0; x < 16; x++) {
	for (int y = 0; y < 16; y++) {
	    pixels[y*16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
	}
    }

    render_info->error_image = sy_render_create_texture_image(render_info, (void*)pixels, VkExtent2D{.width=16, .height=16}, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.pNext = NULL;
    sampler_create_info.flags = 0;
    sampler_create_info.magFilter = VK_FILTER_NEAREST;
    sampler_create_info.minFilter = VK_FILTER_NEAREST;
    sampler_create_info.unnormalizedCoordinates = VK_TRUE;
    sampler_create_info.anisotropyEnable = VK_FALSE;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;


    SY_ERROR_COND(vkCreateSampler(render_info->logical_device, &sampler_create_info, NULL, &render_info->nearest_sampler) != VK_SUCCESS, "RENDER - Failed to create nearest sampler.");

    float vertex_data[] =
	{
	    -1.0f, -1.0f,
	    -1.0f, 1.0f,
	    1.0f, -1.0f,
	    1.0f, 1.0f,
	};
    size_t vertex_data_size = SY_ARRLEN(vertex_data);

    sy_render_create_vertex_buffer(render_info, vertex_data_size * sizeof(vertex_data[0]), (uint8_t*)vertex_data, &render_info->error_image_mesh.vertex_buffer, &render_info->error_image_mesh.vertex_buffer_alloc);

    struct TextBufferData
    {
	glm::vec2 pos_offset;
	glm::uvec2 tex_bottom_left;
	glm::uvec2 tex_top_right;
    } *text_buffer_data;
    render_info->character_amt = 2;
    render_info->storage_buffer_size = render_info->character_amt * sizeof(TextBufferData);
    text_buffer_data = (TextBufferData*)calloc(sizeof(TextBufferData), render_info->character_amt);
    
    text_buffer_data[0].pos_offset = glm::vec2(0.0f, 0.0f);
    text_buffer_data[0].tex_bottom_left = glm::uvec2(0, 0);
    text_buffer_data[0].tex_top_right = glm::uvec2(16, 16);
    text_buffer_data[1].pos_offset = glm::vec2(0.0f, 0.0f);
    text_buffer_data[1].tex_bottom_left = glm::uvec2(0, 0);
    text_buffer_data[1].tex_top_right = glm::uvec2(12, 12);
    
    for (int i = 0; i < SY_RENDER_MAX_FRAMES_IN_FLIGHT; ++i)
    {
	sy_render_create_storage_buffer(render_info, text_buffer_data, sizeof(TextBufferData) * render_info->character_amt, &render_info->storage_buffer[i], &render_info->storage_buffer_allocation[i]);
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

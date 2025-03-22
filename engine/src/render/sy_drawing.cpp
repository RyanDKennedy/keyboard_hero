#include "sy_drawing.hpp"
#include "render/sy_resources.hpp"
#include "sy_ecs.hpp"
#include "sy_macros.hpp"
#include "sy_swapchain.hpp"
#include "sy_opaque_types.hpp"
#include <stdlib.h>

void recreate_swapchain(SyRenderInfo *render_info, SyInputInfo *input_info)
{
    vkDeviceWaitIdle(render_info->logical_device);

    for (int i = 0; i < render_info->swapchain_framebuffers_amt; ++i)
    {
	vkDestroyFramebuffer(render_info->logical_device, render_info->swapchain_framebuffers[i], NULL);
    }
    free(render_info->swapchain_framebuffers);
    sy_render_destroy_swapchain(render_info);

    sy_render_create_swapchain(render_info, input_info->window_width, input_info->window_height);
    sy_render_create_swapchain_framebuffers(render_info);
}

void record_command_buffer(SyRenderInfo *render_info, VkCommandBuffer command_buffer, uint32_t image_index, SyPipeline *pipeline, SyEcs *ecs)
{
    VkCommandBufferBeginInfo command_buffer_begin_info;
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext = NULL;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pInheritanceInfo = NULL;

    SY_ERROR_COND(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS, "RENDER: Failed to start command buffer.");

    VkRenderPassBeginInfo render_pass_begin_info;
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.pNext = NULL;
    render_pass_begin_info.renderPass = render_info->render_pass;
    render_pass_begin_info.framebuffer = render_info->swapchain_framebuffers[image_index];
    render_pass_begin_info.renderArea.offset.x = 0;
    render_pass_begin_info.renderArea.offset.y = 0;
    render_pass_begin_info.renderArea.extent = render_info->swapchain_image_extent;
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &clear_color;
    
    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);

    // set the dynamic things in the pipeline (viewport and scissor)

    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = render_info->swapchain_image_extent.width;
    viewport.height = render_info->swapchain_image_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.extent.width = render_info->swapchain_image_extent.width;
    scissor.extent.height = render_info->swapchain_image_extent.height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    size_t mesh_type_id = ecs->get_type_id<SyMesh>();
    for (size_t i = 0; i < ecs->m_entity_used.m_filled_length; ++i)
    {
	if (ecs->m_entity_used.get<bool>(i) == true)
	{
	    if (ecs->m_entity_data.get<SyEntityData>(i).mask[mesh_type_id] == true)
	    {
		SyMesh *mesh = ecs->component<SyMesh>(i);

		// Buffers
		VkDeviceSize vertex_buffer_offset = 0;
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &mesh->vertex_buffer, &vertex_buffer_offset);
		vkCmdBindIndexBuffer(command_buffer, mesh->index_buffer, 0, VK_INDEX_TYPE_UINT32);
		
		// Uniforms
		// vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mesh->pipeline_layout, 0, 1, &mesh->descriptor_sets[mesh->current_frame], 0, NULL);
		
		// Draw
		vkCmdDrawIndexed(command_buffer, mesh->index_amt, 1, 0, 0, 0);
		
	    }
	}
    }



    vkCmdEndRenderPass(command_buffer);
    SY_ERROR_COND(vkEndCommandBuffer(command_buffer) != VK_SUCCESS, "RENDER: Failed to end command buffer.");
}

void sy_render_draw(SyRenderInfo *render_info, SyPipeline *pipeline, SyInputInfo *input_info, SyEcs *ecs)
{
    VkResult result;

    // Wait for previous frame to finish
    vkWaitForFences(render_info->logical_device, 1, &render_info->in_flight_fences[render_info->current_frame], VK_TRUE, UINT64_MAX);

    // Acquire image from swap chain
    uint32_t image_index;
    result = vkAcquireNextImageKHR(render_info->logical_device, render_info->swapchain, UINT64_MAX,
			  render_info->image_available_semaphores[render_info->current_frame], VK_NULL_HANDLE, &image_index);

    // Check results of acquiring the image, and if we need to recreate swapchain
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    { // has to resize
	recreate_swapchain(render_info, input_info);
	return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    { // error code returned, you can still present if if it is VK_SUBOPTIMAL_KHR
	SY_ERROR("Failed to acquire swap chain miage.");
    }

    // Only reset fence if we know for sure that we will be submitting work
    vkResetFences(render_info->logical_device, 1, &render_info->in_flight_fences[render_info->current_frame]);
    
    // Record command buffer
    vkResetCommandBuffer(render_info->command_buffers[render_info->current_frame], 0);
    record_command_buffer(render_info, render_info->command_buffers[render_info->current_frame], image_index, pipeline, ecs);

    // update_uniform_buffer(render_info, render_info->current_frame);

    // Submit command buffer

    // wait for an image to be available to attach the color attachment
    VkSemaphore wait_semaphores[] = {render_info->image_available_semaphores[render_info->current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSemaphore signal_semaphores[] = {render_info->render_finished_semaphores[render_info->current_frame]};

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &render_info->command_buffers[render_info->current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores; // signal these semaphores when done rendering

    SY_ERROR_COND(vkQueueSubmit(render_info->graphics_queue, 1, &submit_info, render_info->in_flight_fences[render_info->current_frame]) != VK_SUCCESS,
		  "ERROR: Failed to submit queue.");

    // Presentation
    VkSwapchainKHR swap_chains[] = {render_info->swapchain};

    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = 0;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores; // wait until rendering is done
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = NULL;

    result = vkQueuePresentKHR(render_info->present_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || input_info->window_resized)
    {
	recreate_swapchain(render_info, input_info);
    }
    else if (result != VK_SUCCESS)
    {
	SY_ERROR("Failed to present swap chain image.");
    }

    render_info->current_frame = (render_info->current_frame + 1) % render_info->max_frames_in_flight;

}

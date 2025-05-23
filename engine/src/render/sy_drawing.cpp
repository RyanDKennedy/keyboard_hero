#include "sy_drawing.hpp"

#include "components/sy_transform.hpp"
#include "render/sy_buffer.hpp"
#include "render/sy_frame_uniform_data_info.hpp"
#include "render/sy_render_defines.hpp"
#include "render/sy_render_info.hpp"
#include "render/sy_resources.hpp"
#include "render/types/sy_asset_metadata.hpp"
#include "render/types/sy_draw_info.hpp"
#include "render/types/sy_material.hpp"
#include "render/types/sy_ui_text.hpp"
#include "sy_ecs.hpp"
#include "sy_macros.hpp"
#include "sy_swapchain.hpp"
#include "types/sy_camera_settings.hpp"
#include "types/sy_mesh.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>



void recreate_swapchain(SyRenderInfo *render_info, SyInputInfo *input_info)
{
    vkDeviceWaitIdle(render_info->logical_device);

    for (int i = 0; i < (int)render_info->swapchain_framebuffers_amt; ++i)
    {
	vkDestroyFramebuffer(render_info->logical_device, render_info->swapchain_framebuffers[i], NULL);
    }
    free(render_info->swapchain_framebuffers);
    sy_render_destroy_swapchain(render_info);

    sy_render_create_swapchain(render_info, input_info->window_width, input_info->window_height);
    sy_render_create_framebuffers(render_info);
}

VkDescriptorSet create_and_write_to_descriptor_set_and_uniform_buffer(SyRenderInfo *render_info, VkDescriptorSetLayout layout, void *data, size_t data_size)
{
    VkDescriptorSet descriptor_set = render_info->frame_uniform_data[render_info->current_frame].descriptor_allocator.allocate(render_info->logical_device, layout);
    SyBufferAllocation allocation;
    
    { // create uniform buffer and move data into it
	VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	buffer_info.size = data_size;
	buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	
	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
	alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
	
	vmaCreateBuffer(render_info->vma_allocator, &buffer_info, &alloc_info, &allocation.buffer, &allocation.allocation, nullptr);
	render_info->frame_uniform_data[render_info->current_frame].allocations.push_back(allocation);
	
	vmaCopyMemoryToAllocation(render_info->vma_allocator, data, allocation.allocation, 0, data_size);
    }
    
    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = allocation.buffer;
    buffer_info.offset = 0;
    buffer_info.range = data_size;
    
    VkWriteDescriptorSet descriptor_write;
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.pNext = NULL;
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.pImageInfo = NULL;
    descriptor_write.pTexelBufferView = NULL;
    
    vkUpdateDescriptorSets(render_info->logical_device, 1, &descriptor_write, 0, NULL);

    return descriptor_set;
}

VkDescriptorSet create_and_write_to_descriptor_set_and_storage_buffer(SyRenderInfo *render_info, VkDescriptorSetLayout layout, void *data, size_t data_size)
{
    VkDescriptorSet descriptor_set = render_info->frame_uniform_data[render_info->current_frame].descriptor_allocator.allocate(render_info->logical_device, layout);
    SyBufferAllocation allocation;
    
    { // create uniform buffer and move data into it
	VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	buffer_info.size = data_size;
	buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	
	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
	alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
	
	vmaCreateBuffer(render_info->vma_allocator, &buffer_info, &alloc_info, &allocation.buffer, &allocation.allocation, nullptr);
	render_info->frame_uniform_data[render_info->current_frame].allocations.push_back(allocation);
	
	vmaCopyMemoryToAllocation(render_info->vma_allocator, data, allocation.allocation, 0, data_size);
    }
    
    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = allocation.buffer;
    buffer_info.offset = 0;
    buffer_info.range = data_size;
    
    VkWriteDescriptorSet descriptor_write;
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.pNext = NULL;
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;
    descriptor_write.pImageInfo = NULL;
    descriptor_write.pTexelBufferView = NULL;
    
    vkUpdateDescriptorSets(render_info->logical_device, 1, &descriptor_write, 0, NULL);

    return descriptor_set;
}


VkDescriptorSet create_descriptor_set_and_image(SyRenderInfo *render_info, VkDescriptorSetLayout layout, VkImageView image_view, VkSampler sampler)
{
    VkDescriptorSet descriptor_set = render_info->frame_uniform_data[render_info->current_frame].descriptor_allocator.allocate(render_info->logical_device, layout);
    
    VkDescriptorImageInfo image_info = {};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.sampler = sampler;
    image_info.imageView = image_view;
    
    VkWriteDescriptorSet descriptor_write;
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.pNext = NULL;
    descriptor_write.dstSet = descriptor_set;
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = NULL;
    descriptor_write.pImageInfo = &image_info;
    descriptor_write.pTexelBufferView = NULL;
    
    vkUpdateDescriptorSets(render_info->logical_device, 1, &descriptor_write, 0, NULL);

    return descriptor_set;
}

void record_command_buffer(SyRenderInfo *render_info, VkCommandBuffer command_buffer, uint32_t image_index, SyEcs *ecs, SyCameraSettings *camera_settings, float window_aspect_ratio)
{
    { // Reset descriptor bullshit
	render_info->frame_uniform_data[render_info->current_frame].descriptor_allocator.clear_descriptors(render_info->logical_device);
	for (SyBufferAllocation &uniform_allocation : render_info->frame_uniform_data[render_info->current_frame].allocations)
	{
	    vmaDestroyBuffer(render_info->vma_allocator, uniform_allocation.buffer, uniform_allocation.allocation);
	}
	render_info->frame_uniform_data[render_info->current_frame].allocations.clear();
    }



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

    VkClearValue clear_values[2];
    clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clear_values[1].depthStencil = {1.0f, 0};
    render_pass_begin_info.clearValueCount = SY_ARRLEN(clear_values);
    render_pass_begin_info.pClearValues = clear_values;
    
    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);



    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_info->single_color_pipeline);

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
    
    { // Create frame data uniform

	// Create SyCamera
	SyTransform *transform = ecs->component<SyTransform>(camera_settings->active_camera);
	
	glm::mat4 view;
	{
	    glm::vec3 dir(0.0f, 0.0f, 0.0f);

	    dir[0] = glm::sin(glm::radians((float)transform->rotation[1])) * glm::cos(glm::radians((float)transform->rotation[0]));
	    dir[1] = glm::sin(glm::radians((float)transform->rotation[0]));
	    dir[2] = glm::cos(glm::radians((float)transform->rotation[1])) * glm::cos(glm::radians((float)transform->rotation[0]));

	    view = glm::lookAt(transform->position, transform->position + dir, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::mat4 projection;
	if (camera_settings->projection_type == SyCameraProjectionType::perspective)
	{
	    projection = glm::perspective(glm::radians(camera_settings->perspective_settings.fov), camera_settings->perspective_settings.aspect_ratio, camera_settings->perspective_settings.near_plane, camera_settings->perspective_settings.far_plane);
	}
	else
	{
	    projection = glm::ortho(camera_settings->orthographic_settings.left, camera_settings->orthographic_settings.right, camera_settings->orthographic_settings.bottom, camera_settings->orthographic_settings.top, camera_settings->orthographic_settings.near, camera_settings->orthographic_settings.far);
	}
	projection[1][1] *= -1;

	struct
	{
	    glm::mat4 vp_matrix;
	} frame_uniform_structure;
	frame_uniform_structure.vp_matrix = projection * view;

	VkDescriptorSet descriptor_set = create_and_write_to_descriptor_set_and_uniform_buffer(render_info, render_info->frame_descriptor_set_layout, &frame_uniform_structure, sizeof(frame_uniform_structure));
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_info->single_color_pipeline_layout, 0, 1, &descriptor_set, 0, NULL);
    }

    std::vector<size_t> fonts_to_render; // FIXME

    // Draw entities
    for (size_t i = 0; i < ecs->m_entity_used.size(); ++i)
    {
	if (ecs->is_entity_index_used(i) != true)
	    continue;

	if (ecs->entity_has_component<SyDrawInfo>(i) != true)
	    continue;

	SyDrawInfo *draw_info = ecs->component<SyDrawInfo>(i);
	if (draw_info->should_draw == false)
	    continue;
	
	SyAssetMetadata *asset_metadata = ecs->component_from_index<SyAssetMetadata>(draw_info->asset_metadata_id);
	switch(asset_metadata->asset_type)
	{
	    case SyAssetType::mesh:
	    {
		SyMesh *mesh = ecs->component_from_index<SyMesh>(asset_metadata->asset_component_index);

		{
		    struct
		    {
			glm::vec3 diffuse;
		    } material_uniform_struct;
		    material_uniform_struct.diffuse = glm::vec3(1.0f, 0.0f, 1.0f);
		    if (ecs->entity_has_component<SyMaterial>(i) == true)
		    {
			SyMaterial *material = ecs->component<SyMaterial>(i);			
			material_uniform_struct.diffuse = material->diffuse;
		    }

		    VkDescriptorSet descriptor_set = create_and_write_to_descriptor_set_and_uniform_buffer(render_info, render_info->material_descriptor_set_layout, &material_uniform_struct, sizeof(material_uniform_struct));
		    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_info->single_color_pipeline_layout, 1, 1, &descriptor_set, 0, NULL);
		}
		{
		    struct
		    {
			glm::mat4 model_matrix;
		    } mesh_uniform_struct;
		    mesh_uniform_struct.model_matrix = glm::mat4(1);
		    
		    // Get model matrix information
		    if (ecs->entity_has_component<SyTransform>(i) == true)
		    {
			SyTransform *transform = ecs->component<SyTransform>(i);
			
			mesh_uniform_struct.model_matrix = glm::scale(glm::mat4(1), transform->scale) * mesh_uniform_struct.model_matrix;
			mesh_uniform_struct.model_matrix = glm::rotate(glm::mat4(1), glm::radians(transform->rotation[0]), glm::vec3(1.0f, 0.0f, 0.0f)) * mesh_uniform_struct.model_matrix;
			mesh_uniform_struct.model_matrix = glm::rotate(glm::mat4(1), glm::radians(transform->rotation[2]), glm::vec3(0.0f, 0.0f, 1.0f)) * mesh_uniform_struct.model_matrix;
			mesh_uniform_struct.model_matrix = glm::rotate(glm::mat4(1), glm::radians(transform->rotation[1]), glm::vec3(0.0f, 1.0f, 0.0f)) * mesh_uniform_struct.model_matrix;
			mesh_uniform_struct.model_matrix = glm::translate(glm::mat4(1), transform->position) * mesh_uniform_struct.model_matrix;
		    }
		    
		    VkDescriptorSet descriptor_set = create_and_write_to_descriptor_set_and_uniform_buffer(render_info, render_info->object_descriptor_set_layout, &mesh_uniform_struct, sizeof(mesh_uniform_struct));
		    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_info->single_color_pipeline_layout, 2, 1, &descriptor_set, 0, NULL);
		    
		}

		// Buffers
		VkDeviceSize vertex_buffer_offset = 0;
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &mesh->vertex_buffer, &vertex_buffer_offset);
		vkCmdBindIndexBuffer(command_buffer, mesh->index_buffer, 0, VK_INDEX_TYPE_UINT32);
		
		// Draw
		vkCmdDrawIndexed(command_buffer, mesh->index_amt, 1, 0, 0, 0);
	    }
	    break;

	    case SyAssetType::font:
	    {
		fonts_to_render.push_back(i);
	    }
	    break;
	    
	    default:
		continue;
	}
	
    }

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_info->text_pipeline);
    // set the dynamic things in the pipeline (viewport and scissor)
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    
    VkDeviceSize vertex_buffer_offset = 0;
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &render_info->quad_buffer.buffer, &vertex_buffer_offset);

    for (size_t font_entity_index : fonts_to_render)
    {
	SyDrawInfo *draw_info = ecs->component<SyDrawInfo>(font_entity_index);
	SyAssetMetadata *metadata = ecs->component_from_index<SyAssetMetadata>(draw_info->asset_metadata_id);
	SyFont *font = ecs->component_from_index<SyFont>(metadata->asset_component_index);
	SyRenderImage *font_image = ecs->component_from_index<SyRenderImage>(font->texture_index);

	// Assign a SyUIText to render, default if it doesn't have one
	SyUIText ui_text;
	ui_text.text = "No Text";
	ui_text.color = glm::vec3(1.0f, 0.0f, 1.0f);
	ui_text.pos = glm::vec2(0.0f, 0.0f);
	ui_text.scale = glm::vec2(0.1f, 0.1f);
	ui_text.alignment = SyTextAlignment::center;
	if (ecs->entity_has_component<SyUIText>(font_entity_index))
	{
	    ui_text = *ecs->component<SyUIText>(font_entity_index);
	}

	// Calculate if we need to render based on size of text buffer
	struct TextBufferData
	{
	    glm::vec2 pos_offset;
	    glm::vec2 scale;
	    glm::uvec2 tex_bottom_left;
	    glm::uvec2 tex_top_right;
	} *text_buffer_data;

	size_t text_len = strlen(ui_text.text);
	size_t buf_len = text_len;
	for (size_t i = 0; i < strlen(ui_text.text); ++i)
	{
	    if (font->character_map.contains(ui_text.text[i]) == false || ui_text.text[i] == '\n' || ui_text.text[i] == '\t' || ui_text.text[i] == ' ')
		--buf_len;
	}
	if (buf_len == 0)
	    continue;

	// Bind/Create uniforms
	VkDescriptorSet character_map_descriptor_set = create_descriptor_set_and_image(render_info, render_info->character_map_descriptor_set_layout, font_image->image_view, render_info->font_sampler);

	struct
	{
	    glm::vec3 color;
	} character_information_data;
	character_information_data.color = ui_text.color;

	VkDescriptorSet character_information_descriptor_set = create_and_write_to_descriptor_set_and_uniform_buffer(render_info, render_info->character_information_descriptor_set_layout, &character_information_data, sizeof(character_information_data));

	VkDescriptorSet sets[] = {character_map_descriptor_set, character_information_descriptor_set};
	
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_info->text_pipeline_layout, 0, 2, sets, 0, NULL);

	// Create Text Buffer
	size_t storage_buffer_size = sizeof(TextBufferData) * buf_len;
	text_buffer_data = (TextBufferData*)calloc(sizeof(text_buffer_data[0]), buf_len);


	std::vector<float> line_lengths;
	{
	    float next_x = 0;
	    for (size_t i = 0; i < text_len; ++i)
	    {
		if (font->character_map.contains(ui_text.text[i]) == false && ui_text.text[i] != '\n' && ui_text.text[i] != '\t')
		    continue;

		if (ui_text.text[i] == '\n')
		{
		    line_lengths.push_back(next_x);
		    next_x = 0;
		    continue;
		}
		
		if (ui_text.text[i] == '\t')
		{
		    SyFontCharacter char_data = font->character_map[' '];
		    next_x = next_x + (char_data.advance * ui_text.scale[0] / window_aspect_ratio) * 4;
		    continue;
		}

		SyFontCharacter char_data = font->character_map[ui_text.text[i]];
		next_x = next_x + (char_data.advance * ui_text.scale[0] / window_aspect_ratio);
	    }

	    line_lengths.push_back(next_x);

	}
	
	float next_x = ui_text.pos[0];
	float next_y = ui_text.pos[1];
	size_t line_number = 0;
	if (ui_text.alignment == SyTextAlignment::center)
	{
	    next_x -= (float)line_lengths[line_number++] / 2.0f;
	}
	else if (ui_text.alignment == SyTextAlignment::right)
	{
	    next_x -= (float)line_lengths[line_number++];
	}

	size_t buf_index = 0;
	for (size_t i = 0; i < text_len; ++i)
	{
	    if (font->character_map.contains(ui_text.text[i]) == false && ui_text.text[i] != '\n' && ui_text.text[i] != '\t' && ui_text.text[i] != ' ')
		continue;

	    if (ui_text.text[i] == '\n')
	    {
		next_x = ui_text.pos[0];
		next_y = next_y + (font->line_height * ui_text.scale[1]);

		if (ui_text.alignment == SyTextAlignment::center)
		{
		    next_x -= (float)line_lengths[line_number++] / 2.0f;
		}
		else if (ui_text.alignment == SyTextAlignment::right)
		{
		    next_x -= (float)line_lengths[line_number++];
		}
		
		continue;
	    }

	    if (ui_text.text[i] == ' ')
	    {
		SyFontCharacter char_data = font->character_map[' '];
		next_x = next_x + (char_data.advance * ui_text.scale[0] / window_aspect_ratio);
		continue;
	    }

	    if (ui_text.text[i] == '\t')
	    {
		SyFontCharacter char_data = font->character_map[' '];
		next_x = next_x + (char_data.advance * ui_text.scale[0] / window_aspect_ratio) * 4;
		continue;
	    }
	    
	    SyFontCharacter char_data = font->character_map[ui_text.text[i]];

	    text_buffer_data[buf_index].pos_offset = glm::vec2(next_x + char_data.offset[0] * ui_text.scale[0] / window_aspect_ratio, next_y + char_data.offset[1] * ui_text.scale[1]);
	    text_buffer_data[buf_index].scale = glm::vec2(ui_text.scale[0] * char_data.scale[0] / window_aspect_ratio, ui_text.scale[1] * char_data.scale[1]);
	    text_buffer_data[buf_index].tex_bottom_left = glm::uvec2(char_data.tex_bottom_left[0] - 1, char_data.tex_bottom_left[1] - 1);
	    text_buffer_data[buf_index].tex_top_right = glm::uvec2(char_data.tex_top_right[0] + 1, char_data.tex_top_right[1] + 1);
	    ++buf_index;

	    next_x = next_x + (char_data.advance * ui_text.scale[0] / window_aspect_ratio);
	}
	
	VkDescriptorSet text_buffer_descriptor_set = create_and_write_to_descriptor_set_and_storage_buffer(render_info, render_info->text_buffer_descriptor_set_layout, text_buffer_data, storage_buffer_size);
	
	free(text_buffer_data);
	
	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_info->text_pipeline_layout, 2, 1, &text_buffer_descriptor_set, 0, NULL);
	

	// Draw
	vkCmdDraw(command_buffer, 4, buf_len, 0, 0);
    }

    vkCmdEndRenderPass(command_buffer);
    SY_ERROR_COND(vkEndCommandBuffer(command_buffer) != VK_SUCCESS, "RENDER: Failed to end command buffer.");
}

void sy_render_draw(SyRenderInfo *render_info, SyInputInfo *input_info, SyEcs *ecs, SyCameraSettings *camera_settings)
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
    record_command_buffer(render_info, render_info->command_buffers[render_info->current_frame], image_index, ecs, camera_settings, (float)input_info->window_width / input_info->window_height);

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

    render_info->current_frame = (render_info->current_frame + 1) % SY_RENDER_MAX_FRAMES_IN_FLIGHT;
}

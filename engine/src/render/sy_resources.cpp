#include "sy_resources.hpp"

#include <stdlib.h>
#include <vulkan/vulkan_core.h>

#include "sy_physical_device.hpp"
#include "sy_logical_device.hpp"
#include "sy_swapchain.hpp"
#include "sy_pipeline.hpp"
#include "sy_macros.hpp"

void sy_render_create_pipelines(SyRenderInfo *render_info)
{
    { // Create Pipeline Layouts
	VkDescriptorSetLayout layouts[] = {render_info->frame_descriptor_set_layout, render_info->material_descriptor_set_layout};
	
	VkPipelineLayoutCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.pushConstantRangeCount = 0;
	create_info.pPushConstantRanges = NULL;
	create_info.setLayoutCount = SY_ARRLEN(layouts);
	create_info.pSetLayouts = layouts;
	
	vkCreatePipelineLayout(render_info->logical_device, &create_info, NULL, &render_info->single_color_pipeline_layout);
    }

    { // Create single color pipeline
	VkVertexInputBindingDescription binding_description;
	binding_description.binding = 0;
	binding_description.stride = sizeof(float) * 3;
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;    

	VkVertexInputAttributeDescription attr[1];
	attr[0].binding = 0;
	attr[0].location = 0;
	attr[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attr[0].offset = 0;

	SyPipelineCreateInfo create_info;
	create_info.vertex_shader_path = "single_color/vertex.spv";
	create_info.fragment_shader_path = "single_color/fragment.spv";
	create_info.vertex_input_binding_description = binding_description;
	create_info.vertex_input_attribute_descriptions = attr;
	create_info.vertex_input_attribute_descriptions_amt = 1;
	create_info.render_type = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	create_info.subpass_number = 0;
	create_info.pipeline_layout = render_info->single_color_pipeline_layout;

	render_info->single_color_pipeline = sy_render_create_pipeline(render_info, &create_info);
    }

}

void sy_render_create_descriptor_pool(SyRenderInfo *render_info)
{
    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = render_info->max_descriptor_sets_amt;
    
    VkDescriptorPoolCreateInfo pool_create_info;
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.pNext = NULL;
    pool_create_info.flags = 0;
    pool_create_info.poolSizeCount = 1;
    pool_create_info.pPoolSizes = &pool_size;
    pool_create_info.maxSets = render_info->max_descriptor_sets_amt;
    
    SY_ERROR_COND(vkCreateDescriptorPool(render_info->logical_device, &pool_create_info, NULL, &render_info->descriptor_pool) != VK_SUCCESS, "RENDER: Failed to create descriptor pool.");
    
    render_info->descriptor_sets = (SyDescriptorSetDataGroup*)calloc(render_info->max_descriptor_sets_amt, sizeof(SyDescriptorSetDataGroup));
    render_info->descriptor_sets_used = (bool*)calloc(render_info->max_descriptor_sets_amt, sizeof(bool));
    
}

void sy_render_create_sync_objects(SyRenderInfo *render_info)
{
    render_info->image_available_semaphores = (VkSemaphore*)calloc(render_info->max_frames_in_flight, sizeof(VkSemaphore));
    render_info->render_finished_semaphores = (VkSemaphore*)calloc(render_info->max_frames_in_flight, sizeof(VkSemaphore));
    render_info->in_flight_fences = (VkFence*)calloc(render_info->max_frames_in_flight, sizeof(VkFence));

    VkSemaphoreCreateInfo image_available_semaphore_create_info;
    image_available_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    image_available_semaphore_create_info.pNext = NULL;
    image_available_semaphore_create_info.flags = 0;
    
    VkSemaphoreCreateInfo render_finished_semaphore_create_info;
    render_finished_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    render_finished_semaphore_create_info.pNext = NULL;
    render_finished_semaphore_create_info.flags = 0;
    
    VkFenceCreateInfo fence_create_info;
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = NULL;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (size_t i = 0; i < render_info->max_frames_in_flight; ++i)
    {
	VkResult result1 = vkCreateSemaphore(render_info->logical_device, &image_available_semaphore_create_info, NULL, &render_info->image_available_semaphores[i]);
	VkResult result2 = vkCreateSemaphore(render_info->logical_device, &render_finished_semaphore_create_info, NULL, &render_info->render_finished_semaphores[i]);
	VkResult result3 = vkCreateFence(render_info->logical_device, &fence_create_info, NULL, &render_info->in_flight_fences[i]);

	SY_ERROR_COND( result1 != VK_SUCCESS || result2 != VK_SUCCESS || result3 != VK_SUCCESS, "RENDER: Failed to create semaphores or fence.");
    }

}

void sy_render_create_command_buffers(SyRenderInfo *render_info)
{
    render_info->command_buffers_amt = render_info->max_frames_in_flight;
    render_info->command_buffers = (VkCommandBuffer*)calloc(render_info->command_buffers_amt, sizeof(VkCommandBuffer));

    VkCommandBufferAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.commandPool = render_info->command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // primary or secondary command buffer
    allocate_info.commandBufferCount = (uint32_t)render_info->command_buffers_amt;

    SY_ERROR_COND(vkAllocateCommandBuffers(render_info->logical_device, &allocate_info, render_info->command_buffers) != VK_SUCCESS, "RENDER: Failed to allocate command buffers.");
}

void sy_render_create_command_pool(SyRenderInfo *render_info)
{
    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = NULL;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = render_info->graphics_queue_family_index;
    
    SY_ERROR_COND(vkCreateCommandPool(render_info->logical_device, &command_pool_create_info, NULL, &render_info->command_pool) != VK_SUCCESS, "RENDER: Failed to create command pool.");
}

void sy_render_create_descriptor_set_layouts(SyRenderInfo *render_info)
{
    { // frame data layout
	VkDescriptorSetLayoutBinding ubo_layout_binding;
	ubo_layout_binding.binding = 0; // the binding of the uniform inside of the shader
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	ubo_layout_binding.pImmutableSamplers = NULL; // for image sampling

	VkDescriptorSetLayoutBinding bindings[] = {ubo_layout_binding};
	uint32_t bindings_amt = SY_ARRLEN(bindings);

	VkDescriptorSetLayoutCreateInfo layout_create_info;
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.pNext = NULL;
	layout_create_info.flags = 0;
	layout_create_info.bindingCount = bindings_amt;
	layout_create_info.pBindings = bindings;
	
	VkDescriptorSetLayout result;
	
	SY_ERROR_COND(vkCreateDescriptorSetLayout(render_info->logical_device, &layout_create_info, NULL, &render_info->frame_descriptor_set_layout) != VK_SUCCESS, "RENDER: Failed to create descriptor set layout - frame data layout.");
    }

    { // material layout
	VkDescriptorSetLayoutBinding ubo_layout_binding;
	ubo_layout_binding.binding = 0; // the binding of the uniform inside of the shader
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	ubo_layout_binding.pImmutableSamplers = NULL; // for image sampling

	VkDescriptorSetLayoutBinding bindings[] = {ubo_layout_binding};
	uint32_t bindings_amt = SY_ARRLEN(bindings);

	VkDescriptorSetLayoutCreateInfo layout_create_info;
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.pNext = NULL;
	layout_create_info.flags = 0;
	layout_create_info.bindingCount = bindings_amt;
	layout_create_info.pBindings = bindings;
	
	VkDescriptorSetLayout result;
	
	SY_ERROR_COND(vkCreateDescriptorSetLayout(render_info->logical_device, &layout_create_info, NULL, &render_info->material_descriptor_set_layout) != VK_SUCCESS, "RENDER: Failed to create descriptor set layout - material layout.");
    }

    { // object layout
	VkDescriptorSetLayoutBinding ubo_layout_binding;
	ubo_layout_binding.binding = 0; // the binding of the uniform inside of the shader
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	ubo_layout_binding.pImmutableSamplers = NULL; // for image sampling

	VkDescriptorSetLayoutBinding bindings[] = {ubo_layout_binding};
	uint32_t bindings_amt = SY_ARRLEN(bindings);

	VkDescriptorSetLayoutCreateInfo layout_create_info;
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.pNext = NULL;
	layout_create_info.flags = 0;
	layout_create_info.bindingCount = bindings_amt;
	layout_create_info.pBindings = bindings;
	
	VkDescriptorSetLayout result;
	
	SY_ERROR_COND(vkCreateDescriptorSetLayout(render_info->logical_device, &layout_create_info, NULL, &render_info->object_descriptor_set_layout) != VK_SUCCESS, "RENDER: Failed to create descriptor set layout - object layout.");
    }

}

VkRenderPass sy_render_create_simple_render_pass(SyRenderInfo *render_info)
{
    VkAttachmentDescription color_attachment;
    color_attachment.format = render_info->swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // what to do when the image first gets loaded and ready to draw
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // what to do when finished writing to image 
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // same as loadOp but we don't care about stencil
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // same as storeOp but we don't care about stencil
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // what layout is image before render pass
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // what layout to output image as after render pass
    color_attachment.flags = 0;
    
    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0; // what index our referenced color attachment is stored at
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // what layout we want it stored at during the subpass

    VkSubpassDescription subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // specify it is graphics subpass because future may support compute subpass
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference; // layout(location = 0) out ... in frag shader refers to index 0 of this array to write to
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = NULL;
    subpass.flags = 0;

    // Create subpass dependency so that the implicit subpass where the image format gets converted waits until we have an image
    VkSubpassDependency subpass_dependency;

    // subpass 0 relies on VK_SUBPASS_EXTERNAL AKA image conversion
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;

    // wait on the output of the image conversion
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;

    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    subpass_dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo render_pass_create_info;
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.pNext = NULL;
    render_pass_create_info.flags = 0;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment; // NOTE: subpass contains references to attachments, this contains actual attachments
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    VkRenderPass result;

    SY_ERROR_COND(vkCreateRenderPass(render_info->logical_device, &render_pass_create_info, NULL, &result) != VK_SUCCESS, "RENDER: Failed to create render pass.");

    return result;
}

void sy_render_create_swapchain_framebuffers(SyRenderInfo *render_info)
{
    render_info->swapchain_framebuffers_amt = render_info->swapchain_image_views_amt;
    render_info->swapchain_framebuffers = (VkFramebuffer*)calloc(render_info->swapchain_framebuffers_amt, sizeof(VkFramebuffer));

    // Create a framebuffer for each image view
    for (int i = 0; i < render_info->swapchain_framebuffers_amt; ++i)
    {
	VkImageView attachments[] = { render_info->swapchain_image_views[i] };

	VkFramebufferCreateInfo framebuffer_create_info;
	framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_create_info.pNext = NULL;
	framebuffer_create_info.flags = 0;
	framebuffer_create_info.attachmentCount = SY_ARRLEN(attachments);
	framebuffer_create_info.pAttachments = attachments;
	framebuffer_create_info.width = render_info->swapchain_image_extent.width;
	framebuffer_create_info.height = render_info->swapchain_image_extent.height;
	framebuffer_create_info.layers = 1;
	framebuffer_create_info.renderPass = render_info->render_pass;

	SY_ERROR_COND(vkCreateFramebuffer(render_info->logical_device, &framebuffer_create_info, NULL, &render_info->swapchain_framebuffers[i]) != VK_SUCCESS, "RENDER: Failed to create framebuffer.");
    }



}

size_t sy_render_create_descriptor_set(SyRenderInfo *render_info, size_t uniform_size, void *data, VkDescriptorSetLayout descriptor_layout, uint32_t layout_binding)
{
    // find unused index
    int index = -1;
    for (int i = 0; i < render_info->max_descriptor_sets_amt; ++i)
    {
	if (render_info->descriptor_sets_used[i] == false)
	{
	    render_info->descriptor_sets_used[i] = true;
	    index = i;
	    break;
	}
    }
    SY_ERROR_COND(index == -1, "RENDER: Ran out of descriptor sets.");
	
    // Allocate buffers, and copy data to them
    for (int i = 0; i < render_info->max_frames_in_flight; ++i)
    {
	VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	buffer_info.size = uniform_size;
	buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	
	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
	alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
	
	vmaCreateBuffer(render_info->vma_allocator, &buffer_info, &alloc_info, &render_info->descriptor_sets[index].uniform_buffer[i], &render_info->descriptor_sets[index].uniform_buffer_allocation[i], nullptr);

	vmaCopyMemoryToAllocation(render_info->vma_allocator, data, render_info->descriptor_sets[index].uniform_buffer_allocation[i], 0, uniform_size);
    }

    // Allocate descriptor sets
    
    // Initialize layouts array
    uint32_t layout_amt = render_info->max_frames_in_flight;
    VkDescriptorSetLayout *layouts = (VkDescriptorSetLayout*)calloc(layout_amt, sizeof(VkDescriptorSetLayout));
    for (int i = 0; i < layout_amt; ++i)
    {
	layouts[i] = descriptor_layout;
    }
    
    VkDescriptorSetAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.pSetLayouts = layouts;
    allocate_info.descriptorSetCount = layout_amt;
    allocate_info.descriptorPool = render_info->descriptor_pool;
    
    VkResult result;
    SY_ERROR_COND((result = vkAllocateDescriptorSets(render_info->logical_device, &allocate_info, render_info->descriptor_sets[index].descriptor_set)) != VK_SUCCESS, "RENDER: Failed to allocate descriptor sets. out of pool mem: %d", result == VK_ERROR_OUT_OF_POOL_MEMORY);

    // Populate every descriptor set
    for (size_t i = 0; i < render_info->max_frames_in_flight; ++i)
    {
	VkDescriptorBufferInfo buffer_info;
	buffer_info.buffer = render_info->descriptor_sets[index].uniform_buffer[i];
	buffer_info.offset = 0;
	buffer_info.range = uniform_size;
	
	VkWriteDescriptorSet descriptor_write;
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.pNext = NULL;
	descriptor_write.dstSet = render_info->descriptor_sets[index].descriptor_set[i];
	descriptor_write.dstBinding = layout_binding;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pBufferInfo = &buffer_info;
	descriptor_write.pImageInfo = NULL;
	descriptor_write.pTexelBufferView = NULL;
	
	vkUpdateDescriptorSets(render_info->logical_device, 1, &descriptor_write, 0, NULL);
    }

    // Cleanup
    free(layouts);

    return index;
}

#include "sy_resources.hpp"

#include <stdlib.h>

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

void sy_create_command_pool(SyRenderInfo *render_info)
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
    { // single ubo shader layout
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
	
	SY_ERROR_COND(vkCreateDescriptorSetLayout(render_info->logical_device, &layout_create_info, NULL, &render_info->single_ubo_descriptor_set_layout) != VK_SUCCESS, "RENDER: Failed to create descriptor set layout - single ubo.");
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

#include "sy_pipeline.hpp"

#include "render/sy_render_info.hpp"
#include "sy_macros.hpp"

#include <stdlib.h>

void create_render_pass(SyRenderInfo *render_info, SyRenderPipelineInfo *pipeline_info);
void create_descriptor_set_layout(SyRenderInfo *render_info, SyRenderPipelineInfo *pipeline_info);
void create_pipeline(SyRenderInfo *render_info, SyRenderPipelineInfo *pipeline_info);

void sy_render_create_pipeline_resources(SyRenderInfo *render_info, SyRenderPipelineInfo *pipeline_info)
{
    create_render_pass(render_info, pipeline_info);
    create_descriptor_set_layout(render_info, pipeline_info);


    SY_OUTPUT_INFO("Created render pipeline resources.")
}

void sy_render_destroy_pipeline_resources(SyRenderInfo *render_info, SyRenderPipelineInfo *pipeline_info)
{



    vkDestroyDescriptorSetLayout(render_info->logical_device, pipeline_info->descriptor_set_layout, NULL);
    vkDestroyRenderPass(render_info->logical_device, pipeline_info->render_pass, NULL);

    SY_OUTPUT_INFO("Destroyed render pipeline resources.")
}

void create_pipeline(SyRenderInfo *render_info, SyRenderPipelineInfo *pipeline_info)
{
    // Creating Shaders

    // vertex shader
    const char *vertex_shader_path = "shaders/vertex.spv";
    const char *fragment_shader_path = "shaders/fragment.spv";

    char *vertex_shader_code = NULL;
    ssize_t vertex_shader_code_size = get_file_contents(vertex_shader_path, &vertex_shader_code);
    SY_ERROR_COND(vertex_shader_code_size == -1, "PIPELINE: Failed to read file \"%s\".\n", vertex_shader_path);
    // This is freed at the end of this function
    VkShaderModule vertex_shader_module = create_shader_module(vk_info, vertex_shader_code, vertex_shader_code_size - 1);
    free(vertex_shader_code);

    VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info;
    vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_create_info.pNext = NULL;
    vertex_shader_stage_create_info.flags = 0;
    vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_create_info.module = vertex_shader_module;
    vertex_shader_stage_create_info.pName = "main";
    vertex_shader_stage_create_info.pSpecializationInfo = NULL; // specialization constants (uniforms???)
    
    // fragment shader
    char *fragment_shader_code = NULL;
    ssize_t fragment_shader_code_size = get_file_contents(fragment_shader_path, &fragment_shader_code);
    SY_ERROR_COND(fragment_shader_code_size == -1, "PIPELINE: Failed to read file \"%s\".\n", fragment_shader_path);
    // This is freed at the end of this function
    VkShaderModule fragment_shader_module = create_shader_module(vk_info, fragment_shader_code, fragment_shader_code_size - 1);
    free(fragment_shader_code);

    VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info;
    fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_create_info.pNext = NULL;
    fragment_shader_stage_create_info.flags = 0;
    fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_create_info.module = fragment_shader_module;
    fragment_shader_stage_create_info.pName = "main";
    fragment_shader_stage_create_info.pSpecializationInfo = NULL; // specialization constants (uniforms???

    VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_shader_stage_create_info, fragment_shader_stage_create_info};

    // Fixed Functions

    // dynamic state
    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info;
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.pNext = NULL;
    dynamic_state_create_info.flags = 0;
    dynamic_state_create_info.dynamicStateCount = SY_ARRLEN(dynamic_states);
    dynamic_state_create_info.pDynamicStates = dynamic_states;

    // vertex input bindings
    VkVertexInputBindingDescription binding_descriptions[] =  {vertex_get_binding_description()};
    uint32_t attr_descriptions_amt;
    vertex_get_attribute_descriptions(NULL, &attr_descriptions_amt);
    VkVertexInputAttributeDescription *attr_descriptions = calloc(attr_descriptions_amt, sizeof(VkVertexInputAttributeDescription));
    vertex_get_attribute_descriptions(attr_descriptions, &attr_descriptions_amt);

    VkPipelineVertexInputStateCreateInfo vertex_input_create_info;
    vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_create_info.pNext = NULL;
    vertex_input_create_info.flags = 0;
    vertex_input_create_info.vertexBindingDescriptionCount = SY_ARRLEN(binding_descriptions);
    vertex_input_create_info.pVertexBindingDescriptions = binding_descriptions;
    vertex_input_create_info.vertexAttributeDescriptionCount = attr_descriptions_amt;
    vertex_input_create_info.pVertexAttributeDescriptions = attr_descriptions;

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
    input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.pNext = NULL;
    input_assembly_create_info.flags = 0;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // render a list of triangles
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE; // some random EBO shit

    // viewport
    VkViewport viewport;
    viewport.width = render_info->swapchain_image_extent.width;
    viewport.height = render_info->swapchain_image_extent.height;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.minDepth = 0.0f;

    // scissor
    VkRect2D scissor;
    scissor.extent = render_info->swapchain_image_extent;
    VkOffset2D offset;
    offset.x = 0;
    offset.y = 0;
    scissor.offset = offset;

    // viewport state
    VkPipelineViewportStateCreateInfo viewport_state_create_info;
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.pNext = NULL;
    viewport_state_create_info.flags = 0;
    viewport_state_create_info.viewportCount = 1; // Eventhough dynamic state they still need count
    viewport_state_create_info.scissorCount = 1;  // Eventhough dynamic state they still need count
    viewport_state_create_info.pViewports = NULL; // This is because viewport is dynamic state
    viewport_state_create_info.pScissors = NULL;  // This is because scissor is dynamic state

    // rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer_create_info;
    rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_create_info.pNext = NULL;
    rasterizer_create_info.flags = 0;
    rasterizer_create_info.depthClampEnable = VK_FALSE; // This field clamps if object is beyond near and far plane
    rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE; // This field disables going beyond rasterization
    rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL; // This is like wireframe mode, vertice mode, fill mode
    rasterizer_create_info.lineWidth = 1.0f; // self explanatory
//    rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT; // what faces do you want to cull
    rasterizer_create_info.cullMode = VK_CULL_MODE_NONE;
    rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE; // defines what order faces go in, needed for face culling
    rasterizer_create_info.depthBiasEnable = VK_FALSE; // adds a number to the depth
    rasterizer_create_info.depthClampEnable = VK_FALSE; // clamps the depth
    rasterizer_create_info.depthBiasConstantFactor = 0.0f;
    rasterizer_create_info.depthBiasClamp = 0.0f;
    rasterizer_create_info.depthBiasSlopeFactor = 0.0f;

    // multisampling - read the text that goes with this in the tutorial, it's pretty interesting
    VkPipelineMultisampleStateCreateInfo multisample_create_info;
    multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_create_info.pNext = NULL;
    multisample_create_info.flags = 0;
    multisample_create_info.sampleShadingEnable = VK_FALSE;
    multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_create_info.minSampleShading = 1.0f;
    multisample_create_info.pSampleMask = NULL;
    multisample_create_info.alphaToCoverageEnable = VK_FALSE;
    multisample_create_info.alphaToOneEnable = VK_FALSE;

    /* The tutorial doesn't use a depth and stencil buffer so we will pass NULL later on
    // depth and stencil testing
    VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info;
    depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_create_info.pNext = NULL;
    depth_stencil_create_info.flags = 0; */

    // color blending
    /* NOTE:
     * VkPipelineColorBlendAttachmentState contains the configuration per attached frambuffer
     * VkPipelineColorBlendStateCreateInfo contains the global color blending settings */
    const int attachments_amt = 1;
    VkPipelineColorBlendAttachmentState color_blend_attachments[attachments_amt];
    color_blend_attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachments[0].blendEnable = VK_FALSE;
    color_blend_attachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachments[0].colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachments[0].alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blend_create_info;
    color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_create_info.pNext = NULL;
    color_blend_create_info.flags = 0;
    color_blend_create_info.logicOpEnable = VK_FALSE;
    color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_create_info.attachmentCount = attachments_amt;
    color_blend_create_info.pAttachments = color_blend_attachments;
    color_blend_create_info.blendConstants[0] = 0.0f;
    color_blend_create_info.blendConstants[1] = 0.0f;
    color_blend_create_info.blendConstants[2] = 0.0f;
    color_blend_create_info.blendConstants[3] = 0.0f;

    // pipeline layout - this is where you specify your uniforms and their layout (pos 0: 16 bit, pos1: 64 bit, pos2: 32bit)
    VkPipelineLayoutCreateInfo layout_create_info;
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.pNext = NULL;
    layout_create_info.flags = 0;
    layout_create_info.setLayoutCount = 1;
    layout_create_info.pSetLayouts = &pipeline_info->descriptor_set_layout;
    layout_create_info.pushConstantRangeCount = 0;
    layout_create_info.pPushConstantRanges = NULL;

    SY_ERROR_COND(vkCreatePipelineLayout(render_info->logical_device, &layout_create_info, NULL, &pipeline_info->pipeline_layout) != VK_SUCCESS,
		  "PIPELINE: Failed to create a pipeline layout.");

    VkGraphicsPipelineCreateInfo pipeline_create_info;
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.pNext = NULL;
    pipeline_create_info.flags = 0;
    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pStages = shader_stages;
    pipeline_create_info.pVertexInputState = &vertex_input_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasterizer_create_info;
    pipeline_create_info.pMultisampleState = &multisample_create_info;
    pipeline_create_info.pDepthStencilState = NULL;
    pipeline_create_info.pColorBlendState = &color_blend_create_info;
    pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    pipeline_create_info.layout = pipeline_info->pipeline_layout;
    pipeline_create_info.renderPass = pipeline_info->render_pass;
    pipeline_create_info.subpass = 0; // index of subpass where this pipeline will be used
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = -1;

    SY_ERROR_COND(vkCreateGraphicsPipelines(render_info->logical_device, VK_NULL_HANDLE, 1, &pipeline_create_info, NULL, &pipeline_info->pipeline) != VK_SUCCESS, "PIPELINE: Failed to create graphics pipeline.");

    // Cleanup
    free(attr_descriptions);
    vkDestroyShaderModule(render_info->logical_device, vertex_shader_module, NULL);
    vkDestroyShaderModule(render_info->logical_device, fragment_shader_module, NULL);

}

void create_descriptor_set_layout(SyRenderInfo *render_info, SyRenderPipelineInfo *pipeline_info)
{
    VkDescriptorSetLayoutBinding ubo_layout_binding;
    ubo_layout_binding.binding = 0; // the binding of the uniform inside of the shader
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = NULL; // for image sampling
    
    VkDescriptorSetLayoutCreateInfo layout_create_info;
    layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_create_info.pNext = NULL;
    layout_create_info.flags = 0;
    layout_create_info.bindingCount = 1;
    layout_create_info.pBindings = &ubo_layout_binding;

    SY_ERROR_COND(vkCreateDescriptorSetLayout(render_info->logical_device, &layout_create_info, NULL, &pipeline_info->descriptor_set_layout) != VK_SUCCESS, "PIPELINE: Failed to create descriptor set layout.");

}

void create_render_pass(SyRenderInfo *render_info, SyRenderPipelineInfo *pipeline_info)
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

    // finish executing color attachment on image conversion before going through with color attachment on subpass 0
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    subpass_dependency.srcAccessMask = 0;
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

    SY_ERROR_COND(vkCreateRenderPass(render_info->logical_device, &render_pass_create_info, NULL, &pipeline_info->render_pass) != VK_SUCCESS, "PIPELINE: Failed to create render pass.");

}

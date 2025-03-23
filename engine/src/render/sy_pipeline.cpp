#include "sy_pipeline.hpp"

#include <stdlib.h>

#include "render/sy_render_info.hpp"
#include "sy_macros.hpp"
#include "sy_utils.hpp"
#include "sy_buffer.hpp"

VkShaderModule create_shader_module(SyRenderInfo *render_info, char *code, ssize_t code_size);

VkPipeline sy_render_create_pipeline(SyRenderInfo *render_info, SyPipelineCreateInfo *pipeline_create_info)
{

    VkPipeline result;

    // Creating Shaders

    // vertex shader
    size_t vertex_shader_code_size;
    char *vertex_shader_code = sy_read_resource_file(pipeline_create_info->vertex_shader_path, &vertex_shader_code_size);
    SY_ERROR_COND(vertex_shader_code == NULL, "PIPELINE: couldn't read resources file %s", pipeline_create_info->vertex_shader_path);

    // This is freed at the end of this function
    VkShaderModule vertex_shader_module = create_shader_module(render_info, vertex_shader_code, vertex_shader_code_size);
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
    size_t fragment_shader_code_size;
    char *fragment_shader_code = sy_read_resource_file(pipeline_create_info->fragment_shader_path, &fragment_shader_code_size);
    SY_ERROR_COND(fragment_shader_code == NULL, "PIPELINE: couldn't read resources file %s", pipeline_create_info->fragment_shader_path);

    // This is freed at the end of this function
    VkShaderModule fragment_shader_module = create_shader_module(render_info, fragment_shader_code, fragment_shader_code_size);
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
    VkVertexInputBindingDescription binding_descriptions[] =  {pipeline_create_info->vertex_input_binding_description};
    uint32_t attr_descriptions_amt = pipeline_create_info->vertex_input_attribute_descriptions_amt;
    VkVertexInputAttributeDescription *attr_descriptions = pipeline_create_info->vertex_input_attribute_descriptions;

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
    input_assembly_create_info.topology = pipeline_create_info->render_type;
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

    // The tutorial doesn't use a depth and stencil buffer so we will pass NULL later on
    // depth and stencil testing
    // VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info;
    // depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // depth_stencil_create_info.pNext = NULL;
    // depth_stencil_create_info.flags = 0;

    // color blending
    // NOTE:
    // VkPipelineColorBlendAttachmentState contains the configuration per attached frambuffer
    // VkPipelineColorBlendStateCreateInfo contains the global color blending settings
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

    VkGraphicsPipelineCreateInfo vulkan_pipeline_create_info;
    vulkan_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vulkan_pipeline_create_info.pNext = NULL;
    vulkan_pipeline_create_info.flags = 0;
    vulkan_pipeline_create_info.stageCount = SY_ARRLEN(shader_stages);
    vulkan_pipeline_create_info.pStages = shader_stages;
    vulkan_pipeline_create_info.pVertexInputState = &vertex_input_create_info;
    vulkan_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    vulkan_pipeline_create_info.pViewportState = &viewport_state_create_info;
    vulkan_pipeline_create_info.pRasterizationState = &rasterizer_create_info;
    vulkan_pipeline_create_info.pMultisampleState = &multisample_create_info;
    vulkan_pipeline_create_info.pDepthStencilState = NULL;
    vulkan_pipeline_create_info.pColorBlendState = &color_blend_create_info;
    vulkan_pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    vulkan_pipeline_create_info.layout = pipeline_create_info->pipeline_layout;
    vulkan_pipeline_create_info.renderPass = render_info->render_pass;
    vulkan_pipeline_create_info.subpass = pipeline_create_info->subpass_number; // index of subpass where this pipeline will be used
    vulkan_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    vulkan_pipeline_create_info.basePipelineIndex = -1;

    SY_ERROR_COND(vkCreateGraphicsPipelines(render_info->logical_device, VK_NULL_HANDLE, 1, &vulkan_pipeline_create_info, NULL, &result) != VK_SUCCESS, "PIPELINE: Failed to create graphics pipeline.");

    // Cleanup
    vkDestroyShaderModule(render_info->logical_device, vertex_shader_module, NULL);
    vkDestroyShaderModule(render_info->logical_device, fragment_shader_module, NULL);
    
    return result;
}

VkShaderModule create_shader_module(SyRenderInfo *render_info, char *code, ssize_t code_size)
{
    VkShaderModuleCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.codeSize = code_size;
    create_info.pCode = (uint32_t*)code;
    
    VkShaderModule shader_module;
    SY_ERROR_COND(vkCreateShaderModule(render_info->logical_device, &create_info, NULL, &shader_module) != VK_SUCCESS,
		  "PIPELINE: Couldn't make shader module.");

    return shader_module;
}

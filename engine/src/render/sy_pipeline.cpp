#include "sy_pipeline.hpp"

#include <stdlib.h>

#include "render/sy_render_info.hpp"
#include "sy_macros.hpp"
#include "sy_utils.hpp"
#include "sy_buffer.hpp"

SyPipeline create_pipeline_object(SyRenderInfo *render_info, SyPipelineCreateInfo *pipeline_create_info);
VkShaderModule create_shader_module(SyRenderInfo *render_info, char *code, ssize_t code_size);
void create_descriptor_pool(SyRenderInfo *render_info, SyPipeline *pipeline);
void create_uniforms(SyRenderInfo *render_info, SyPipeline *pipeline, size_t ubo_size);
void create_descriptor_sets(SyRenderInfo *render_info, SyPipeline *pipeline, VkDeviceSize uniform_size);

SyPipeline sy_render_create_pipeline(SyRenderInfo *render_info, SyPipelineCreateInfo *pipeline_create_info)
{
    SyPipeline result = create_pipeline_object(render_info, pipeline_create_info);
    // create_uniforms(render_info, &result, pipeline_create_info->ubo_size);
    // create_descriptor_pool(render_info, &result);
    // create_descriptor_sets(render_info, &result, sizeof(float)); // FIXME: change from sizeof(int)

    return result;
}

void sy_render_destroy_pipeline(SyRenderInfo *render_info, SyPipeline *pipeline)
{
    // vkDestroyDescriptorPool(render_info->logical_device, pipeline->descriptor_pool, NULL);

    // for (size_t i = 0; i < render_info->max_frames_in_flight; ++i)
    // {
    // 	vkDestroyBuffer(render_info->logical_device, pipeline->uniform_buffers[i], NULL);
    // 	vkFreeMemory(render_info->logical_device, pipeline->uniform_buffers_memory[i], NULL);
    // }
    // free(pipeline->uniform_buffers);
    // free(pipeline->uniform_buffers_memory);
    // free(pipeline->uniform_buffers_mapped);

    vkDestroyPipelineLayout(render_info->logical_device, pipeline->pipeline_layout, NULL);
    
    vkDestroyPipeline(render_info->logical_device, pipeline->pipeline, NULL);

}

// void create_descriptor_sets(SyRenderInfo *render_info, SyPipeline *pipeline, VkDeviceSize uniform_size)
// {
//     // Allocate descriptor sets

//     // Initialize layouts array with every member set to vk_info->descriptor_set_layout
//     uint32_t layout_amt = render_info->max_frames_in_flight;
//     VkDescriptorSetLayout *layouts = (VkDescriptorSetLayout*)calloc(layout_amt, sizeof(VkDescriptorSetLayout));
//     for (int i = 0; i < layout_amt; ++i)
//     {
// 	layouts[i] = render_info->single_ubo_descriptor_set_layout;
//     }

//     VkDescriptorSetAllocateInfo allocate_info;
//     allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//     allocate_info.pNext = NULL;
//     allocate_info.pSetLayouts = layouts;
//     allocate_info.descriptorSetCount = layout_amt;
//     allocate_info.descriptorPool = pipeline->descriptor_pool;

//     pipeline->descriptor_sets_amt = render_info->max_frames_in_flight;
//     pipeline->descriptor_sets = (VkDescriptorSet*)calloc(pipeline->descriptor_sets_amt, sizeof(VkDescriptorSet));
    
//     SY_ERROR_COND(vkAllocateDescriptorSets(render_info->logical_device, &allocate_info, pipeline->descriptor_sets) != VK_SUCCESS, "PIPELINE: Failed to allocate descriptor sets.");

//     // Populate every descriptor set
//     for (size_t i = 0; i < pipeline->descriptor_sets_amt; ++i)
//     {
// 	VkDescriptorBufferInfo buffer_info;
// 	buffer_info.buffer = pipeline->uniform_buffers[i];
// 	buffer_info.offset = 0;
// 	buffer_info.range = uniform_size;

// 	VkWriteDescriptorSet descriptor_write;
// 	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
// 	descriptor_write.pNext = NULL;
// 	descriptor_write.dstSet = pipeline->descriptor_sets[i];
// 	descriptor_write.dstBinding = 0;
// 	descriptor_write.dstArrayElement = 0;
// 	descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
// 	descriptor_write.descriptorCount = 1;
// 	descriptor_write.pBufferInfo = &buffer_info;
// 	descriptor_write.pImageInfo = NULL;
// 	descriptor_write.pTexelBufferView = NULL;

// 	vkUpdateDescriptorSets(render_info->logical_device, 1, &descriptor_write, 0, NULL);
//     }

//     // Cleanup
//     free(layouts);
// }

// void create_descriptor_pool(SyRenderInfo *render_info, SyPipeline *pipeline)
// {
//     VkDescriptorPoolSize pool_size;
//     pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     pool_size.descriptorCount = render_info->max_frames_in_flight;

//     VkDescriptorPoolCreateInfo pool_create_info;
//     pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//     pool_create_info.pNext = NULL;
//     pool_create_info.flags = 0;
//     pool_create_info.poolSizeCount = 1;
//     pool_create_info.pPoolSizes = &pool_size;
//     pool_create_info.maxSets = render_info->max_frames_in_flight;

//     SY_ERROR_COND(vkCreateDescriptorPool(render_info->logical_device, &pool_create_info, NULL, &pipeline->descriptor_pool) != VK_SUCCESS, "PIPELINE: Failed to create descriptor pool.");
// }

// void create_uniforms(SyRenderInfo *render_info, SyPipeline *pipeline, size_t ubo_size)
// {
//     pipeline->uniform_buffers_amt = render_info->max_frames_in_flight;
//     pipeline->uniform_buffers = (VkBuffer*)calloc(pipeline->uniform_buffers_amt, sizeof(VkBuffer));
//     pipeline->uniform_buffers_memory = (VkDeviceMemory*)calloc(pipeline->uniform_buffers_amt, sizeof(VkDeviceMemory));
//     pipeline->uniform_buffers_mapped = (void**)calloc(pipeline->uniform_buffers_amt, sizeof(void*));

//     for (size_t i = 0; i < render_info->max_frames_in_flight; ++i)
//     {
// 	sy_create_buffer(render_info, ubo_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &pipeline->uniform_buffers[i], &pipeline->uniform_buffers_memory[i]);

// 	SY_ERROR_COND(vkMapMemory(render_info->logical_device, pipeline->uniform_buffers_memory[i], 0, ubo_size, 0, &pipeline->uniform_buffers_mapped[i]) != VK_SUCCESS, "PIPELINE: Failed to map uniform buffer %lu", i);
//     }

// }

SyPipeline create_pipeline_object(SyRenderInfo *render_info, SyPipelineCreateInfo *pipeline_create_info)
{

    SyPipeline result;

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

    // pipeline layout - this is where you specify your uniforms and their layout (pos 0: 16 bit, pos1: 64 bit, pos2: 32bit)
    VkPipelineLayoutCreateInfo layout_create_info;
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.pNext = NULL;
    layout_create_info.flags = 0;
    layout_create_info.setLayoutCount = 0;
    layout_create_info.pSetLayouts = NULL;
    // layout_create_info.setLayoutCount = pipeline_create_info->descriptor_set_layouts_amt;
    // layout_create_info.pSetLayouts = pipeline_create_info->descriptor_set_layouts;
    layout_create_info.pushConstantRangeCount = 0;
    layout_create_info.pPushConstantRanges = NULL;

    SY_ERROR_COND(vkCreatePipelineLayout(render_info->logical_device, &layout_create_info, NULL, &result.pipeline_layout) != VK_SUCCESS,
		  "PIPELINE: Failed to create a pipeline layout.");

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
    vulkan_pipeline_create_info.layout = result.pipeline_layout;
    vulkan_pipeline_create_info.renderPass = render_info->render_pass;
    vulkan_pipeline_create_info.subpass = pipeline_create_info->subpass_number; // index of subpass where this pipeline will be used
    vulkan_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    vulkan_pipeline_create_info.basePipelineIndex = -1;

    SY_ERROR_COND(vkCreateGraphicsPipelines(render_info->logical_device, VK_NULL_HANDLE, 1, &vulkan_pipeline_create_info, NULL, &result.pipeline) != VK_SUCCESS, "PIPELINE: Failed to create graphics pipeline.");

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

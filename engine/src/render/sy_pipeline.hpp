#pragma once

#include "sy_render_settings.hpp"

#include "sy_render_info.hpp"
#include <vulkan/vulkan_core.h>


struct SyPipelineCreateInfo
{
    const char *vertex_shader_path;
    const char *fragment_shader_path;
// TODO   const char *geometry_shader_path;
// TODO   bool has_geometry_shader;
    
    VkVertexInputBindingDescription vertex_input_binding_description;
    VkVertexInputAttributeDescription *vertex_input_attribute_descriptions;
    uint32_t vertex_input_attribute_descriptions_amt;
    
    VkPrimitiveTopology render_type;

    uint32_t subpass_number;
    
    VkPipelineLayout pipeline_layout;
};



VkPipeline sy_render_create_pipeline(SyRenderInfo *render_info, SyPipelineCreateInfo *pipeline_create_info);
void sy_render_destroy_pipeline(SyRenderInfo *render_info, VkPipeline *pipeline);

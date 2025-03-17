#pragma once

#include "sy_render_settings.hpp"

#include "sy_render_info.hpp"
#include <vulkan/vulkan_core.h>


struct SyPipelineCreateInfo
{
    const char *vertex_shader_path;
    const char *fragment_shader_path;
// TODO    const char *geometry_shader_path;
// TODO   bool has_geometry_shader;
    
    VkVertexInputBindingDescription vertex_input_binding_description;
    VkVertexInputAttributeDescription *vertex_input_attribute_descriptions;
    uint32_t vertex_input_attribute_descriptions_amt;
    
    VkPrimitiveTopology render_type;
    
    VkDescriptorSetLayout *descriptor_set_layouts;
    uint32_t descriptor_set_layouts_amt;
    
    uint32_t subpass_number;
    
};


struct SyPipeline
{
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;

    uint32_t uniform_buffers_amt;
    VkBuffer *uniform_buffers;
    VkDeviceMemory *uniform_buffers_memory;
    void **uniform_buffers_mapped;

    VkDescriptorPool descriptor_pool;
    VkDescriptorSet *camera_descriptor_sets;
};


SyPipeline sy_render_create_pipeline(SyRenderInfo *render_info, SyPipelineCreateInfo *pipeline_create_info);

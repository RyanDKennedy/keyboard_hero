#pragma once

#include "sy_render_settings.hpp"

#include "sy_render_info.hpp"

struct SyRenderPipelineInfo
{
    VkRenderPass render_pass;

    VkDescriptorSetLayout descriptor_set_layout;

    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;

    uint32_t uniform_buffers_amt;
    VkBuffer *uniform_buffers;
    VkDeviceMemory *uniform_buffers_memory;
    void **uniform_buffers_mapped;


    uint32_t descriptor_sets_amt;
    VkDescriptorSet *descriptor_sets;

};

void sy_render_create_pipeline_resources(SyRenderInfo *render_info, SyRenderPipelineInfo *pipeline_info);

void sy_render_destroy_pipeline_resources(SyRenderInfo *render_info, SyRenderPipelineInfo *pipeline_info);


#pragma once

#include "render/sy_render_defines.hpp"
#include "render/sy_render_settings.hpp" // this needs to be first because it sets up some #define's

#include "render/sy_image.hpp"

#include <vulkan/vulkan.h>
#include "render/types/sy_mesh.hpp"
#include "vk_mem_alloc.h"

#include "glm_include.hpp"

#include "sy_frame_uniform_data_info.hpp"


struct SyRenderInfo
{
    VmaAllocator vma_allocator;
    
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physical_device;
    uint32_t present_queue_family_index;
    uint32_t graphics_queue_family_index;

    VkDevice logical_device;
    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSwapchainKHR swapchain;
    VkImage *swapchain_images;
    uint32_t swapchain_images_amt;
    VkExtent2D swapchain_image_extent;
    VkFormat swapchain_image_format;
    VkImageView *swapchain_image_views;
    uint32_t swapchain_image_views_amt;

    VkFramebuffer *swapchain_framebuffers;
    uint32_t swapchain_framebuffers_amt;

    VkCommandPool command_pool;

    uint32_t command_buffers_amt;
    VkCommandBuffer *command_buffers;

    // The render pass
    VkRenderPass render_pass;

    VkSemaphore *image_available_semaphores; // array of size SY_RENDER_MAX_FRAMES_IN_FLIGHT
    VkSemaphore *render_finished_semaphores; // array of size SY_RENDER_MAX_FRAMES_IN_FLIGHT
    VkFence *in_flight_fences; // array of size SY_RENDER_MAX_FRAMES_IN_FLIGHT

    uint32_t current_frame; // 0 - SY_RENDER_MAX_FRAMES_IN_FLIGHT

    // Uniforms
    SyFrameUniformDataInfo *frame_uniform_data; // array of size SY_RENDER_MAX_FRAMES_IN_FLIGHT

    // Depth
    // depth images amt is same as swapchain_images_amt;
    VkImage *depth_images;
    VmaAllocation *depth_image_allocations;
    VkImageView *depth_image_views;
    VkFormat depth_format;

    VkDescriptorSetLayout frame_descriptor_set_layout;
    VkDescriptorSetLayout material_descriptor_set_layout;
    VkDescriptorSetLayout object_descriptor_set_layout;
    VkPipelineLayout single_color_pipeline_layout;
    VkPipeline single_color_pipeline;

    VkDescriptorSetLayout character_map_descriptor_set_layout;
    VkDescriptorSetLayout character_information_descriptor_set_layout;
    VkDescriptorSetLayout text_buffer_descriptor_set_layout;
    VkPipelineLayout text_pipeline_layout;
    VkPipeline text_pipeline;

    // FIXME:
    SyRenderImage error_image;
    VkSampler font_sampler;
    SyMesh error_image_mesh;
    VkBuffer storage_buffer[SY_RENDER_MAX_FRAMES_IN_FLIGHT];
    VmaAllocation storage_buffer_allocation[SY_RENDER_MAX_FRAMES_IN_FLIGHT];
    uint32_t character_amt;
    size_t storage_buffer_size;
    
};

void sy_render_info_init(SyRenderInfo *render_info, int win_width, int win_height);
void sy_render_info_deinit(SyRenderInfo *render_info);



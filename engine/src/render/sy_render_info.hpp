#pragma once

#include "render/sy_render_settings.hpp" // this needs to be first because it sets up some #define's

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

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

    // descriptors
    VkDescriptorSetLayout frame_data_descriptor_set_layout;

    // The render pass
    VkRenderPass render_pass;

    VkSemaphore *image_available_semaphores;
    VkSemaphore *render_finished_semaphores;
    VkFence *in_flight_fences;

    int max_frames_in_flight;
    uint32_t current_frame;

    VkPipelineLayout single_color_pipeline_layout;
    VkPipeline single_color_pipeline;
};

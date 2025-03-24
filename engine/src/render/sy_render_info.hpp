#pragma once

#include "render/sy_render_settings.hpp" // this needs to be first because it sets up some #define's

#include "sy_descriptor_set_data.hpp"

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include "sy_render_defines.hpp"

#include "glm_include.hpp"

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

    VkSemaphore *image_available_semaphores; // array of size max_frames_in_flight
    VkSemaphore *render_finished_semaphores; // array of size max_frames_in_flight
    VkFence *in_flight_fences; // array of size max_frames_in_flight

    const int max_frames_in_flight = SY_RENDER_MAX_FRAMES_IN_FLIGHT;
    uint32_t current_frame;

    VkDescriptorSetLayout frame_descriptor_set_layout;
    VkDescriptorSetLayout material_descriptor_set_layout;
    VkDescriptorSetLayout object_descriptor_set_layout;

    VkPipelineLayout single_color_pipeline_layout;
    VkPipeline single_color_pipeline;


    VkDescriptorPool descriptor_pool;
    const size_t max_descriptor_sets_amt = 20;
    SyDescriptorSetDataGroup *descriptor_sets;
    bool *descriptor_sets_used;

    int frame_descriptor_index;

    // FIXME:
    glm::vec3 pos;
    glm::vec3 rot;

};

// FIXME:
struct SyFrameData
{
    glm::mat4 vp_matrix;
};

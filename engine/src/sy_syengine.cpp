#include "sy_syengine.hpp"

#include <stdio.h>
#include <vulkan/vulkan_core.h>

#include "render/sy_buffer.hpp"
#include "render/sy_drawing.hpp"
#include "sy_ecs.hpp"
#include "sy_macros.hpp"

#include "render/sy_render.hpp"
#include "render/sy_resources.hpp"
#include "render/sy_physical_device.hpp"
#include "render/sy_logical_device.hpp"
#include "render/sy_swapchain.hpp"
#include "sy_utils.hpp"

#ifdef NDEBUG
extern "C"
void app_init(SyAppInfo *app_info);

extern "C"
void app_run(SyAppInfo *app_info);

extern "C"
void app_destroy(SyAppInfo *app_info);
#endif

void renderer_init(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    /* ORDER FOR RENDER SYSTEM INIT
       1. create descriptor set layouts
       1. create render pass
       2. create framebuffers
       2. create pipelines
	  1. create uniform buffers
          2. create descriptor sets

     */
    
    platform_info->render_info.max_frames_in_flight = 2;
    sy_render_create_physical_device(&platform_info->render_info);
    {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(platform_info->render_info.physical_device, &props);
	SY_OUTPUT_INFO("using device %s", props.deviceName);
    }
    sy_render_create_logical_device(&platform_info->render_info);
    sy_render_create_swapchain(&platform_info->render_info, platform_info->input_info.window_width, platform_info->input_info.window_height);
    sy_render_create_command_pool(&platform_info->render_info);
    // sy_render_create_descriptor_set_layouts(&platform_info->render_info);
    platform_info->render_info.render_pass = sy_render_create_simple_render_pass(&platform_info->render_info);
    sy_render_create_swapchain_framebuffers(&platform_info->render_info);
    sy_render_create_command_buffers(&platform_info->render_info);
    sy_render_create_sync_objects(&platform_info->render_info);
    

    { // Create single color pipeline
	VkVertexInputBindingDescription binding_description;
	binding_description.binding = 0;
	binding_description.stride = sizeof(float) * 2;
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;    

	VkVertexInputAttributeDescription attr[1];
	attr[0].binding = 0;
	attr[0].location = 0;
	attr[0].format = VK_FORMAT_R32G32_SFLOAT;
	attr[0].offset = 0;

	SyPipelineCreateInfo create_info;
	create_info.vertex_shader_path = "single_color/vertex.spv";
	create_info.fragment_shader_path = "single_color/fragment.spv";
	create_info.vertex_input_binding_description = binding_description;
	create_info.vertex_input_attribute_descriptions = attr;
	create_info.vertex_input_attribute_descriptions_amt = 1;
	create_info.render_type = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	create_info.subpass_number = 0;
	// create_info.descriptor_set_layouts = &platform_info->render_info.single_ubo_descriptor_set_layout;
	// create_info.descriptor_set_layouts = VK_NULL_HANDLE;
	// create_info.descriptor_set_layouts_amt = 0;
	// create_info.ubo_size = sizeof(float) * 1;
	// create_info.ubo_size = sizeof(float) * 1;

	platform_info->pipeline = sy_render_create_pipeline(&platform_info->render_info, &create_info);
    }

    // init render types in ecs
    sy_render_init_ecs(&app_info->ecs);

    // FIXME:
    float vertex_data[] =
	{
	    -0.5f, -0.5f,
	    0.5f, -0.5f,
	    0.5f, 0.5f,
	    -0.5f, 0.5f
	};
    sy_render_create_vertex_buffer(&platform_info->render_info, SY_ARRLEN(vertex_data), sizeof(float) * 2, vertex_data, &platform_info->render_info.vertex_buffer, &platform_info->render_info.vertex_buffer_memory);

    uint32_t index_data[] =
	{
	    0, 3, 2, 2, 1, 0
	};
    platform_info->render_info.index_amt = SY_ARRLEN(index_data);
    sy_render_create_index_buffer(&platform_info->render_info, SY_ARRLEN(index_data), index_data, &platform_info->render_info.index_buffer, &platform_info->render_info.index_buffer_memory);

}

void renderer_cleanup(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{

    sy_render_destroy_pipeline(&platform_info->render_info, &platform_info->pipeline);

    free(platform_info->render_info.command_buffers);

    for (int i = 0; i < platform_info->render_info.max_frames_in_flight; ++i)
    {
	vkDestroySemaphore(platform_info->render_info.logical_device, platform_info->render_info.image_available_semaphores[i], NULL);
	vkDestroySemaphore(platform_info->render_info.logical_device, platform_info->render_info.render_finished_semaphores[i], NULL);
	vkDestroyFence(platform_info->render_info.logical_device, platform_info->render_info.in_flight_fences[i], NULL);
    }
    free(platform_info->render_info.image_available_semaphores);
    free(platform_info->render_info.render_finished_semaphores);
    free(platform_info->render_info.in_flight_fences);
    
    for (int i = 0; i < platform_info->render_info.swapchain_framebuffers_amt; ++i)
    {
	vkDestroyFramebuffer(platform_info->render_info.logical_device, platform_info->render_info.swapchain_framebuffers[i], NULL);
    }
    free(platform_info->render_info.swapchain_framebuffers);

    vkDestroyRenderPass(platform_info->render_info.logical_device, platform_info->render_info.render_pass, NULL);
    // vkDestroyDescriptorSetLayout(platform_info->render_info.logical_device, platform_info->render_info.single_ubo_descriptor_set_layout, NULL);

    vkDestroyCommandPool(platform_info->render_info.logical_device, platform_info->render_info.command_pool, NULL); // command buffers are freed when command pool is freed

    sy_render_destroy_swapchain(&platform_info->render_info);
    vkDestroyDevice(platform_info->render_info.logical_device, NULL);

    
}


void engine_init(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    SY_OUTPUT_INFO("Starting Engine");

    app_info->stop_game = false;

    // ECS Init
    app_info->ecs.initialize();

    // Arena/Allocation Init
    SY_ERROR_COND(app_info->persistent_arena.initialize(4096) != 0, "Failed to allocate data for persistent arena.");
    SY_ERROR_COND(app_info->frame_arena.initialize(4096) != 0, "Failed to allocate data for frame arena.");
    app_info->global_mem_size = 2048;
    app_info->global_mem = app_info->persistent_arena.alloc(app_info->global_mem_size);

    app_info->delta_time = 0.0;

    renderer_init(platform_info, app_info);

#ifndef NDEBUG
    platform_info->app_init(app_info);
#else
    app_init(app_info);
#endif


    // stop the game signal
    if (app_info->stop_game == true)
    {
	platform_info->end_engine = true;
    }
}

void engine_run(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    app_info->frame_arena.free_all();
    app_info->input_info = platform_info->input_info;
    app_info->delta_time = (double)platform_info->delta_time / 1000000.0;

    SyInputInfo &input = app_info->input_info;

    // close the engine
    if (input.window_should_close)
    {
	platform_info->end_engine = true;
    }

#ifndef NDEBUG
    // THIS IS DLL HOT RELOAD STUFF, SO DON'T INCLUDE IN RELEASE VERSION
    
    // reload dlls for app
    if (input.f1)
    {
	platform_info->reload_dll = true;
    }


    // Run the app_dll_init function after dll has been reloaded 
    if (platform_info->dll_first_run == true)
    {
	platform_info->app_dll_init(app_info);
	platform_info->dll_first_run = false;
    }

    // run the app
    platform_info->app_run(app_info);

    // if the dll is about to be reloaded then run the app_dll_exit function
    if (platform_info->reload_dll == true)
    {
	platform_info->app_dll_exit(app_info);
    }
#else

    app_run(app_info);

#endif

    // NOTE: The input below this is stuff responding to data received by running the app

    // stop the game signal
    if (app_info->stop_game == true)
    {
	platform_info->end_engine = true;
    }

    sy_render_draw(&platform_info->render_info, &platform_info->pipeline);

}

void engine_destroy(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    vkDeviceWaitIdle(platform_info->render_info.logical_device);

    SY_OUTPUT_INFO("Ending Engine");

#ifndef NDEBUG
    platform_info->app_destroy(app_info);
#else
    app_destroy(app_info);
#endif

    renderer_cleanup(platform_info, app_info);


    { // Cleanup App Info
	// Arena/Allocation Cleanup
	app_info->persistent_arena.destroy();
	app_info->frame_arena.destroy();
    }

    app_info->ecs.destroy();
}

#include "sy_syengine.hpp"

#include <stdio.h>
#include <vulkan/vulkan_core.h>

#include "render/sy_buffer.hpp"
#include "render/sy_drawing.hpp"
#include "render/sy_render_info.hpp"
#include "render/types/sy_material.hpp"
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
    platform_info->render_info.current_frame = 0;
    sy_render_create_physical_device(&platform_info->render_info);
    {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(platform_info->render_info.physical_device, &props);
	SY_OUTPUT_INFO("using device %s", props.deviceName);
    }
    sy_render_create_logical_device(&platform_info->render_info);
    sy_render_create_swapchain(&platform_info->render_info, platform_info->input_info.window_width, platform_info->input_info.window_height);
    sy_render_create_command_pool(&platform_info->render_info);
    platform_info->render_info.render_pass = sy_render_create_simple_render_pass(&platform_info->render_info);
    sy_render_create_swapchain_framebuffers(&platform_info->render_info);
    sy_render_create_command_buffers(&platform_info->render_info);
    sy_render_create_sync_objects(&platform_info->render_info);
    sy_render_create_descriptor_set_layouts(&platform_info->render_info);

    VmaAllocatorCreateInfo vma_allocator_create_info = {};
    vma_allocator_create_info.flags = 0;
    vma_allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_0;
    vma_allocator_create_info.physicalDevice = platform_info->render_info.physical_device;
    vma_allocator_create_info.device = platform_info->render_info.logical_device;
    vma_allocator_create_info.instance = platform_info->render_info.instance;
    vma_allocator_create_info.pVulkanFunctions = NULL;
    vmaCreateAllocator(&vma_allocator_create_info, &platform_info->render_info.vma_allocator);

    sy_render_create_pipelines(&platform_info->render_info);

    // init render types in ecs
    sy_render_init_ecs(&app_info->ecs);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyMesh);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyMaterialComponent);

    sy_render_create_descriptor_pool(&platform_info->render_info);

    { // Create the frame data descriptor sets / uniform buffers
	SyFrameData frame_data = {};
	platform_info->render_info.frame_descriptor_index = sy_render_create_descriptor_set(&platform_info->render_info, sizeof(SyFrameData), &frame_data, platform_info->render_info.frame_descriptor_set_layout, 0);
    }

    size_t material_component_index;
    {
	int material_type_id = app_info->ecs.get_type_id<SyMaterialComponent>();
	material_component_index = app_info->ecs.get_unused_component<SyMaterialComponent>();
	SyMaterialComponent *material_comp = &app_info->ecs.m_component_data_arr[material_type_id].get<SyMaterialComponent>(material_component_index);

	material_comp->descriptor_set_index = -1;
	material_comp->material.ambient[0] = 0.0f;
	material_comp->material.ambient[1] = 0.0f;
	material_comp->material.ambient[2] = 0.0f;
	material_comp->material.diffuse = {0.0f, 1.0f, 1.0f};
	material_comp->material.specular[0] = 0.0f;
	material_comp->material.specular[1] = 0.0f;
	material_comp->material.specular[2] = 0.0f;
    }

    size_t mesh_component_index;
    {
	int mesh_type_id = app_info->ecs.get_type_id<SyMesh>();
	mesh_component_index = app_info->ecs.get_unused_component<SyMesh>();
	SyMesh *mesh_comp = &app_info->ecs.m_component_data_arr[mesh_type_id].get<SyMesh>(mesh_component_index);

	float vertex_data[] =
	    {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f
	    };
	
	uint32_t index_data[] =
	    {
		0, 3, 2, 2, 1, 0
	    };

	mesh_comp->index_amt = SY_ARRLEN(index_data);
	sy_render_create_vertex_buffer(&platform_info->render_info, SY_ARRLEN(vertex_data), sizeof(float) * 3, vertex_data, &mesh_comp->vertex_buffer, &mesh_comp->vertex_buffer_alloc);
	sy_render_create_index_buffer(&platform_info->render_info, SY_ARRLEN(index_data), index_data, &mesh_comp->index_buffer, &mesh_comp->index_buffer_alloc);
    }

    SyEntityHandle square = app_info->ecs.new_entity();
    app_info->ecs.entity_assign_component<SyMaterialComponent>(square, material_component_index);
    app_info->ecs.entity_assign_component<SyMesh>(square, mesh_component_index);

    SY_OUTPUT_INFO("finished render init");

}

void renderer_cleanup(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    // Cleanup meshes
    size_t mesh_type_id = app_info->ecs.get_type_id<SyMesh>();
    for (size_t i = 0; i < app_info->ecs.m_entity_used.m_filled_length; ++i)
    {
	if (app_info->ecs.m_entity_used.get<bool>(i) == true)
	{
	    if (app_info->ecs.m_entity_data.get<SyEntityData>(i).mask[mesh_type_id] == true)
	    {
		SyMesh *mesh = app_info->ecs.component<SyMesh>(i);

		vmaDestroyBuffer(platform_info->render_info.vma_allocator, mesh->vertex_buffer, mesh->vertex_buffer_alloc);
		vmaDestroyBuffer(platform_info->render_info.vma_allocator, mesh->index_buffer, mesh->index_buffer_alloc);
	    }
	}
    }

    // Cleanup descriptor sets
    for (int i = 0; i < platform_info->render_info.max_descriptor_sets_amt; ++i)
    {
	if (platform_info->render_info.descriptor_sets_used[i] == true)
	{
	    for (int j = 0; j < platform_info->render_info.max_frames_in_flight; ++j)
	    {
		vmaDestroyBuffer(platform_info->render_info.vma_allocator, platform_info->render_info.descriptor_sets[i].uniform_buffer[j], platform_info->render_info.descriptor_sets[i].uniform_buffer_allocation[j]);
	    }
	}
    }
    free(platform_info->render_info.descriptor_sets);
    free(platform_info->render_info.descriptor_sets_used);
    vkDestroyDescriptorPool(platform_info->render_info.logical_device, platform_info->render_info.descriptor_pool, NULL);

    vkDestroyPipeline(platform_info->render_info.logical_device, platform_info->render_info.single_color_pipeline, NULL);

    vkDestroyPipelineLayout(platform_info->render_info.logical_device, platform_info->render_info.single_color_pipeline_layout, NULL);

    vmaDestroyAllocator(platform_info->render_info.vma_allocator);

    vkDestroyDescriptorSetLayout(platform_info->render_info.logical_device, platform_info->render_info.frame_descriptor_set_layout, NULL);
    vkDestroyDescriptorSetLayout(platform_info->render_info.logical_device, platform_info->render_info.material_descriptor_set_layout, NULL);
    vkDestroyDescriptorSetLayout(platform_info->render_info.logical_device, platform_info->render_info.object_descriptor_set_layout, NULL);

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

    platform_info->render_info.pos = {0.0f, 0.0f, 0.0f};
    platform_info->render_info.rot = {0.0f, 0.0f};

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

        float rot_speed = 180;
    if (app_info->input_info.arrow_up && platform_info->render_info.rot[1] < 85.f)
	platform_info->render_info.rot[1] += rot_speed * app_info->delta_time;

    if (app_info->input_info.arrow_down && platform_info->render_info.rot[1] > -85.f)
	platform_info->render_info.rot[1] -= rot_speed * app_info->delta_time;

    if (app_info->input_info.arrow_left)
	platform_info->render_info.rot[0] -= rot_speed * app_info->delta_time;

    if (app_info->input_info.arrow_right)
	platform_info->render_info.rot[0] += rot_speed * app_info->delta_time;


    float move_speed = 2.0f;
    if (app_info->input_info.w)
	platform_info->render_info.pos[2] -= move_speed * app_info->delta_time;

    if (app_info->input_info.s)
	platform_info->render_info.pos[2] += move_speed * app_info->delta_time;

    if (app_info->input_info.a)
	platform_info->render_info.pos[0] -= move_speed * app_info->delta_time;

    if (app_info->input_info.d)
	platform_info->render_info.pos[0] += move_speed * app_info->delta_time;

    if (app_info->input_info.space)
	platform_info->render_info.pos[1] += move_speed * app_info->delta_time;

    if (app_info->input_info.shift_left)
	platform_info->render_info.pos[1] -= move_speed * app_info->delta_time;



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

    sy_render_draw(&platform_info->render_info, &platform_info->input_info, &app_info->ecs);

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

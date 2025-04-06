#include "sy_syengine.hpp"

#include <stdio.h>
#include <vulkan/vulkan_core.h>

#include "sy_ecs.hpp"
#include "sy_macros.hpp"

#include "render/sy_buffer.hpp"
#include "render/sy_drawing.hpp"
#include "render/sy_render_info.hpp"
#include "render/types/sy_mesh.hpp"

#include "obj_parser/sy_obj_parser.hpp"


#ifdef NDEBUG
extern "C"
void app_init(SyAppInfo *app_info);

extern "C"
void app_run(SyAppInfo *app_info);

extern "C"
void app_destroy(SyAppInfo *app_info);
#endif

void engine_init(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state)
{
    SY_OUTPUT_INFO("Starting Engine");


    app_info->stop_game = false;
    app_info->ecs.initialize();
    app_info->delta_time = 0.0;
    SY_ERROR_COND(app_info->persistent_arena.initialize(4096) != 0, "Failed to allocate data for persistent arena.");
    SY_ERROR_COND(app_info->frame_arena.initialize(4096) != 0, "Failed to allocate data for frame arena.");

    // Register ECS Types
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyMesh);

    // Init renderer
    sy_render_info_init(&platform_info->render_info, platform_info->input_info.window_width, platform_info->input_info.window_height);

    if (1)
    {
	size_t mesh_component_index;
	{
	    // Create component
	    int mesh_type_id = app_info->ecs.get_type_id<SyMesh>();
	    mesh_component_index = app_info->ecs.get_unused_component<SyMesh>();
	    SyMesh *mesh_comp = &app_info->ecs.m_component_data_arr[mesh_type_id].get<SyMesh>(mesh_component_index);
	    
	    // Create and fill buffers
	    uint32_t *index_data = NULL;
	    float *vertex_data = NULL;
	    size_t index_data_size = 0;
	    size_t vertex_data_size = 0;
	    sy_parse_obj("cube.obj", &vertex_data, &vertex_data_size, &index_data, &index_data_size);
	    mesh_comp->index_amt = index_data_size;
	    sy_render_create_vertex_buffer(&platform_info->render_info, vertex_data_size, sizeof(float) * 3, vertex_data, &mesh_comp->vertex_buffer, &mesh_comp->vertex_buffer_alloc);
	    sy_render_create_index_buffer(&platform_info->render_info, index_data_size, index_data, &mesh_comp->index_buffer, &mesh_comp->index_buffer_alloc);
	    free(vertex_data);
	    free(index_data);
	}
	
	SyEntityHandle square = app_info->ecs.new_entity();
	app_info->ecs.entity_assign_component<SyMesh>(square, mesh_component_index);
    }	    

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

void engine_run(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state)
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

    // stop the game signal
    if (app_info->stop_game == true)
    {
	platform_info->end_engine = true;
    }

    sy_render_draw(&platform_info->render_info, &platform_info->input_info, &app_info->ecs);

}

void engine_destroy(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state)
{
    SY_OUTPUT_INFO("Ending Engine");

#ifndef NDEBUG
    platform_info->app_destroy(app_info);
#else
    app_destroy(app_info);
#endif

    // Cleanup meshes FIXME:
    vkDeviceWaitIdle(platform_info->render_info.logical_device);
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

    app_info->ecs.destroy();

    sy_render_info_deinit(&platform_info->render_info);

    { // Cleanup App Info
	// Arena/Allocation Cleanup
	app_info->persistent_arena.destroy();
	app_info->frame_arena.destroy();
    }


}

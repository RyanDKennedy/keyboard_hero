#include "sy_syengine.hpp"

#include <stdio.h>

#include "sy_ecs.hpp"
#include "sy_macros.hpp"
#include "render/sy_render.hpp"
#include "render/sy_resources.hpp"

#ifdef NDEBUG
extern "C"
void app_init(SyAppInfo *app_info);

extern "C"
void app_run(SyAppInfo *app_info);

extern "C"
void app_destroy(SyAppInfo *app_info);
#endif


void engine_init(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    SY_OUTPUT_INFO("Starting Engine");
    
    app_info->stop_game = false;

    // ECS Init
    app_info->ecs.initialize();

    // Arena/Allocation Init
    SY_ERROR_COND(app_info->persistent_arena.initialize(4096) != 0, "Failed to allocate data for persistent arena.");
    SY_ERROR_COND(app_info->frame_arena.initialize(4096) != 0, "Failed to allocate data for frame arena.");

    app_info->delta_time = 0.0;

    app_info->global_mem_size = 2048;
    app_info->global_mem = app_info->persistent_arena.alloc(app_info->global_mem_size);

    // create render resources
    sy_render_create_resources(&platform_info->render_info, platform_info->input_info.window_width, platform_info->input_info.window_height);
    sy_render_create_pipeline_resources(&platform_info->render_info, &platform_info->pipeline_info);

    // init render types in ecs
    sy_render_init_ecs(&app_info->ecs);

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

}

void engine_destroy(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    SY_OUTPUT_INFO("Ending Engine");

#ifndef NDEBUG
    platform_info->app_destroy(app_info);
#else
    app_destroy(app_info);
#endif

    sy_render_destroy_pipeline_resources(&platform_info->render_info, &platform_info->pipeline_info);
    sy_render_destroy_resources(&platform_info->render_info);

    { // Cleanup App Info
	// Arena/Allocation Cleanup
	app_info->persistent_arena.destroy();
	app_info->frame_arena.destroy();
    }

    app_info->ecs.destroy();
}

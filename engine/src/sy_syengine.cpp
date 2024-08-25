#include "sy_syengine.hpp"

#include <stdio.h>

#include "sy_ecs.hpp"
#include "sy_macros.hpp"

void engine_init(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    SY_OUTPUT_INFO("Starting Engine");
    
    app_info->stop_game = false;

    SY_ECS_REGISTER_TYPE(app_info->ecs, int);

    platform_info->app_init(app_info);
}

void engine_run(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    app_info->frame_arena.free_all();

    SyInputInfo &input = app_info->input_info;

    // close the engine
    if (input.escape || input.window_should_close)
    {
	platform_info->end_engine = true;
    }

    // reload dlls for app
    if (input.r)
    {
	platform_info->reload_dll = true;
    }

    // run the app
    platform_info->app_run(app_info);

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
}

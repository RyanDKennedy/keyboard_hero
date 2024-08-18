#include "sy_syengine.hpp"

#include <stdio.h>
#include <chrono>
#include <thread>

#include "sy_ecs.hpp"
#include "sy_macros.hpp"

// FIXME


void engine_init(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    SY_OUTPUT_INFO("Starting Engine");
    
    SY_ECS_REGISTER_TYPE(app_info->ecs, double);
    SY_ECS_REGISTER_TYPE(app_info->ecs, int);

    platform_info->app_init(app_info);
}

void engine_run(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    app_info->frame_arena.free_all();

    SyInputInfo &input = app_info->input_info;

    if (input.enter)
    {
	printf("FPS: %lf\n", 1000.0 / app_info->delta_time);
    }

    if (input.escape || input.window_should_close)
    {
	platform_info->end_engine = true;
    }

    platform_info->app_run(app_info);
}

void engine_destroy(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    SY_OUTPUT_INFO("Ending Engine");
}

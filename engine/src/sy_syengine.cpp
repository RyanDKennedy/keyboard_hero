#include "sy_syengine.hpp"

#include <stdio.h>
#include <chrono>
#include <thread>

#include "sy_macros.hpp"

void init_engine(SyenginePlatformInfo *platform_info)
{
    SY_OUTPUT_INFO("Starting Engine");
    
    platform_info->ecs.register_type<int>();

    printf("int id: %lu\n", platform_info->ecs.get_type_id<int>());

}

void run_engine(SyenginePlatformInfo *platform_info)
{
    platform_info->frame_arena.free_all();

    SyInputInfo &input = platform_info->input_info;

    if (input.enter)
    {
	printf("FPS: %lf\n", 1000.0 / platform_info->delta_time);
    }

    if (input.escape || input.window_should_close)
    {
	platform_info->end_engine = true;
    }
}

void destroy_engine(SyenginePlatformInfo *platform_info)
{
    SY_OUTPUT_INFO("Ending Engine");
}

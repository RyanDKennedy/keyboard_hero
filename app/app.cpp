#include <stdio.h>

#include "sy_ecs.hpp"
#include "sy_app_info.hpp"

#include "global.hpp"

extern "C"
void app_init(SyAppInfo *app_info)
{
    app_info->global_mem_size = 2048;
    app_info->global_mem = app_info->persistent_arena.alloc(app_info->global_mem_size);

    // Check if global_mem is enough for global struct
    if (app_info->global_mem_size < sizeof(Global))
    {
	SY_OUTPUT_INFO("app_info->global_mem_size < sizeof(Global)");
	app_info->stop_game = true;
	return;
    }

    g_state = (Global*)app_info->global_mem;
}

extern "C"
void app_run(SyAppInfo *app_info)
{
    g_state = (Global*)app_info->global_mem;

    if (app_info->input_info.q)
	app_info->stop_game = true;

    if (app_info->input_info.p)
	printf("FPS: %f\n", 1.0f / app_info->delta_time);


}

extern "C"
void app_destroy(SyAppInfo *app_info)
{
    SY_OUTPUT_INFO("destroying app");
}


#ifndef NDEBUG
// When I reload the dll this will run instead of app_init, this will not run on the initial start
extern "C"
void app_dll_init(SyAppInfo *app_info)
{
    SY_OUTPUT_DEBUG("new dll init")
}

// When I reload the dll this will run before the dll is reloaded
extern "C"
void app_dll_exit(SyAppInfo *app_info)
{
    SY_OUTPUT_DEBUG("old dll exit")
    
}
#endif

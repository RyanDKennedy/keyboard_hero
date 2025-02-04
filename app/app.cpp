#include <stdio.h>

#include "sy_ecs.hpp"
#include "sy_syengine.hpp"

struct Global
{

};

extern "C"
void app_init(SyAppInfo *app_info)
{
    // Check if global_mem is enough for global struct
    if (app_info->global_mem_size < sizeof(Global))
    {
	SY_OUTPUT_INFO("app_info->global_mem_size < sizeof(Global)");
	app_info->stop_game = true;
	return;
    }

    Global *global = (Global*)app_info->global_mem;

    // FIXME
    SY_ECS_REGISTER_TYPE(app_info->ecs, int);

}

extern "C"
void app_run(SyAppInfo *app_info)
{
    Global *global = (Global*)app_info->global_mem;

    if (app_info->input_info.q)
	app_info->stop_game = true;

}

extern "C"
void app_destroy(SyAppInfo *app_info)
{

}

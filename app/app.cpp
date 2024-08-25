#include <stdio.h>

#include "sy_ecs.hpp"
#include "sy_syengine.hpp"

extern "C"
void app_init(SyAppInfo *app_info)
{
    SY_ECS_REGISTER_TYPE(app_info->ecs, int);

}

extern "C"
void app_run(SyAppInfo *app_info)
{
    if (app_info->input_info.q)
	app_info->stop_game = true;
    if (app_info->input_info.h)
	printf("a\n");
}

extern "C"
void app_destroy(SyAppInfo *app_info)
{

}

#include <stdio.h>

#include "sy_ecs.hpp"
#include "sy_syengine.hpp"

extern "C"
void app_init(SyAppInfo *app_info)
{
    SY_ECS_REGISTER_TYPE(app_info->ecs, int);
    SY_ECS_REGISTER_TYPE(app_info->ecs, double);
    SY_ECS_REGISTER_TYPE(app_info->ecs, float);
}

extern "C"
void app_run(SyAppInfo *app_info)
{

}

extern "C"
void app_destroy(SyAppInfo *app_info)
{

}

#pragma once

#include "sy_platform_info.hpp"
#include "sy_app_info.hpp"

struct SyEngineState
{

    
};

void engine_init(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state);
void engine_run(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state);
void engine_destroy(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state);

#pragma once

#include "sy_platform_info.hpp"
#include "sy_app_info.hpp"
#include "sound/sy_sound.hpp"

struct SyEngineState
{
    SySoundInfo sound_info;
    
};

void engine_init(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state);
void engine_run(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state);
void engine_destroy(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state);

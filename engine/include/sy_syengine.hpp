#pragma once

#include "sy_input_info.hpp"
#include "sy_arena.hpp"
#include "sy_ecs.hpp"

struct SyenginePlatformInfo
{
    SyArena persistent_arena;
    SyArena frame_arena;
    SyEcs ecs;
    SyInputInfo input_info;
    double delta_time;
    bool end_engine; // mark this true to end engine

};

void init_engine(SyenginePlatformInfo *platform_info);
void run_engine(SyenginePlatformInfo *platform_info);
void destroy_engine(SyenginePlatformInfo *platform_info);

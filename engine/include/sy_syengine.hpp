#pragma once

#include "sy_input_info.hpp"
#include "sy_arena.hpp"
#include "sy_ecs.hpp"

struct SyPlatformInfo;
struct SyAppInfo;

// used to communicate platform <--> engine
struct SyPlatformInfo
{
    void *dll_handle;
    void (*app_init)(SyAppInfo*);
    void (*app_run)(SyAppInfo*);
    void (*app_destroy)(SyAppInfo*);
    bool end_engine; // mark this true to end engine
};

// used to communicate engine <--> app
struct SyAppInfo
{
    SyArena persistent_arena;
    SyArena frame_arena;
    SyEcs ecs;
    SyInputInfo input_info;
    double delta_time;    
};

void engine_init(SyPlatformInfo *platform_info, SyAppInfo *app_info);
void engine_run(SyPlatformInfo *platform_info, SyAppInfo *app_info);
void engine_destroy(SyPlatformInfo *platform_info, SyAppInfo *app_info);

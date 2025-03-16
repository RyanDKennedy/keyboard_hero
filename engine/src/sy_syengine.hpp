#pragma once

/* TODO:
   - move SyAppInfo out of this file so that the app doesn't have to get information about the platform info
   - find a better place for pipeline info besides platform info since the platform doesn't need it
 */

#include "sy_input_info.hpp"
#include "sy_arena.hpp"
#include "sy_ecs.hpp"

#include "render/sy_render_info.hpp"
#include "render/sy_pipeline.hpp"

struct SyPlatformInfo;
struct SyAppInfo;

// used to communicate platform <--> engine
struct SyPlatformInfo
{
#ifndef NDEBUG
    void *dll_handle;
    void (*app_init)(SyAppInfo*);
    void (*app_run)(SyAppInfo*);
    void (*app_destroy)(SyAppInfo*);
    void (*app_dll_init)(SyAppInfo*);
    void (*app_dll_exit)(SyAppInfo*);
    bool reload_dll;
    bool dll_first_run; // when the dll has just been loaded
#endif

    bool end_engine; // mark this true to end engine

    SyInputInfo input_info;
    size_t delta_time; // in microseconds

    SyRenderInfo render_info;
    SyRenderPipelineInfo pipeline_info;
};

// used to communicate engine <--> app
struct SyAppInfo
{
    SyArena persistent_arena;
    SyArena frame_arena;
    SyEcs ecs;
    SyInputInfo input_info;
    double delta_time; // in seconds
    bool stop_game;

    void *global_mem; // pointer to memory that the user can use
    size_t global_mem_size;
};

void engine_init(SyPlatformInfo *platform_info, SyAppInfo *app_info);
void engine_run(SyPlatformInfo *platform_info, SyAppInfo *app_info);
void engine_destroy(SyPlatformInfo *platform_info, SyAppInfo *app_info);

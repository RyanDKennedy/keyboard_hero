#pragma once

#include "sy_app_info.hpp"
#include "sy_input_info.hpp"
#include "render/sy_render_info.hpp"

// used to communicate platform <--> engine
// and engine state
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
};

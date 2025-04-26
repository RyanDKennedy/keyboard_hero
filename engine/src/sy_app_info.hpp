#pragma once

#include "render/types/sy_asset_metadata.hpp"
#include "sy_input_info.hpp"
#include "sy_arena.hpp"
#include "sy_ecs.hpp"
#include "glm_include.hpp"
#include "types/sy_camera_settings.hpp"

// used to communicate engine <--> app
struct SyAppInfo
{
    // NOTE: this is only so that you can pass it to functions that load assets inside the app code you will never actually do anything with render info yourself
    void *render_info;
    
    SyArena persistent_arena;
    SyArena frame_arena;
    SyEcs ecs;
    SyInputInfo input_info;
    double delta_time; // in seconds
    bool stop_game;

    void *global_mem; // pointer to memory that the user can use
    //size_t global_mem_size;

    SyCameraSettings camera_settings;

#ifndef NDEBUG
    // ASSET SYSTEM FUNCTION POINTERS:
    // These are only used for debug builds
    size_t(*sy_load_asset_from_file)(void *render_info, SyEcs *ecs, const char *file_path, SyAssetType type);
#endif
};

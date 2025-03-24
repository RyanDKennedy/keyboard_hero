#pragma once

#include "sy_input_info.hpp"
#include "sy_arena.hpp"
#include "sy_ecs.hpp"
#include "glm_include.hpp"

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

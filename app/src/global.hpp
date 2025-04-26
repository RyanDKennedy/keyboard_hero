#pragma once

#include "sy_ecs.hpp"
#include "sy_app_info.hpp"
#include "components/sy_transform.hpp"
#include "render/types/sy_material.hpp"
#include "render/types/sy_draw_info.hpp"
#include "render/types/sy_asset_metadata.hpp"
#include "render/types/sy_ui_text.hpp"
#include "types/sy_camera_settings.hpp"
#include "glm_include.hpp"

#ifndef NDEBUG
#define SY_LOAD_ASSET_FROM_FILE(render_info, ...) app_info->sy_load_asset_from_file((void*)(render_info), __VA_ARGS__);
#else
#include "asset_system/sy_asset_system.hpp"
#define SY_LOAD_ASSET_FROM_FILE(render_info, ...) sy_load_asset_from_file((SyRenderInfo*)(render_info), __VA_ARGS__);
#endif

enum class GameMode
{
    none, // for testing
    menu,
    play_menu,
    create,
    edit,
};

struct Global
{
    GameMode game_mode;
    size_t font_asset_metadata_index;
    

    SyEntityHandle player;

    // Menu stuff
    static const size_t buttons_amt = 3;
    SyEntityHandle buttons[buttons_amt];
    size_t selected_btn;
    SyEntityHandle menu_title;

};

inline Global *g_state;

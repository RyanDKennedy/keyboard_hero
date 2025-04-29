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

#include <sqlite3.h>

#include "db.hpp"

#ifndef NDEBUG
#define SY_LOAD_ASSET_FROM_FILE(render_info, ...) app_info->sy_load_asset_from_file((void*)(render_info), __VA_ARGS__);
#else
#include "asset_system/sy_asset_system.hpp"
#define SY_LOAD_ASSET_FROM_FILE(render_info, ...) sy_load_asset_from_file((SyRenderInfo*)(render_info), __VA_ARGS__);
#endif

enum class GameMode
{
    menu,
    create,
    edit,
    picker,
    play,
};

struct EntityNote
{
    DBNote note;
    SyEntityHandle entity;
};

struct MenuCtx
{
    static const size_t buttons_amt = 3;
    SyEntityHandle buttons[buttons_amt];
    size_t selected_btn;
    SyEntityHandle menu_title;

    bool able_to_play; // this determines if play button is greyed out, this only happens when the db has no songs in it.
};

struct PickerCtx
{
    size_t persistent_arena_starting_alloc;

    size_t songs_amt;
    DBSong *songs;
    SyEntityHandle *song_labels;
    size_t selected_song;

    GameMode next_game_mode;
};

struct CreateCtx
{
    SyEntityHandle name_label;
    char *name_label_data;
    size_t name_label_data_len;

    SyEntityHandle name;
    char *name_data;
    size_t name_data_len;
};

struct EditCtx
{
    size_t persistent_arena_starting_alloc;

    DBSong song;

    SyEntityHandle title;
    char *title_data;

    static const size_t keys_amt = 4;
    SyEntityHandle key_entities[keys_amt];

    SyEntityHandle display;
    size_t display_data_size;
    char *display_data;

    size_t note_asset_metadata_id;
    size_t persistent_arena_notes_alloc;
    size_t notes_amt;
    EntityNote *notes;

    size_t currently_selected_note;

    SyEntityHandle note_display;
};

struct PlayCtx
{
    size_t persistent_arena_starting_alloc;

    DBSong song;

    SyEntityHandle title;
    char *title_data;

    static const size_t keys_amt = 4;
    SyEntityHandle key_entities[keys_amt];

    size_t note_asset_metadata_id;
    size_t notes_amt;
    EntityNote *notes;

    float time_running;
};

struct Global
{
    sqlite3 *db;

    GameMode game_mode;
    size_t font_asset_metadata_index;

    SyEntityHandle player;

    MenuCtx menu_ctx;
    PickerCtx picker_ctx;
    EditCtx edit_ctx;
    CreateCtx create_ctx;
    PlayCtx play_ctx;
};

inline Global *g_state;

#include "picker.hpp"

#include "db.hpp"
#include "edit.hpp"
#include "global.hpp"
#include "menu.hpp"
#include "render/types/sy_draw_info.hpp"

void picker_load(SyAppInfo *app_info)
{
    PickerCtx *picker_ctx = &g_state->picker_ctx;
    
}

void picker_start(SyAppInfo *app_info, GameMode next_game_mode)
{
    PickerCtx *picker_ctx = &g_state->picker_ctx;

    picker_ctx->persistent_arena_starting_alloc = app_info->persistent_arena.m_current_offset;

    picker_ctx->next_game_mode = next_game_mode;

    db_get_all_songs(g_state->db, NULL, &picker_ctx->songs_amt);
    
    picker_ctx->songs = (DBSong*)app_info->persistent_arena.alloc(sizeof(DBSong) * picker_ctx->songs_amt);

    db_get_all_songs(g_state->db, picker_ctx->songs, NULL);

    picker_ctx->song_labels = (SyEntityHandle*)app_info->persistent_arena.alloc(sizeof(SyEntityHandle) * picker_ctx->songs_amt);

    picker_ctx->selected_song = 0;

    float center_offset = -1.0f * picker_ctx->selected_song * 0.1;
    for (size_t i = 0; i < picker_ctx->songs_amt; ++i)
    {
	picker_ctx->song_labels[i] = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(picker_ctx->song_labels[i]);
	app_info->ecs.entity_add_component<SyUIText>(picker_ctx->song_labels[i]);

	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(picker_ctx->song_labels[i]);
	SyUIText *ui_text = app_info->ecs.component<SyUIText>(picker_ctx->song_labels[i]);

	draw_info->should_draw = true;
	draw_info->asset_metadata_id = g_state->font_asset_metadata_index;

	ui_text->text = picker_ctx->songs[i].name;
	ui_text->alignment = SyTextAlignment::center;
	ui_text->color = glm::vec3(1.0f, 1.0f, 1.0f);
	ui_text->pos = glm::vec2(0.0f, i * 0.1 + center_offset);
	ui_text->scale = glm::vec2(0.075, 0.075);

	if (i == picker_ctx->selected_song)
	{
	    ui_text->color = glm::vec3(1.0f, 1.0f, 0.0f);
	}
    }
}

void picker_run(SyAppInfo *app_info)
{
    PickerCtx *picker_ctx = &g_state->picker_ctx;

    if (app_info->input_info.arrow_up == SyKeyState::released || app_info->input_info.arrow_down == SyKeyState::released)
    {
	if (app_info->input_info.arrow_up == SyKeyState::released && picker_ctx->selected_song > 0)
	{
	    --picker_ctx->selected_song;
	}

	if (app_info->input_info.arrow_down == SyKeyState::released && picker_ctx->selected_song < picker_ctx->songs_amt - 1)
	{
	    ++picker_ctx->selected_song;
	}

	float center_offset = -1.0f * picker_ctx->selected_song * 0.1;
	for (size_t i = 0; i < picker_ctx->songs_amt; ++i)
	{
	    SyUIText *ui_text = app_info->ecs.component<SyUIText>(picker_ctx->song_labels[i]);
	    
	    ui_text->color = glm::vec3(1.0f, 1.0f, 1.0f);
	    ui_text->pos = glm::vec2(0.0f, i * 0.1 + center_offset);
	    
	    if (i == picker_ctx->selected_song)
	    {
		ui_text->color = glm::vec3(1.0f, 1.0f, 0.0f);
	    }
	}
    }

    if (app_info->input_info.escape == SyKeyState::released)
    {
	picker_stop(app_info);
	g_state->game_mode = GameMode::menu;
	menu_start(app_info);
	return;
    }

    if (app_info->input_info.enter == SyKeyState::released)
    {
	DBSong selected_song = picker_ctx->songs[picker_ctx->selected_song];

	picker_stop(app_info);
	g_state->game_mode = picker_ctx->next_game_mode;
	switch (g_state->game_mode)
	{
	    case GameMode::play:
		break;
	    case GameMode::edit:
		edit_start(app_info, selected_song);
	}
    }
}
void picker_stop(SyAppInfo *app_info)
{
    PickerCtx *picker_ctx = &g_state->picker_ctx;

    for (size_t i = 0; i < picker_ctx->songs_amt; ++i)
    {
	app_info->ecs.destroy_entity(picker_ctx->song_labels[i]);
    }

    app_info->persistent_arena.m_current_offset = picker_ctx->persistent_arena_starting_alloc;

}

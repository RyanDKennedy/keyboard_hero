#include "deletion.hpp"

#include "db.hpp"
#include "edit.hpp"
#include "global.hpp"
#include "menu.hpp"
#include "play.hpp"
#include "render/types/sy_draw_info.hpp"

void deletion_load(SyAppInfo *app_info)
{
    DeletionCtx *deletion_ctx = &g_state->deletion_ctx;
}

void deletion_start(SyAppInfo *app_info)
{
    DeletionCtx *deletion_ctx = &g_state->deletion_ctx;

    deletion_ctx->persistent_arena_starting_alloc = app_info->persistent_arena.m_current_offset;

    db_get_all_songs(g_state->db, NULL, &deletion_ctx->songs_amt);
    
    deletion_ctx->songs = (DBSong*)app_info->persistent_arena.alloc(sizeof(DBSong) * deletion_ctx->songs_amt);

    db_get_all_songs(g_state->db, deletion_ctx->songs, NULL);

    deletion_ctx->song_labels = (SyEntityHandle*)app_info->persistent_arena.alloc(sizeof(SyEntityHandle) * deletion_ctx->songs_amt);

    deletion_ctx->selected_song = 0;

    float center_offset = -1.0f * deletion_ctx->selected_song * 0.1;
    for (size_t i = 0; i < deletion_ctx->songs_amt; ++i)
    {
	deletion_ctx->song_labels[i] = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(deletion_ctx->song_labels[i]);
	app_info->ecs.entity_add_component<SyUIText>(deletion_ctx->song_labels[i]);

	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(deletion_ctx->song_labels[i]);
	SyUIText *ui_text = app_info->ecs.component<SyUIText>(deletion_ctx->song_labels[i]);

	draw_info->should_draw = true;
	draw_info->asset_metadata_id = g_state->font_asset_metadata_index;

	ui_text->text = deletion_ctx->songs[i].name;
	ui_text->alignment = SyTextAlignment::center;
	ui_text->color = glm::vec3(1.0f, 1.0f, 1.0f);
	ui_text->pos = glm::vec2(0.0f, i * 0.1 + center_offset);
	ui_text->scale = glm::vec2(0.075, 0.075);

	if (i == deletion_ctx->selected_song)
	{
	    ui_text->color = glm::vec3(1.0f, 1.0f, 0.0f);
	}
    }
}

void deletion_run(SyAppInfo *app_info)
{
    DeletionCtx *deletion_ctx = &g_state->deletion_ctx;

    if (app_info->input_info.arrow_up == SyKeyState::released || app_info->input_info.arrow_down == SyKeyState::released)
    {
	if (app_info->input_info.arrow_up == SyKeyState::released && deletion_ctx->selected_song > 0)
	{
	    --deletion_ctx->selected_song;
	}

	if (app_info->input_info.arrow_down == SyKeyState::released && deletion_ctx->selected_song < deletion_ctx->songs_amt - 1)
	{
	    ++deletion_ctx->selected_song;
	}

	float center_offset = -1.0f * deletion_ctx->selected_song * 0.1;
	for (size_t i = 0; i < deletion_ctx->songs_amt; ++i)
	{
	    SyUIText *ui_text = app_info->ecs.component<SyUIText>(deletion_ctx->song_labels[i]);
	    
	    ui_text->color = glm::vec3(1.0f, 1.0f, 1.0f);
	    ui_text->pos = glm::vec2(0.0f, i * 0.1 + center_offset);
	    
	    if (i == deletion_ctx->selected_song)
	    {
		ui_text->color = glm::vec3(1.0f, 1.0f, 0.0f);
	    }
	}
    }

    if (app_info->input_info.escape == SyKeyState::released)
    {
	deletion_stop(app_info);
	g_state->game_mode = GameMode::menu;
	menu_start(app_info);
	return;
    }

    if (app_info->input_info.enter == SyKeyState::released)
    {
	DBSong selected_song = deletion_ctx->songs[deletion_ctx->selected_song];
	db_delete_song(g_state->db, selected_song.id);
	deletion_stop(app_info);

	g_state->game_mode = GameMode::menu;
	menu_start(app_info);
    }
}
void deletion_stop(SyAppInfo *app_info)
{
    DeletionCtx *deletion_ctx = &g_state->deletion_ctx;

    for (size_t i = 0; i < deletion_ctx->songs_amt; ++i)
    {
	app_info->ecs.destroy_entity(deletion_ctx->song_labels[i]);
    }

    app_info->persistent_arena.m_current_offset = deletion_ctx->persistent_arena_starting_alloc;

}

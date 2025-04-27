#include "edit.hpp"
#include "global.hpp"
#include "menu.hpp"
#include "render/types/sy_asset_metadata.hpp"
#include "render/types/sy_material.hpp"
#include "util.hpp"

void edit_load(SyAppInfo *app_info)
{
    EditCtx *edit_ctx = &g_state->edit_ctx;

    { // create title
	edit_ctx->title = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(edit_ctx->title);
	app_info->ecs.entity_add_component<SyUIText>(edit_ctx->title);
	SyDrawInfo *title_draw_info = app_info->ecs.component<SyDrawInfo>(edit_ctx->title);
	title_draw_info->should_draw = false;
	title_draw_info->asset_metadata_id = g_state->font_asset_metadata_index;
	SyUIText *ui_text = app_info->ecs.component<SyUIText>(edit_ctx->title);
	ui_text->alignment = SyTextAlignment::center;
	ui_text->color = glm::vec3(1.f, 1.f, 1.f);
	ui_text->pos = glm::vec2(0.f, -0.85f);
	ui_text->scale = glm::vec2(0.1f, 0.1f);
	ui_text->text = "Song Customizer";
    }

    { // create keys
	size_t key_mesh_metadata_index = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "app/key.obj", SyAssetType::mesh);

	for (size_t i = 0; i < edit_ctx->keys_amt; ++i)
	{
	    edit_ctx->key_entities[i] = app_info->ecs.new_entity();
	    app_info->ecs.entity_add_component<SyDrawInfo>(edit_ctx->key_entities[i]);
	    app_info->ecs.entity_add_component<SyTransform>(edit_ctx->key_entities[i]);
	    app_info->ecs.entity_add_component<SyMaterial>(edit_ctx->key_entities[i]);

	    SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(edit_ctx->key_entities[i]);
	    SyTransform *transform = app_info->ecs.component<SyTransform>(edit_ctx->key_entities[i]);
	    SyMaterial *material = app_info->ecs.component<SyMaterial>(edit_ctx->key_entities[i]);

	    draw_info->should_draw = false;
	    draw_info->asset_metadata_id = key_mesh_metadata_index;

	    transform->position = glm::vec3(-3.0f + i * 2.0f, 0.0f, 0.0f);
	    transform->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	    transform->scale = glm::vec3(1.0f, 1.0f, 1.0f);
	    
	    material->diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	}
	
	
    }
}

void edit_start(SyAppInfo *app_info, const char *song_name)
{
    EditCtx *edit_ctx = &g_state->edit_ctx;

    edit_ctx->persistent_arena_starting_alloc = app_info->persistent_arena.m_current_offset;

    // allocate and fill title data and song data
    edit_ctx->title_data = (char*)app_info->persistent_arena.alloc(strlen("Song Customizer: ") + strlen(song_name) + 1);
    sprintf(edit_ctx->title_data, "Song Customizer: %s", song_name);
    app_info->ecs.component<SyUIText>(edit_ctx->title)->text = edit_ctx->title_data;

    edit_ctx->song_name = (char*)app_info->persistent_arena.alloc(strlen(song_name) + 1);
    strcpy(edit_ctx->song_name, song_name);

    // make everything draw
    app_info->ecs.component<SyDrawInfo>(edit_ctx->title)->should_draw = true;
    for (size_t i = 0; i < edit_ctx->keys_amt; ++i)
    {
	app_info->ecs.component<SyDrawInfo>(edit_ctx->key_entities[i])->should_draw = true;
    }
}

void edit_run(SyAppInfo *app_info)
{
    EditCtx *edit_ctx = &g_state->edit_ctx;

    orthographic_movement(app_info, 15, 15, 15);

    if (app_info->input_info.escape == SyKeyState::released)
    {
	edit_stop(app_info);
	g_state->game_mode = GameMode::menu;
	menu_start(app_info);
	return;
    }
}

void edit_stop(SyAppInfo *app_info)
{
    EditCtx *edit_ctx = &g_state->edit_ctx;

    app_info->ecs.component<SyDrawInfo>(edit_ctx->title)->should_draw = false;
    for (size_t i = 0; i < edit_ctx->keys_amt; ++i)
    {
	app_info->ecs.component<SyDrawInfo>(edit_ctx->key_entities[i])->should_draw = false;
    }

    app_info->persistent_arena.m_current_offset = edit_ctx->persistent_arena_starting_alloc;
}

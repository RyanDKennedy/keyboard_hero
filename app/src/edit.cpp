#include "edit.hpp"
#include "db.hpp"
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
	ui_text->pos = glm::vec2(0.f, -0.93f);
	ui_text->scale = glm::vec2(0.05f, 0.05f);
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
	    transform->scale = glm::vec3(0.4f, 0.4f, 1.f);
	    
	    material->diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	}
    }

    { // create display
	edit_ctx->display = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(edit_ctx->display);
	app_info->ecs.entity_add_component<SyUIText>(edit_ctx->display);
	SyDrawInfo *display_draw_info = app_info->ecs.component<SyDrawInfo>(edit_ctx->display);
	display_draw_info->should_draw = false;
	display_draw_info->asset_metadata_id = g_state->font_asset_metadata_index;
	SyUIText *ui_text = app_info->ecs.component<SyUIText>(edit_ctx->display);
	ui_text->alignment = SyTextAlignment::center;
	ui_text->color = glm::vec3(0.8f, 0.8f, 1.f);
	ui_text->pos = glm::vec2(0.f, -0.85f);
	ui_text->scale = glm::vec2(0.05f, 0.05f);
	ui_text->text = "You shouldn't see this text";
    }
}

void edit_start(SyAppInfo *app_info, DBSong song)
{
    EditCtx *edit_ctx = &g_state->edit_ctx;

    // load song
    edit_ctx->song = song;

    // get offset of arena before this game mode
    edit_ctx->persistent_arena_starting_alloc = app_info->persistent_arena.m_current_offset;

    // allocate and fill title data and song data
    edit_ctx->title_data = (char*)app_info->persistent_arena.alloc(strlen("Song Customizer: ") + strlen(song.name) + 1);
    sprintf(edit_ctx->title_data, "Song Customizer: %s", song.name);
    app_info->ecs.component<SyUIText>(edit_ctx->title)->text = edit_ctx->title_data;

    // update display
    edit_ctx->display_data_size = strlen("Duration: 0000.00s\nCurrent Player Time: 0000.00s") + 1;
    edit_ctx->display_data = (char*)app_info->persistent_arena.alloc(edit_ctx->display_data_size);
    app_info->ecs.component<SyUIText>(edit_ctx->display)->text = edit_ctx->display_data;
    snprintf(edit_ctx->display_data, edit_ctx->display_data_size, "Duration: %.2fs\nCurrent Player Time: %.2fs", edit_ctx->song.duration, 0.0);


    // Change keys length
    for (size_t i = 0; i < edit_ctx->keys_amt; ++i)
    {
	SyTransform *transform = app_info->ecs.component<SyTransform>(edit_ctx->key_entities[i]);
	transform->scale = glm::vec3(0.4f, 0.4f, 10.0f * song.duration);
    }

    // make everything draw
    app_info->ecs.component<SyDrawInfo>(edit_ctx->title)->should_draw = true;
    app_info->ecs.component<SyDrawInfo>(edit_ctx->display)->should_draw = true;
for (size_t i = 0; i < edit_ctx->keys_amt; ++i)
    {
	app_info->ecs.component<SyDrawInfo>(edit_ctx->key_entities[i])->should_draw = true;
    }

    { // set player position
	SyTransform *transform = app_info->ecs.component<SyTransform>(g_state->player);
	transform->position = glm::vec3(0.0f, 5.0f, 10.0f);
	transform->rotation = glm::vec3(-1.0f, 180.0f, 0.0f);
	transform->scale = glm::vec3(0.0f, 0.0f, 0.0f);
    }
}

void edit_run(SyAppInfo *app_info)
{
    EditCtx *edit_ctx = &g_state->edit_ctx;



    { // player movement
	SyTransform *player_transform = app_info->ecs.component<SyTransform>(app_info->camera_settings.active_camera); 
	
	glm::vec3 front;
	{
	    front[0] = glm::sin(glm::radians((float)player_transform->rotation[1])) * glm::cos(glm::radians((float)player_transform->rotation[0]));
	    front[1] = 0.0f;
	    front[2] = glm::cos(glm::radians((float)player_transform->rotation[1])) * glm::cos(glm::radians((float)player_transform->rotation[0]));
	    front = glm::normalize(front);
	}
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	
	if (abs(app_info->input_info.mouse_dy) != 0)
	{
	    player_transform->rotation[0] += app_info->delta_time * 15.0f * -app_info->input_info.mouse_dy;
	    if (player_transform->rotation[0] < -20.0)
		player_transform->rotation[0] = -20.0;
	    
	    if (player_transform->rotation[0] > -0.5)
		player_transform->rotation[0] = -0.5;
	}    
	
	if (app_info->input_info.w == SyKeyState::pressed)
	    player_transform->position[2] -= 10.0f * app_info->delta_time;
	
	if (app_info->input_info.s == SyKeyState::pressed)
	    player_transform->position[2] += 10.0f * app_info->delta_time;
	
	if (player_transform->position[2] > 10.0f)
	    player_transform->position[2] = 10.0f;
	
	if (player_transform->position[2] / -10 + 1 > edit_ctx->song.duration)
	    player_transform->position[2] = (edit_ctx->song.duration - 1) * -10.0f;

	app_info->camera_settings.perspective_settings.aspect_ratio = (float)app_info->input_info.window_width / app_info->input_info.window_height;
    }

    float current_player_time = app_info->ecs.component<SyTransform>(g_state->player)->position[2] / -10.0f + 1;

    // Change song duration
    if (app_info->input_info.p == SyKeyState::pressed || app_info->input_info.o == SyKeyState::pressed)
    {
	int sign = (app_info->input_info.p == SyKeyState::pressed) - (app_info->input_info.o == SyKeyState::pressed);
	
	edit_ctx->song.duration += sign * 3.0f * app_info->delta_time;

	if (edit_ctx->song.duration < 1.0f)
	    edit_ctx->song.duration = 1.0f;

	// Change keys length
	for (size_t i = 0; i < edit_ctx->keys_amt; ++i)
	{
	    SyTransform *transform = app_info->ecs.component<SyTransform>(edit_ctx->key_entities[i]);
	    transform->scale = glm::vec3(0.4f, 0.4f, 10.0f * edit_ctx->song.duration);
	}
    }

    // update display
    snprintf(edit_ctx->display_data, edit_ctx->display_data_size, "Duration: %.2fs\nCurrent Player Time: %.2fs", edit_ctx->song.duration, current_player_time);


    if (app_info->input_info.escape == SyKeyState::released)
    {
	// save changes
	db_update_song(g_state->db, edit_ctx->song);

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
    app_info->ecs.component<SyDrawInfo>(edit_ctx->display)->should_draw = false;
    for (size_t i = 0; i < edit_ctx->keys_amt; ++i)
    {
	app_info->ecs.component<SyDrawInfo>(edit_ctx->key_entities[i])->should_draw = false;
    }

    app_info->persistent_arena.m_current_offset = edit_ctx->persistent_arena_starting_alloc;
}

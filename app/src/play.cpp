#include "play.hpp"
#include "menu.hpp"

void play_load(SyAppInfo *app_info)
{
    PlayCtx *play_ctx = &g_state->play_ctx;

    { // create title
	play_ctx->title = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(play_ctx->title);
	app_info->ecs.entity_add_component<SyUIText>(play_ctx->title);
	SyDrawInfo *title_draw_info = app_info->ecs.component<SyDrawInfo>(play_ctx->title);
	title_draw_info->should_draw = false;
	title_draw_info->asset_metadata_id = g_state->font_asset_metadata_index;
	SyUIText *ui_text = app_info->ecs.component<SyUIText>(play_ctx->title);
	ui_text->alignment = SyTextAlignment::center;
	ui_text->color = glm::vec3(1.f, 1.f, 1.f);
	ui_text->pos = glm::vec2(0.f, -0.93f);
	ui_text->scale = glm::vec2(0.05f, 0.05f);
	ui_text->text = "You shouldn't see this";
    }

    { // create keys
	size_t key_mesh_metadata_index = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "app/key.obj", SyAssetType::mesh);

	for (size_t i = 0; i < play_ctx->keys_amt; ++i)
	{
	    play_ctx->key_entities[i] = app_info->ecs.new_entity();
	    app_info->ecs.entity_add_component<SyDrawInfo>(play_ctx->key_entities[i]);
	    app_info->ecs.entity_add_component<SyTransform>(play_ctx->key_entities[i]);
	    app_info->ecs.entity_add_component<SyMaterial>(play_ctx->key_entities[i]);

	    SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(play_ctx->key_entities[i]);
	    SyTransform *transform = app_info->ecs.component<SyTransform>(play_ctx->key_entities[i]);
	    SyMaterial *material = app_info->ecs.component<SyMaterial>(play_ctx->key_entities[i]);

	    draw_info->should_draw = false;
	    draw_info->asset_metadata_id = key_mesh_metadata_index;

	    transform->position = glm::vec3(-3.0f + i * 2.0f, 0.0f, 0.0f);
	    transform->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	    transform->scale = glm::vec3(0.4f, 0.4f, 1.f);
	    
	    material->diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	}
    }

}

void play_start(SyAppInfo *app_info, DBSong song)
{
    PlayCtx *play_ctx = &g_state->play_ctx;

    // load song
    play_ctx->song = song;

    // get offset of arena before this game mode
    play_ctx->persistent_arena_starting_alloc = app_info->persistent_arena.m_current_offset;

    // allocate and fill title data and song data
    play_ctx->title_data = (char*)app_info->persistent_arena.alloc(strlen("Currently Playing: ") + strlen(song.name) + 1);
    sprintf(play_ctx->title_data, "Currently Playing: %s", song.name);
    app_info->ecs.component<SyUIText>(play_ctx->title)->text = play_ctx->title_data;

    // change keys length
    for (size_t i = 0; i < play_ctx->keys_amt; ++i)
    {
	SyTransform *transform = app_info->ecs.component<SyTransform>(play_ctx->key_entities[i]);
	transform->scale[2] = 10.0f * song.duration;
    }

    // make everything draw
    app_info->ecs.component<SyDrawInfo>(play_ctx->title)->should_draw = true;
    for (size_t i = 0; i < play_ctx->keys_amt; ++i)
    {
	app_info->ecs.component<SyDrawInfo>(play_ctx->key_entities[i])->should_draw = true;
    }

    { // set player position
	SyTransform *transform = app_info->ecs.component<SyTransform>(g_state->player);
	transform->position = glm::vec3(0.0f, 5.0f, 10.0f);
	transform->rotation = glm::vec3(-1.0f, 180.0f, 0.0f);
	transform->scale = glm::vec3(0.0f, 0.0f, 0.0f);
    }

}

void play_run(SyAppInfo *app_info)
{
    PlayCtx *play_ctx = &g_state->play_ctx;

    if (app_info->input_info.escape == SyKeyState::released)
    {
	play_stop(app_info);
	g_state->game_mode = GameMode::menu;
	menu_start(app_info);
	return;
    }

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
	
	if (player_transform->position[2] / -10 + 1 > play_ctx->song.duration)
	    player_transform->position[2] = (play_ctx->song.duration - 1) * -10.0f;

	app_info->camera_settings.perspective_settings.aspect_ratio = (float)app_info->input_info.window_width / app_info->input_info.window_height;
    }

}
void play_stop(SyAppInfo *app_info)
{
    PlayCtx *play_ctx = &g_state->play_ctx;

    // hide_everything
    app_info->ecs.component<SyDrawInfo>(play_ctx->title)->should_draw = false;
    for (size_t i = 0; i < play_ctx->keys_amt; ++i)
    {
	app_info->ecs.component<SyDrawInfo>(play_ctx->key_entities[i])->should_draw = false;
    }
}

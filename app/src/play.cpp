#include "play.hpp"
#include "db.hpp"
#include "global.hpp"
#include "menu.hpp"
#include "sound/types/sy_audio_info.hpp"
#include "text_display.hpp"
#include "render/types/sy_ui_text.hpp"

void play_load(SyAppInfo *app_info)
{
    PlayCtx *play_ctx = &g_state->play_ctx;

    play_ctx->note_asset_metadata_id = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "app/note.obj", SyAssetType::mesh);

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
	ui_text->text = "Score: 0.0";
    }

    size_t hum_asset_metadata_id = SY_LOAD_ASSET_FROM_FILE(&app_info->sound_info, &app_info->ecs, "app/hum.wav", SyAssetType::audio);
    { // create keys
	size_t key_mesh_metadata_index = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "app/key.obj", SyAssetType::mesh);

	for (size_t i = 0; i < play_ctx->keys_amt; ++i)
	{
	    play_ctx->key_entities[i] = app_info->ecs.new_entity();
	    app_info->ecs.entity_add_component<SyDrawInfo>(play_ctx->key_entities[i]);
	    app_info->ecs.entity_add_component<SyTransform>(play_ctx->key_entities[i]);
	    app_info->ecs.entity_add_component<SyMaterial>(play_ctx->key_entities[i]);
	    SyAudioInfo *audio_info = app_info->ecs.entity_add_component<SyAudioInfo>(play_ctx->key_entities[i]);
	    audio_info->audio_asset_metadata_id = hum_asset_metadata_id;
	    audio_info->should_play = false;
	    audio_info->should_stop = false;
	    audio_info->gain = 0.5f;
	    audio_info->pitch = 1.f  + 0.25f * (i + 1);
	    audio_info->loop = true;
	    audio_info->needs_audio_state_generated = true;
	    

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

    {
	play_ctx->cursor = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(play_ctx->cursor);
	app_info->ecs.entity_add_component<SyTransform>(play_ctx->cursor);
	app_info->ecs.entity_add_component<SyMaterial>(play_ctx->cursor);
	
	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(play_ctx->cursor);
	SyTransform *transform = app_info->ecs.component<SyTransform>(play_ctx->cursor);
	SyMaterial *material = app_info->ecs.component<SyMaterial>(play_ctx->cursor);
	
	draw_info->should_draw = false;
	draw_info->asset_metadata_id = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "app/cube.obj", SyAssetType::mesh);
	
	transform->position = glm::vec3(0.0f, 0.4f, 0.0f);
	transform->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	transform->scale = glm::vec3(10.f, 0.3f, 0.3f);
	
	material->diffuse = glm::vec3(0.0f, 0.8f, 0.0f);

    }

    {
	play_ctx->score_label = app_info->ecs.new_entity();
	SyDrawInfo *draw_info = app_info->ecs.entity_add_component<SyDrawInfo>(play_ctx->score_label);
	draw_info->should_draw = false;
	draw_info->asset_metadata_id = g_state->font_asset_metadata_index;

	SyUIText *text = app_info->ecs.entity_add_component<SyUIText>(play_ctx->score_label);
	text->text = "You should not see this";
	text->alignment = SyTextAlignment::center;
	text->color = glm::vec3(1.0f, 1.0f, 0.0f);
	text->pos = glm::vec2(0.0f, -0.8);
	text->scale = glm::vec2(0.05f, 0.05f);
    }
}

void play_start(SyAppInfo *app_info, DBSong song)
{
    PlayCtx *play_ctx = &g_state->play_ctx;

    // load song
    play_ctx->song = song;

    play_ctx->time_running = 0.0f;
    play_ctx->score = 0.0f;
    for (size_t i = 0; i < play_ctx->keys_amt; ++i)
    {
	play_ctx->keys_playing[i] = false;
    }

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

    // retrieve notes
    size_t frame_arena_notes_alloc = app_info->frame_arena.m_current_offset;
    db_get_all_notes_from_song(g_state->db, play_ctx->song.id, NULL, &play_ctx->notes_amt);
    DBNote *notes = (DBNote*)app_info->frame_arena.alloc(play_ctx->notes_amt * sizeof(DBNote));
    play_ctx->notes = (EntityNote*)app_info->persistent_arena.alloc(play_ctx->notes_amt * sizeof(EntityNote));
    db_get_all_notes_from_song(g_state->db, play_ctx->song.id, notes, NULL);
    for (size_t i = 0; i < play_ctx->notes_amt; ++i)
    {
	play_ctx->notes[i].note = notes[i];
	SyEntityHandle entity = app_info->ecs.new_entity();
	play_ctx->notes[i].entity = entity;
	
	SyDrawInfo *draw_info = app_info->ecs.entity_add_component<SyDrawInfo>(entity);
	SyMaterial *material = app_info->ecs.entity_add_component<SyMaterial>(entity);
	SyTransform *transform = app_info->ecs.entity_add_component<SyTransform>(entity);
	
	draw_info->should_draw = true;
	draw_info->asset_metadata_id = play_ctx->note_asset_metadata_id;
	
	material->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	
	transform->scale = glm::vec3(0.35f, 0.35f, 10.0f * play_ctx->notes[i].note.duration);
	transform->position = glm::vec3(0.0f, 0.7f, -10.0f * play_ctx->notes[i].note.timestamp);
	transform->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	
	transform->position[0] = app_info->ecs.component<SyTransform>(play_ctx->key_entities[play_ctx->notes[i].note.key])->position[0];	

    }
    app_info->frame_arena.m_current_offset = frame_arena_notes_alloc;

    // make everything draw
    app_info->ecs.component<SyDrawInfo>(play_ctx->title)->should_draw = true;
    app_info->ecs.component<SyDrawInfo>(play_ctx->cursor)->should_draw = true;
    app_info->ecs.component<SyDrawInfo>(play_ctx->score_label)->should_draw = true;
    for (size_t i = 0; i < play_ctx->keys_amt; ++i)
    {
	app_info->ecs.component<SyDrawInfo>(play_ctx->key_entities[i])->should_draw = true;
    }

    { // set player position
	SyTransform *transform = app_info->ecs.component<SyTransform>(g_state->player);
	transform->position = glm::vec3(0.0f, 5.0f, 10.0f);
	transform->rotation = glm::vec3(-1.0f, 180.0f, 0.0f);
	transform->scale = glm::vec3(0.0f, 0.0f, 0.0f);

	transform->position[2] = (play_ctx->time_running - 1) * -10.0f;

	app_info->camera_settings.perspective_settings.aspect_ratio = (float)app_info->input_info.window_width / app_info->input_info.window_height;

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

    play_ctx->time_running += app_info->delta_time;

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
	

	player_transform->position[2] = (play_ctx->time_running - 1.5f) * -10.0f;
	app_info->ecs.component<SyTransform>(play_ctx->cursor)->position[2] = (play_ctx->time_running) * -10.0f;

	if (play_ctx->time_running > play_ctx->song.duration)
	{
	    play_stop(app_info);
	    g_state->game_mode = GameMode::text_display;
	    char *score_text = (char*)app_info->frame_arena.alloc(50 * sizeof(char));
	    snprintf(score_text, 20, "Final Score: %.2f", play_ctx->score);
	    text_dpy_start(app_info, score_text, 10.0f);
	    return;
	}   


	app_info->camera_settings.perspective_settings.aspect_ratio = (float)app_info->input_info.window_width / app_info->input_info.window_height;
    }

    {
	uint8_t key_input = 0;
	key_input |= (app_info->input_info.one == SyKeyState::pressed) << 0;
	key_input |= (app_info->input_info.two == SyKeyState::pressed) << 1;
	key_input |= (app_info->input_info.three == SyKeyState::pressed) << 2;
	key_input |= (app_info->input_info.four == SyKeyState::pressed) << 3;

	if (g_state->using_piano_device)
	    key_input |= ~(ft232h_get_gpio_state(&g_state->piano_device) & 0xFF);

	// play noise
	for (size_t i = 0; i < play_ctx->keys_amt; ++i)
	{
	    if ( (key_input & (1 << i)) && play_ctx->keys_playing[i] == false)
	    {
		play_ctx->keys_playing[i] = true;
		app_info->ecs.component<SyAudioInfo>(play_ctx->key_entities[i])->should_play = true;
	    }

	    if ( !(key_input & (1 << i)) && play_ctx->keys_playing[i] == true)
	    {
		play_ctx->keys_playing[i] = false;
		app_info->ecs.component<SyAudioInfo>(play_ctx->key_entities[i])->should_stop = true;
	    }
	}


	// color notes & apply score addition
	bool key_status[4] = {};
	for (size_t i = 0; i < play_ctx->notes_amt; ++i)
	{
	    EntityNote note = play_ctx->notes[i];

	    if (play_ctx->time_running > note.note.timestamp &&
		play_ctx->time_running < note.note.timestamp + note.note.duration)
	    {
		if (key_input & (1 << note.note.key))
		{
		    float contrib = 1.0f / note.note.duration;
		    SyMaterial *material = app_info->ecs.component<SyMaterial>(note.entity);
		    //material->diffuse[0] -= contrib * app_info->delta_time;
		    //material->diffuse[1] -= contrib * app_info->delta_time;
		    material->diffuse[2] -= contrib * app_info->delta_time;

		    key_status[note.note.key] = true;

		    play_ctx->score += 100.0f * app_info->delta_time;
		}
	    }
	}

	// color keys & apply score subtraction
	for (size_t i = 0; i < 4; ++i)
	{
	    SyMaterial *material = app_info->ecs.component<SyMaterial>(play_ctx->key_entities[i]);

	    if (key_status[i] == (key_input & (1 << i)))
	    {
		material->diffuse = glm::vec3(0.5, 0.5, 0.5);
	    }

	    if (key_status[i] == false && key_input & (1 << i))
	    {
		float contrib = 1.0f / 0.5f;
		material->diffuse[0] += contrib * app_info->delta_time;
		material->diffuse[1] -= contrib * app_info->delta_time;
		material->diffuse[2] -= contrib * app_info->delta_time;

		if (material->diffuse[0] > 1.0f)
		{
		    material->diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
		    play_ctx->score -= 100.0f * app_info->delta_time;
		}

	    }

	}

    }

    { // update score label
	char *score_text = (char*)app_info->frame_arena.alloc(20 * sizeof(char));
	snprintf(score_text, 20, "Score: %.1f", play_ctx->score);
	app_info->ecs.component<SyUIText>(play_ctx->score_label)->text = score_text;
    }

}

void play_stop(SyAppInfo *app_info)
{
    PlayCtx *play_ctx = &g_state->play_ctx;

    // Make notes shut up
    for (size_t i = 0; i < play_ctx->keys_amt; ++i)
    {
	SyAudioInfo *audio_info = app_info->ecs.component<SyAudioInfo>(play_ctx->key_entities[i]);
	audio_info->should_stop = true;
    }

    // destroy notes
    for (size_t i = 0; i < play_ctx->notes_amt; ++i)
    {
	app_info->ecs.destroy_entity(play_ctx->notes[i].entity);
    }

    // hide_everything
    app_info->ecs.component<SyDrawInfo>(play_ctx->title)->should_draw = false;
    app_info->ecs.component<SyDrawInfo>(play_ctx->cursor)->should_draw = false;
    app_info->ecs.component<SyDrawInfo>(play_ctx->score_label)->should_draw = false;
    for (size_t i = 0; i < play_ctx->keys_amt; ++i)
    {
	app_info->ecs.component<SyDrawInfo>(play_ctx->key_entities[i])->should_draw = false;
    }

    app_info->persistent_arena.m_current_offset = play_ctx->persistent_arena_starting_alloc;
}

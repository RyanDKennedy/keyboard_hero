#include "edit.hpp"
#include "db.hpp"
#include "global.hpp"
#include "menu.hpp"
#include "render/types/sy_asset_metadata.hpp"
#include "render/types/sy_material.hpp"
#include "util.hpp"

void reload_notes(SyAppInfo *app_info)
{
    EditCtx *edit_ctx = &g_state->edit_ctx;

    for (size_t i = 0; i < edit_ctx->notes_amt; ++i)
    {
	app_info->ecs.destroy_entity(edit_ctx->notes[i].entity);
    }
    
    // retrieve

    // reset persistent arena
    app_info->persistent_arena.m_current_offset = edit_ctx->persistent_arena_notes_alloc;	

    db_get_all_notes_from_song(g_state->db, edit_ctx->song.id, NULL, &edit_ctx->notes_amt);
    edit_ctx->notes = (EntityNote*)app_info->persistent_arena.alloc(sizeof(EntityNote) * edit_ctx->notes_amt);
    
    size_t frame_arena_offset = app_info->frame_arena.m_current_offset;
    
    DBNote *db_notes = (DBNote*)app_info->frame_arena.alloc(sizeof(DBNote) * edit_ctx->notes_amt);
    db_get_all_notes_from_song(g_state->db, edit_ctx->song.id, db_notes, NULL);
    
    // format and create entities
    for (size_t i = 0; i < edit_ctx->notes_amt; ++i)
    {
	edit_ctx->notes[i].note = db_notes[i];
	SyEntityHandle entity = app_info->ecs.new_entity();
	edit_ctx->notes[i].entity = entity;
	
	SyDrawInfo *draw_info = app_info->ecs.entity_add_component<SyDrawInfo>(entity);
	SyMaterial *material = app_info->ecs.entity_add_component<SyMaterial>(entity);
	SyTransform *transform = app_info->ecs.entity_add_component<SyTransform>(entity);
	
	draw_info->should_draw = true;
	draw_info->asset_metadata_id = edit_ctx->note_asset_metadata_id;
	
	material->diffuse = glm::vec3(1.0f, 0.5f, 1.0f);
	
	transform->scale = glm::vec3(0.35f, 0.35f, 10.0f * edit_ctx->notes[i].note.duration);
	transform->position = glm::vec3(0.0f, 0.7f, -10.0f * edit_ctx->notes[i].note.timestamp);
	transform->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	
	transform->position[0] = app_info->ecs.component<SyTransform>(edit_ctx->key_entities[edit_ctx->notes[i].note.key])->position[0];
    }
    
    app_info->frame_arena.m_current_offset = frame_arena_offset;

    // Find closest note and select it

    float current_player_time = app_info->ecs.component<SyTransform>(g_state->player)->position[2] / -10.0f + 1.0f;
    for (size_t i = 0; i < edit_ctx->notes_amt; ++i)
    {
	edit_ctx->currently_selected_note = i;

	if (edit_ctx->notes[i].note.timestamp <= current_player_time && edit_ctx->notes[i].note.timestamp + edit_ctx->notes[i].note.duration >= current_player_time)
	{
	    break;
	}

	if (edit_ctx->notes[i].note.timestamp >= current_player_time)
	    break;
    }

}

void edit_load(SyAppInfo *app_info)
{
    EditCtx *edit_ctx = &g_state->edit_ctx;

    edit_ctx->note_asset_metadata_id = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "app/note.obj", SyAssetType::mesh);

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
    edit_ctx->display_data_size = strlen("Song Duration: 0000.00s\nCurrent Player Time: 0000.00s") + 1;
    edit_ctx->display_data = (char*)app_info->persistent_arena.alloc(edit_ctx->display_data_size);
    app_info->ecs.component<SyUIText>(edit_ctx->display)->text = edit_ctx->display_data;
    snprintf(edit_ctx->display_data, edit_ctx->display_data_size, "Song Duration: %.2fs\nCurrent Player Time: %.2fs", edit_ctx->song.duration, 0.0);


    // change keys length
    for (size_t i = 0; i < edit_ctx->keys_amt; ++i)
    {
	SyTransform *transform = app_info->ecs.component<SyTransform>(edit_ctx->key_entities[i]);
	transform->scale = glm::vec3(0.4f, 0.4f, 10.0f * song.duration);
    }

    { // notes
	edit_ctx->currently_selected_note = 0;

	// retrieve
	edit_ctx->persistent_arena_notes_alloc = app_info->persistent_arena.m_current_offset;	
	db_get_all_notes_from_song(g_state->db, edit_ctx->song.id, NULL, &edit_ctx->notes_amt);
	edit_ctx->notes = (EntityNote*)app_info->persistent_arena.alloc(sizeof(EntityNote) * edit_ctx->notes_amt);
	
	size_t frame_arena_offset = app_info->frame_arena.m_current_offset;

	DBNote *db_notes = (DBNote*)app_info->frame_arena.alloc(sizeof(DBNote) * edit_ctx->notes_amt);
	db_get_all_notes_from_song(g_state->db, edit_ctx->song.id, db_notes, NULL);

	// format and create entities
	for (size_t i = 0; i < edit_ctx->notes_amt; ++i)
	{
	    edit_ctx->notes[i].note = db_notes[i];
	    SyEntityHandle entity = app_info->ecs.new_entity();
	    edit_ctx->notes[i].entity = entity;

	    SyDrawInfo *draw_info = app_info->ecs.entity_add_component<SyDrawInfo>(entity);
	    SyMaterial *material = app_info->ecs.entity_add_component<SyMaterial>(entity);
	    SyTransform *transform = app_info->ecs.entity_add_component<SyTransform>(entity);
	    
	    draw_info->should_draw = true;
	    draw_info->asset_metadata_id = edit_ctx->note_asset_metadata_id;

	    material->diffuse = glm::vec3(1.0f, 0.5f, 1.0f);

	    transform->scale = glm::vec3(0.35f, 0.35f, 10.0f * edit_ctx->notes[i].note.duration);
	    transform->position = glm::vec3(0.0f, 0.7f, -10.0f * edit_ctx->notes[i].note.timestamp);
	    transform->rotation = glm::vec3(0.0f, 0.0f, 0.0f);

	    transform->position[0] = app_info->ecs.component<SyTransform>(edit_ctx->key_entities[edit_ctx->notes[i].note.key])->position[0];
	}

	app_info->frame_arena.m_current_offset = frame_arena_offset;
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

    float current_player_time = app_info->ecs.component<SyTransform>(g_state->player)->position[2] / -10.0f + 1.0f;

    // Change song duration
    if (app_info->input_info.p == SyKeyState::pressed || app_info->input_info.o == SyKeyState::pressed)
    {
	int sign = (app_info->input_info.p == SyKeyState::pressed) - (app_info->input_info.o == SyKeyState::pressed);
	
	edit_ctx->song.duration += sign * app_info->delta_time;

	if (edit_ctx->song.duration < 1.0f)
	    edit_ctx->song.duration = 1.0f;

	// Change keys length
	for (size_t i = 0; i < edit_ctx->keys_amt; ++i)
	{
	    SyTransform *transform = app_info->ecs.component<SyTransform>(edit_ctx->key_entities[i]);
	    transform->scale = glm::vec3(0.4f, 0.4f, 10.0f * edit_ctx->song.duration);
	}
    }
    if (app_info->input_info.p == SyKeyState::released ||
	app_info->input_info.o == SyKeyState::released)
    {
	db_update_song(g_state->db, edit_ctx->song);	
    }


    // delete notes
    if (app_info->input_info.q == SyKeyState::released && edit_ctx->notes_amt > 0)
    {
	db_delete_note(g_state->db, edit_ctx->notes[edit_ctx->currently_selected_note].note.id);
	reload_notes(app_info);
    }
    
    // add notes
    if (app_info->input_info.e == SyKeyState::released)
    {
	DBNote new_note = db_create_note(g_state->db, edit_ctx->song.id, 0, current_player_time, 1.0f);
	reload_notes(app_info);
	for (size_t i = 0; i < edit_ctx->notes_amt; ++i)
	{
	    if (new_note.id == edit_ctx->notes[i].note.id)
	    {
		edit_ctx->currently_selected_note = i;
		break;
	    }
	}
    }

    // change selected note
    if (app_info->input_info.a == SyKeyState::released && edit_ctx->currently_selected_note > 0)
	--edit_ctx->currently_selected_note;
    if (app_info->input_info.d == SyKeyState::released && edit_ctx->currently_selected_note < edit_ctx->notes_amt - 1)
	++edit_ctx->currently_selected_note;

    // change note key
    if (app_info->input_info.arrow_left == SyKeyState::released ||
	app_info->input_info.arrow_right == SyKeyState::released)
    {

	if (app_info->input_info.arrow_left == SyKeyState::released && edit_ctx->notes[edit_ctx->currently_selected_note].note.key > 0)
	    --edit_ctx->notes[edit_ctx->currently_selected_note].note.key;
	
	if (app_info->input_info.arrow_right == SyKeyState::released && edit_ctx->notes[edit_ctx->currently_selected_note].note.key < 3)
	    ++edit_ctx->notes[edit_ctx->currently_selected_note].note.key;
	
	db_update_note(g_state->db, edit_ctx->notes[edit_ctx->currently_selected_note].note);

	SyTransform *transform = app_info->ecs.component<SyTransform>(edit_ctx->notes[edit_ctx->currently_selected_note].entity);
	transform->position[0] = app_info->ecs.component<SyTransform>(edit_ctx->key_entities[edit_ctx->notes[edit_ctx->currently_selected_note].note.key])->position[0];

    }

    // change note length
    if (app_info->input_info.space == SyKeyState::pressed)
    {
	edit_ctx->notes[edit_ctx->currently_selected_note].note.duration += 0.5 * app_info->delta_time;
	app_info->ecs.component<SyTransform>(edit_ctx->notes[edit_ctx->currently_selected_note].entity)->scale[2] = 10 * edit_ctx->notes[edit_ctx->currently_selected_note].note.duration;
    }
    if (app_info->input_info.shift_left == SyKeyState::pressed)
    {
	edit_ctx->notes[edit_ctx->currently_selected_note].note.duration -= 0.5 * app_info->delta_time;

	if (edit_ctx->notes[edit_ctx->currently_selected_note].note.duration < 0.05)
	    edit_ctx->notes[edit_ctx->currently_selected_note].note.duration = 0.05;

	app_info->ecs.component<SyTransform>(edit_ctx->notes[edit_ctx->currently_selected_note].entity)->scale[2] = 10 * edit_ctx->notes[edit_ctx->currently_selected_note].note.duration;
    }
    if (app_info->input_info.space == SyKeyState::released || app_info->input_info.shift_left == SyKeyState::released)
    {
	db_update_note(g_state->db, edit_ctx->notes[edit_ctx->currently_selected_note].note);
    }

    // change note position
    if (app_info->input_info.arrow_up == SyKeyState::pressed)
    {
	edit_ctx->notes[edit_ctx->currently_selected_note].note.timestamp += 0.5 * app_info->delta_time;
	app_info->ecs.component<SyTransform>(edit_ctx->notes[edit_ctx->currently_selected_note].entity)->position[2] = -10 * edit_ctx->notes[edit_ctx->currently_selected_note].note.timestamp;
    }
    if (app_info->input_info.arrow_down == SyKeyState::pressed)
    {
	edit_ctx->notes[edit_ctx->currently_selected_note].note.timestamp -= 0.5 * app_info->delta_time;
	app_info->ecs.component<SyTransform>(edit_ctx->notes[edit_ctx->currently_selected_note].entity)->position[2] = -10 * edit_ctx->notes[edit_ctx->currently_selected_note].note.timestamp;
    }
    if (app_info->input_info.arrow_up == SyKeyState::released ||
	app_info->input_info.arrow_down == SyKeyState::released)
    {
	DBNote selected_note = edit_ctx->notes[edit_ctx->currently_selected_note].note;
	db_update_note(g_state->db, edit_ctx->notes[edit_ctx->currently_selected_note].note);
	reload_notes(app_info);
	for (size_t i = 0; i < edit_ctx->notes_amt; ++i)
	{
	    if (selected_note.id == edit_ctx->notes[i].note.id)
	    {
		edit_ctx->currently_selected_note = i;
		break;
	    }
	}
    }

    // set notes colors
    for (size_t i = 0; i < edit_ctx->notes_amt; ++i)
    {
	SyMaterial *material = app_info->ecs.component<SyMaterial>(edit_ctx->notes[i].entity);
	material->diffuse = glm::vec3(1.0f, 0.5f, 1.0f);

	if (i == edit_ctx->currently_selected_note)
	    material->diffuse = glm::vec3(1.0f, 1.0f, 0.0f);
    }

    // update display
    snprintf(edit_ctx->display_data, edit_ctx->display_data_size, "Song Duration: %.2fs\nCurrent Player Time: %.2fs", edit_ctx->song.duration, current_player_time);

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

    for (size_t i = 0; i < edit_ctx->notes_amt; ++i)
    {
	app_info->ecs.destroy_entity(edit_ctx->notes[i].entity);
    }

    app_info->ecs.component<SyDrawInfo>(edit_ctx->title)->should_draw = false;
    app_info->ecs.component<SyDrawInfo>(edit_ctx->display)->should_draw = false;
    for (size_t i = 0; i < edit_ctx->keys_amt; ++i)
    {
	app_info->ecs.component<SyDrawInfo>(edit_ctx->key_entities[i])->should_draw = false;
    }

    app_info->persistent_arena.m_current_offset = edit_ctx->persistent_arena_starting_alloc;
}

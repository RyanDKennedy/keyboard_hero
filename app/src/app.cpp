// TODO: give reloadable functions declspec for windows os dll support

#include <stdio.h>

#include "global.hpp"
#include "util.hpp"

#include "menu.hpp"
#include "edit.hpp"
#include "create.hpp"

void register_ecs_components(SyEcs *ecs)
{
    SY_ECS_REGISTER_TYPE(*ecs, SyAssetMetadata);
    SY_ECS_REGISTER_TYPE(*ecs, SyDrawInfo);
    SY_ECS_REGISTER_TYPE(*ecs, SyTransform);
    SY_ECS_REGISTER_TYPE(*ecs, SyMaterial);
    SY_ECS_REGISTER_TYPE(*ecs, SyUIText);
}

extern "C"
void app_init(SyAppInfo *app_info)
{
    register_ecs_components(&app_info->ecs);
    app_info->global_mem = app_info->persistent_arena.alloc(sizeof(Global));
    g_state = (Global*)app_info->global_mem;
    new (g_state) Global;

    // Init db
    db_init(&g_state->db);

    g_state->game_mode = GameMode::menu;

    { // Player creation
	g_state->player = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyTransform>(g_state->player);

	SyTransform *transform = app_info->ecs.component<SyTransform>(g_state->player);
	transform->position = glm::vec3(0.0f, 0.0f, 0.0f);
	transform->rotation = glm::vec3(0.0f, 180.0f, 0.0f);
	transform->scale = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    { // Camera configuration
	app_info->camera_settings.active_camera = g_state->player;

	app_info->camera_settings.projection_type = SyCameraProjectionType::perspective;
	app_info->camera_settings.perspective_settings.aspect_ratio = (float)app_info->input_info.window_width / app_info->input_info.window_width;
	app_info->camera_settings.perspective_settings.far_plane = 100.f;
	app_info->camera_settings.perspective_settings.near_plane = 0.1f;
	app_info->camera_settings.perspective_settings.fov = 65.f;
    }

    g_state->font_asset_metadata_index = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "fonts/UbuntuMono-R.ttf", SyAssetType::font);

    menu_load(app_info);
    edit_load(app_info);
    create_load(app_info);

    menu_start(app_info);
}

extern "C"
void app_run(SyAppInfo *app_info)
{
    g_state = (Global*)app_info->global_mem;

    if (app_info->input_info.forward_slash == SyKeyState::pressed)
	printf("FPS: %f\n", 1.0f / app_info->delta_time);

    //orthographic_movement(app_info, 5.0f, 5.0f, 5.0f);
    //perspective_movement(app_info, 5.0f, 5.0f);

    switch(g_state->game_mode)
    {
	case GameMode::none: // this is debug scenario
	    break;

	case GameMode::menu:
	    menu_run(app_info);
	    break;

	case GameMode::edit:
	    edit_run(app_info);
	    break;

	case GameMode::create:
	    create_run(app_info);
	    break;

	case GameMode::picker:
	    break;

	case GameMode::play:
	    break;
    }
}

extern "C"
void app_destroy(SyAppInfo *app_info)
{
    g_state = (Global*)app_info->global_mem;
    g_state->~Global();

    db_close(&g_state->db);
}


#ifndef NDEBUG
// When I reload the dll this will run before the dll is reloaded
extern "C"
void app_dll_exit(SyAppInfo *app_info)
{
    g_state = (Global*)app_info->global_mem;

    db_close(&g_state->db);

    SY_OUTPUT_DEBUG("old dll exit")
    
}

// When I reload the dll this will run instead of app_init, this will not run on the initial start
extern "C"
void app_dll_init(SyAppInfo *app_info)
{
    g_state = (Global*)app_info->global_mem;

    register_ecs_components(&app_info->ecs);

    db_init(&g_state->db);

    SY_OUTPUT_DEBUG("new dll init")
}
#endif

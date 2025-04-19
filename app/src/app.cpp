#include <stdio.h>

#include "global.hpp"
#include "util.hpp"

#include "menu.hpp"

void register_ecs_components(SyEcs *ecs)
{
    SY_ECS_REGISTER_TYPE(*ecs, SyAssetMetadata);
    SY_ECS_REGISTER_TYPE(*ecs, SyDrawInfo);
    SY_ECS_REGISTER_TYPE(*ecs, SyTransform);
    SY_ECS_REGISTER_TYPE(*ecs, SyMaterial);
}

extern "C"
void app_init(SyAppInfo *app_info)
{
    register_ecs_components(&app_info->ecs);

    app_info->global_mem = app_info->persistent_arena.alloc(sizeof(Global));
    g_state = (Global*)app_info->global_mem;

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
	app_info->camera_settings.perspective_settings.aspect_ratio = (float)app_info->input_info.window_width / app_info->input_info.window_height;
	app_info->camera_settings.perspective_settings.far_plane = 100.f;
	app_info->camera_settings.perspective_settings.near_plane = 0.1f;
	app_info->camera_settings.perspective_settings.fov = 65.f;

/*
	app_info->camera_settings.projection_type = SyCameraProjectionType::orthographic;
	app_info->camera_settings.orthographic_settings.top = 10.0f;
	app_info->camera_settings.orthographic_settings.bottom = -10.0f;
	app_info->camera_settings.orthographic_settings.near = -50.0f;
	app_info->camera_settings.orthographic_settings.far = 50.0f;
*/
    }

    menu_load(app_info);

    menu_start(app_info);
}

extern "C"
void app_run(SyAppInfo *app_info)
{
    g_state = (Global*)app_info->global_mem;

    if (app_info->input_info.escape == SyKeyState::released)
	app_info->stop_game = true;

    if (app_info->input_info.p == SyKeyState::pressed)
	printf("FPS: %f\n", 1.0f / app_info->delta_time);

    if (app_info->input_info.r == SyKeyState::pressed)
    {
	SyTransform *transform = app_info->ecs.component<SyTransform>(g_state->player);
	print_transform("", transform);
    }

    //orthographic_movement(app_info, 5.0f, 5.0f, 5.0f);
    perspective_movement(app_info, 5.0f, 5.0f);

    if (g_state->game_mode == GameMode::menu)
	menu_run(app_info);
}

extern "C"
void app_destroy(SyAppInfo *app_info)
{
    g_state = (Global*)app_info->global_mem;
}


#ifndef NDEBUG
// When I reload the dll this will run instead of app_init, this will not run on the initial start
extern "C"
void app_dll_init(SyAppInfo *app_info)
{
    g_state = (Global*)app_info->global_mem;

    register_ecs_components(&app_info->ecs);

    SY_OUTPUT_DEBUG("new dll init")
}

// When I reload the dll this will run before the dll is reloaded
extern "C"
void app_dll_exit(SyAppInfo *app_info)
{
    g_state = (Global*)app_info->global_mem;

    SY_OUTPUT_DEBUG("old dll exit")
    
}
#endif

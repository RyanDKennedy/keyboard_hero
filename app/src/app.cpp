#include <stdio.h>

#include "sy_ecs.hpp"
#include "sy_app_info.hpp"
#include "components/sy_transform.hpp"
#include "render/types/sy_material.hpp"
#include "render/types/sy_draw_info.hpp"
#include "render/types/sy_asset_metadata.hpp"
#include "types/sy_camera_settings.hpp"
#include "glm_include.hpp"

#include "global.hpp"
#include "util.hpp"

#ifndef NDEBUG
#define SY_LOAD_MESH_FROM_OBJ(render_info, ...) app_info->sy_load_mesh_from_obj((void*)(render_info), __VA_ARGS__);
#else
#include "asset_system/sy_asset_system.hpp"
#define SY_LOAD_MESH_FROM_OBJ(render_info, ...) sy_load_mesh_from_obj((SyRenderInfo*)(render_info), __VA_ARGS__);
#endif

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

    size_t play_metadata_id = SY_LOAD_MESH_FROM_OBJ(app_info->render_info, &app_info->ecs, "play.obj");
    size_t edit_metadata_id = SY_LOAD_MESH_FROM_OBJ(app_info->render_info, &app_info->ecs, "edit.obj");
    size_t create_metadata_id = SY_LOAD_MESH_FROM_OBJ(app_info->render_info, &app_info->ecs, "create.obj");
    for (size_t i = 0; i < g_state->buttons_amt; ++i)
    { // Entity square configuration
	g_state->buttons[i] = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(g_state->buttons[i]);
	app_info->ecs.entity_add_component<SyTransform>(g_state->buttons[i]);
	app_info->ecs.entity_add_component<SyMaterial>(g_state->buttons[i]);
	
	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(g_state->buttons[i]);
	draw_info->should_draw = true;
	
	SyTransform *transform = app_info->ecs.component<SyTransform>(g_state->buttons[i]);
	transform->position = glm::vec3(0.0f, ((float)g_state->buttons_amt / 2) - (1.0f * i), 0.0f);
	transform->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	transform->scale = glm::vec3(1.0f, 1.0f, 1.0f);

	SyMaterial *material = app_info->ecs.component<SyMaterial>(g_state->buttons[i]);
	material->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    }
    app_info->ecs.component<SyDrawInfo>(g_state->buttons[0])->asset_metadata_id = play_metadata_id;
    app_info->ecs.component<SyDrawInfo>(g_state->buttons[1])->asset_metadata_id = edit_metadata_id;
    app_info->ecs.component<SyDrawInfo>(g_state->buttons[2])->asset_metadata_id = create_metadata_id;
    g_state->selected_btn = 0;
    
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

    if (app_info->input_info.arrow_up == SyKeyState::released)
    {
	if (g_state->selected_btn > 0)
	    g_state->selected_btn -= 1;
	else
	    g_state->selected_btn = g_state->buttons_amt - 1;
    }
    if (app_info->input_info.arrow_down == SyKeyState::released)
    {
	g_state->selected_btn += 1;
	g_state->selected_btn %= g_state->buttons_amt;
    }

    // Buttons color
    for (size_t i = 0; i < g_state->buttons_amt; ++i)
    {
	SyMaterial *material = app_info->ecs.component<SyMaterial>(g_state->buttons[i]);
	material->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    }
    app_info->ecs.component<SyMaterial>(g_state->buttons[g_state->selected_btn])->diffuse = glm::vec3(1.0f, 1.0f, 0.0f);

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

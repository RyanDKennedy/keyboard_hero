#include <stdio.h>

#include "sy_ecs.hpp"
#include "sy_app_info.hpp"
#include "components/sy_transform.hpp"
#include "render/types/sy_draw_info.hpp"
#include "render/types/sy_asset_metadata.hpp"
#include "glm_include.hpp"

#include "global.hpp"

#ifndef NDEBUG
#define SY_LOAD_MESH_FROM_OBJ(render_info, ...) app_info->sy_load_mesh_from_obj((void*)render_info, __VA_ARGS__);
#else
#include "asset_system/sy_asset_system.hpp"
#define SY_LOAD_MESH_FROM_OBJ(render_info, ...) sy_load_mesh_from_obj((SyRenderInfo*)render_info, __VA_ARGS__);
#endif

void register_ecs_components(SyEcs *ecs)
{
    SY_ECS_REGISTER_TYPE(*ecs, SyAssetMetadata);
    SY_ECS_REGISTER_TYPE(*ecs, SyDrawInfo);
    SY_ECS_REGISTER_TYPE(*ecs, SyTransform);
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
	app_info->camera_settings.aspect_ratio = (float)app_info->input_info.window_width / app_info->input_info.window_height;
	app_info->camera_settings.fov = 45.0f;
	app_info->camera_settings.near_plane = 0.1f;
	app_info->camera_settings.far_plane = 100.0f;
    }

    { // Entity square configuration
	g_state->entity_square = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(g_state->entity_square);
	app_info->ecs.entity_add_component<SyTransform>(g_state->entity_square);
	
	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(g_state->entity_square);
	draw_info->asset_metadata_id = SY_LOAD_MESH_FROM_OBJ(app_info->render_info, &app_info->ecs, "axis.obj");
	draw_info->should_draw = true;
	
	SyTransform *transform = app_info->ecs.component<SyTransform>(g_state->entity_square);
	transform->position = glm::vec3(0.0f, 0.0f, 0.0f);
	transform->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	transform->scale = glm::vec3(0.0f, 0.0f, 0.0f);
    }

}

extern "C"
void app_run(SyAppInfo *app_info)
{
    g_state = (Global*)app_info->global_mem;

    if (app_info->input_info.escape)
	app_info->stop_game = true;

    app_info->camera_settings.aspect_ratio = (float)app_info->input_info.window_width / app_info->input_info.window_height;

    SyTransform *player_transform = app_info->ecs.component<SyTransform>(g_state->player); 

    glm::vec3 front;
    {
 	front[0] = glm::sin(glm::radians((float)player_transform->rotation[1])) * glm::cos(glm::radians((float)player_transform->rotation[0]));
	front[1] = 0.0f;
	front[2] = glm::cos(glm::radians((float)player_transform->rotation[1])) * glm::cos(glm::radians((float)player_transform->rotation[0]));
	front = glm::normalize(front);
    }
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    glm::vec3 right = glm::cross(front, up);

    const float speed = 5.0f;
    if (app_info->input_info.w)
	player_transform->position += (float)(app_info->delta_time * speed) * front;

    if (app_info->input_info.s)
	player_transform->position -= (float)(app_info->delta_time * speed) * front;

    if (app_info->input_info.d)
	player_transform->position += (float)(app_info->delta_time * speed) * right;

    if (app_info->input_info.a)
	player_transform->position -= (float)(app_info->delta_time * speed) * right;

    if (app_info->input_info.space)
	player_transform->position += (float)(speed * app_info->delta_time) * up;

    if (app_info->input_info.shift_left)
	player_transform->position -= (float)(speed * app_info->delta_time) * up;

    const float rot_speed = 5.0f;

    if (app_info->input_info.mouse_dx != 0)
	player_transform->rotation[1] += app_info->delta_time * rot_speed * -app_info->input_info.mouse_dx;

    if (app_info->input_info.mouse_dy != 0)
	player_transform->rotation[0] += app_info->delta_time * rot_speed * -app_info->input_info.mouse_dy;





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

#include "util.hpp"

#include "components/sy_transform.hpp"
#include "render/types/sy_material.hpp"
#include "render/types/sy_draw_info.hpp"
#include "render/types/sy_asset_metadata.hpp"

glm::vec3 make_rgb_from_255(float r, float g, float b)
{
    return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
}

void orthographic_movement(SyAppInfo *app_info, float camera_speed, float zoom_speed, float move_speed)
{
    SyTransform *player_transform = app_info->ecs.component<SyTransform>(app_info->camera_settings.active_camera); 

    glm::vec3 front;
    {
 	front[0] = glm::sin(glm::radians((float)player_transform->rotation[1])) * glm::cos(glm::radians((float)player_transform->rotation[0]));
	front[1] = 0.0f;
	front[2] = glm::cos(glm::radians((float)player_transform->rotation[1])) * glm::cos(glm::radians((float)player_transform->rotation[0]));
	front = glm::normalize(front);
    }
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    glm::vec3 right = glm::cross(front, up);

    if (abs(app_info->input_info.mouse_dx) != 0)
	player_transform->rotation[1] += app_info->delta_time * camera_speed * -app_info->input_info.mouse_dx;
    
    if (abs(app_info->input_info.mouse_dy) != 0)
    {
	player_transform->rotation[0] -= app_info->delta_time * camera_speed * -app_info->input_info.mouse_dy;
	if (player_transform->rotation[0] < -85.0)
	    player_transform->rotation[0] = -85.0;
	
	if (player_transform->rotation[0] > 85.0)
	    player_transform->rotation[0] = 85.0;
    }    
    
    float aspect_ratio = (float)app_info->input_info.window_width / app_info->input_info.window_height;
    app_info->camera_settings.orthographic_settings.left = app_info->camera_settings.orthographic_settings.bottom * aspect_ratio;
    app_info->camera_settings.orthographic_settings.right = app_info->camera_settings.orthographic_settings.top * aspect_ratio;
}

void perspective_movement(SyAppInfo *app_info, float camera_speed, float move_speed)
{
    SyTransform *player_transform = app_info->ecs.component<SyTransform>(app_info->camera_settings.active_camera); 

    glm::vec3 front;
    {
 	front[0] = glm::sin(glm::radians((float)player_transform->rotation[1])) * glm::cos(glm::radians((float)player_transform->rotation[0]));
	front[1] = 0.0f;
	front[2] = glm::cos(glm::radians((float)player_transform->rotation[1])) * glm::cos(glm::radians((float)player_transform->rotation[0]));
	front = glm::normalize(front);
    }
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    glm::vec3 right = glm::cross(front, up);

    if (abs(app_info->input_info.mouse_dx) != 0)
	player_transform->rotation[1] += app_info->delta_time * camera_speed * -app_info->input_info.mouse_dx;
    
    if (abs(app_info->input_info.mouse_dy) != 0)
    {
	player_transform->rotation[0] += app_info->delta_time * camera_speed * -app_info->input_info.mouse_dy;
	if (player_transform->rotation[0] < -85.0)
	    player_transform->rotation[0] = -85.0;
	
	if (player_transform->rotation[0] > 85.0)
	    player_transform->rotation[0] = 85.0;
    }    
    
    if (app_info->input_info.w == SyKeyState::pressed)
	player_transform->position += (float)(app_info->delta_time * move_speed) * front;
    
    if (app_info->input_info.s == SyKeyState::pressed)
	player_transform->position -= (float)(app_info->delta_time * move_speed) * front;
    
    if (app_info->input_info.d == SyKeyState::pressed)
	player_transform->position += (float)(app_info->delta_time * move_speed) * right;
    
    if (app_info->input_info.a == SyKeyState::pressed)
	player_transform->position -= (float)(app_info->delta_time * move_speed) * right;
    
    if (app_info->input_info.space == SyKeyState::pressed)
	player_transform->position += (float)(move_speed * app_info->delta_time) * up;
    
    if (app_info->input_info.shift_left == SyKeyState::pressed)
	player_transform->position -= (float)(move_speed * app_info->delta_time) * up;
    
    app_info->camera_settings.perspective_settings.aspect_ratio = (float)app_info->input_info.window_width / app_info->input_info.window_height;
}

void print_transform(const char *prefix, SyTransform *transform)
{
    printf("%s pos(%f, %f, %f) rot(%f, %f, %f) scale(%f, %f, %f)\n", prefix, transform->position[0], transform->position[1], transform->position[2], transform->rotation[0], transform->rotation[1], transform->rotation[2], transform->scale[0], transform->scale[1], transform->scale[2]);
}


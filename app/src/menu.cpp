#include "menu.hpp"
#include "global.hpp"

void menu_load(SyAppInfo *app_info)
{

    size_t font_index = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf", SyAssetType::font);
    size_t font2_index = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "/usr/share/fonts/truetype/quicksand/Quicksand-Regular.ttf", SyAssetType::font);

    size_t play_metadata_id = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "play.obj", SyAssetType::mesh);
    size_t edit_metadata_id = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "edit.obj", SyAssetType::mesh);
    size_t create_metadata_id = SY_LOAD_ASSET_FROM_FILE(app_info->render_info, &app_info->ecs, "create.obj", SyAssetType::mesh);
    for (size_t i = 0; i < g_state->buttons_amt; ++i)
    {
	g_state->buttons[i] = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(g_state->buttons[i]);
	app_info->ecs.entity_add_component<SyTransform>(g_state->buttons[i]);
	app_info->ecs.entity_add_component<SyMaterial>(g_state->buttons[i]);
	
	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(g_state->buttons[i]);
	draw_info->should_draw = false;
	
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


    if (1)
    {
    SyEntityHandle text = app_info->ecs.new_entity();
    app_info->ecs.entity_add_component<SyDrawInfo>(text);
    SyDrawInfo *text_draw_info = app_info->ecs.component<SyDrawInfo>(text);
    text_draw_info->should_draw = true;
    text_draw_info->asset_metadata_id = font_index;
    }
    else
    {
    SyEntityHandle text = app_info->ecs.new_entity();
    app_info->ecs.entity_add_component<SyDrawInfo>(text);
    SyDrawInfo *text_draw_info = app_info->ecs.component<SyDrawInfo>(text);
    text_draw_info->should_draw = true;
    text_draw_info->asset_metadata_id = font2_index;
    }

}

void menu_start(SyAppInfo *app_info)
{
    g_state->selected_btn = 0;

    for (size_t i = 0; i < g_state->buttons_amt; ++i)
    {
	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(g_state->buttons[i]);
	draw_info->should_draw = true;
    }
}

void menu_run(SyAppInfo *app_info)
{
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
void menu_destroy(SyAppInfo *app_info)
{
    for (size_t i = 0; i < g_state->buttons_amt; ++i)
    {
	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(g_state->buttons[i]);
	draw_info->should_draw = false;
    }
}

#include "menu.hpp"
#include "global.hpp"

void menu_load(SyAppInfo *app_info)
{
    for (size_t i = 0; i < g_state->buttons_amt; ++i)
    {
	g_state->buttons[i] = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(g_state->buttons[i]);
	app_info->ecs.entity_add_component<SyUIText>(g_state->buttons[i]);
	SyDrawInfo *text_draw_info = app_info->ecs.component<SyDrawInfo>(g_state->buttons[i]);
	text_draw_info->should_draw = false;
	text_draw_info->asset_metadata_id = g_state->font_asset_metadata_index;
	SyUIText *ui_text = app_info->ecs.component<SyUIText>(g_state->buttons[i]);
	ui_text->alignment = SyTextAlignment::left;
	ui_text->color = glm::vec3(0.5f, 0.5f, 0.5f);
	ui_text->pos = glm::vec2(-0.9f, -0.6f + i * 0.15f);
	ui_text->scale = glm::vec2(0.1f, 0.1f);
    }
    app_info->ecs.component<SyUIText>(g_state->buttons[0])->text = "Start";
    app_info->ecs.component<SyUIText>(g_state->buttons[1])->text = "Create";
    app_info->ecs.component<SyUIText>(g_state->buttons[2])->text = "Edit";

    g_state->menu_title = app_info->ecs.new_entity();
    app_info->ecs.entity_add_component<SyDrawInfo>(g_state->menu_title);
    app_info->ecs.entity_add_component<SyUIText>(g_state->menu_title);
    SyDrawInfo *text_draw_info = app_info->ecs.component<SyDrawInfo>(g_state->menu_title);
    text_draw_info->should_draw = false;
    text_draw_info->asset_metadata_id = g_state->font_asset_metadata_index;
    SyUIText *ui_text = app_info->ecs.component<SyUIText>(g_state->menu_title);
    ui_text->alignment = SyTextAlignment::center;
    ui_text->color = glm::vec3(1.f, 1.f, 1.f);
    ui_text->pos = glm::vec2(0.f, -0.75f);
    ui_text->scale = glm::vec2(0.15f, 0.15f);
    ui_text->text = "Keyboard Hero";
    
}

void menu_start(SyAppInfo *app_info)
{
    g_state->selected_btn = 0;

    for (size_t i = 0; i < g_state->buttons_amt; ++i)
    {
	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(g_state->buttons[i]);
	draw_info->should_draw = true;
    }

    app_info->ecs.component<SyDrawInfo>(g_state->menu_title)->should_draw = true;
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
	SyUIText *ui_text = app_info->ecs.component<SyUIText>(g_state->buttons[i]);
	ui_text->color = glm::vec3(0.5f, 0.5f, 0.5f);
    }
    app_info->ecs.component<SyUIText>(g_state->buttons[g_state->selected_btn])->color = glm::vec3(1.0f, 1.0f, 0.0f);

    if (app_info->input_info.arrow_right == SyKeyState::released)
    {
	menu_stop(app_info);

	switch (g_state->selected_btn)
	{
	    case 0:
		g_state->game_mode = GameMode::play_menu;
		break;

	    case 1:
		g_state->game_mode = GameMode::create;
		break;

	    case 2:
		g_state->game_mode = GameMode::edit;
		break;
	}
    }

}
void menu_stop(SyAppInfo *app_info)
{
    for (size_t i = 0; i < g_state->buttons_amt; ++i)
    {
	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(g_state->buttons[i]);
	draw_info->should_draw = false;
    }

    app_info->ecs.component<SyDrawInfo>(g_state->menu_title)->should_draw = false;
}

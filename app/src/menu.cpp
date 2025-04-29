#include "menu.hpp"
#include "create.hpp"
#include "db.hpp"
#include "global.hpp"
#include "picker.hpp"

void menu_load(SyAppInfo *app_info)
{
    MenuCtx *menu_ctx = &g_state->menu_ctx;

    for (size_t i = 0; i < menu_ctx->buttons_amt; ++i)
    {
	menu_ctx->buttons[i] = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(menu_ctx->buttons[i]);
	app_info->ecs.entity_add_component<SyUIText>(menu_ctx->buttons[i]);
	SyDrawInfo *text_draw_info = app_info->ecs.component<SyDrawInfo>(menu_ctx->buttons[i]);
	text_draw_info->should_draw = false;
	text_draw_info->asset_metadata_id = g_state->font_asset_metadata_index;
	SyUIText *ui_text = app_info->ecs.component<SyUIText>(menu_ctx->buttons[i]);
	ui_text->alignment = SyTextAlignment::center;
	ui_text->color = glm::vec3(0.5f, 0.5f, 0.5f);
	ui_text->pos = glm::vec2(0.0f, -0.15f + i * 0.15f);
	ui_text->scale = glm::vec2(0.1f, 0.1f);
    }
    app_info->ecs.component<SyUIText>(menu_ctx->buttons[0])->text = "Play Song";
    app_info->ecs.component<SyUIText>(menu_ctx->buttons[1])->text = "Create Song";
    app_info->ecs.component<SyUIText>(menu_ctx->buttons[2])->text = "Edit Song";

    menu_ctx->menu_title = app_info->ecs.new_entity();
    app_info->ecs.entity_add_component<SyDrawInfo>(menu_ctx->menu_title);
    app_info->ecs.entity_add_component<SyUIText>(menu_ctx->menu_title);
    SyDrawInfo *text_draw_info = app_info->ecs.component<SyDrawInfo>(menu_ctx->menu_title);
    text_draw_info->should_draw = false;
    text_draw_info->asset_metadata_id = g_state->font_asset_metadata_index;
    SyUIText *ui_text = app_info->ecs.component<SyUIText>(menu_ctx->menu_title);
    ui_text->alignment = SyTextAlignment::center;
    ui_text->color = glm::vec3(1.f, 1.f, 1.f);
    ui_text->pos = glm::vec2(0.f, -0.575f);
    ui_text->scale = glm::vec2(0.15f, 0.15f);
    ui_text->text = "Keyboard Hero";
    
}

void menu_start(SyAppInfo *app_info)
{
    MenuCtx *menu_ctx = &g_state->menu_ctx;

    menu_ctx->selected_btn = 0;

    size_t songs_amt = 0;
    db_get_all_songs(g_state->db, NULL, &songs_amt);
    menu_ctx->able_to_play = songs_amt != 0;

    // draw stuff
    for (size_t i = 0; i < menu_ctx->buttons_amt; ++i)
    {
	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(menu_ctx->buttons[i]);
	draw_info->should_draw = true;
    }
    app_info->ecs.component<SyDrawInfo>(menu_ctx->menu_title)->should_draw = true;
}

void menu_run(SyAppInfo *app_info)
{
    MenuCtx *menu_ctx = &g_state->menu_ctx;


    if (app_info->input_info.arrow_up == SyKeyState::released)
    {
	if (menu_ctx->selected_btn > 0)
	    menu_ctx->selected_btn -= 1;
	else
	    menu_ctx->selected_btn = menu_ctx->buttons_amt - 1;
    }
    if (app_info->input_info.arrow_down == SyKeyState::released)
    {
	menu_ctx->selected_btn += 1;
	menu_ctx->selected_btn %= menu_ctx->buttons_amt;
    }

    if (menu_ctx->able_to_play == false)
	menu_ctx->selected_btn = 1;

    // Buttons color
    {
	for (size_t i = 0; i < menu_ctx->buttons_amt; ++i)
	{
	    SyUIText *ui_text = app_info->ecs.component<SyUIText>(menu_ctx->buttons[i]);
	    ui_text->color = glm::vec3(0.5f, 0.5f, 0.5f);
	}

	if (menu_ctx->able_to_play == false)
	{
	    app_info->ecs.component<SyUIText>(menu_ctx->buttons[0])->color = glm::vec3(0.2, 0.1, 0.1);
	    app_info->ecs.component<SyUIText>(menu_ctx->buttons[2])->color = glm::vec3(0.2, 0.1, 0.1);
	}

	app_info->ecs.component<SyUIText>(menu_ctx->buttons[menu_ctx->selected_btn])->color = glm::vec3(1.0f, 1.0f, 0.0f);
	
    }


    if (app_info->input_info.arrow_right == SyKeyState::released)
    {
	menu_stop(app_info);

	switch (menu_ctx->selected_btn)
	{
	    case 0:
		g_state->game_mode = GameMode::picker;
		picker_start(app_info, GameMode::play);
		break;

	    case 1:
		g_state->game_mode = GameMode::create;
		create_start(app_info);
		break;

	    case 2:
		g_state->game_mode = GameMode::picker;
		picker_start(app_info, GameMode::edit);
		break;
	}
	return;
    }

}
void menu_stop(SyAppInfo *app_info)
{
    MenuCtx *menu_ctx = &g_state->menu_ctx;

    for (size_t i = 0; i < menu_ctx->buttons_amt; ++i)
    {
	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(menu_ctx->buttons[i]);
	draw_info->should_draw = false;
    }

    app_info->ecs.component<SyDrawInfo>(menu_ctx->menu_title)->should_draw = false;
}

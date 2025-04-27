#include "create.hpp"
#include "edit.hpp"
#include "menu.hpp"

void create_load(SyAppInfo *app_info)
{
    CreateCtx *create_ctx = &g_state->create_ctx;

    {
	create_ctx->name_label_data_len = strlen("Enter Song Name (00/00):") + 1;
	create_ctx->name_label_data = (char*)app_info->persistent_arena.alloc(create_ctx->name_label_data_len);
	sprintf(create_ctx->name_label_data, "Enter Song Name (%.2d/%.2lu):", 0, create_ctx->name_data_len - 1);

	create_ctx->name_label = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(create_ctx->name_label);
	app_info->ecs.entity_add_component<SyUIText>(create_ctx->name_label);
	SyDrawInfo *draw_info = app_info->ecs.component<SyDrawInfo>(create_ctx->name_label);
	draw_info->should_draw = false;
	draw_info->asset_metadata_id = g_state->font_asset_metadata_index;
	SyUIText *ui_text = app_info->ecs.component<SyUIText>(create_ctx->name_label);
	ui_text->alignment = SyTextAlignment::center;
	ui_text->color = glm::vec3(1.f, 1.f, 1.f);
	ui_text->pos = glm::vec2(0.f, -0.25f);
	ui_text->scale = glm::vec2(0.06f, 0.06f);
	ui_text->text = create_ctx->name_label_data;

    }


    {
	create_ctx->name_data_len = 31;
	create_ctx->name_data = (char*)app_info->persistent_arena.alloc(create_ctx->name_data_len);

	create_ctx->name = app_info->ecs.new_entity();
	app_info->ecs.entity_add_component<SyDrawInfo>(create_ctx->name);
	app_info->ecs.entity_add_component<SyUIText>(create_ctx->name);
	SyDrawInfo *name_draw_info = app_info->ecs.component<SyDrawInfo>(create_ctx->name);
	name_draw_info->should_draw = false;
	name_draw_info->asset_metadata_id = g_state->font_asset_metadata_index;
	SyUIText *ui_text = app_info->ecs.component<SyUIText>(create_ctx->name);
	ui_text->alignment = SyTextAlignment::center;
	ui_text->color = glm::vec3(1.f, 1.f, 0.f);
	ui_text->pos = glm::vec2(0.f, 0.f);
	ui_text->scale = glm::vec2(0.06f, 0.06f);
	ui_text->text = create_ctx->name_data;
    }
}

void create_start(SyAppInfo *app_info)
{
    CreateCtx *create_ctx = &g_state->create_ctx;
    app_info->ecs.component<SyDrawInfo>(create_ctx->name)->should_draw = true;
    app_info->ecs.component<SyDrawInfo>(create_ctx->name_label)->should_draw = true;

    memset(create_ctx->name_data, 0, create_ctx->name_data_len);
    sprintf(create_ctx->name_label_data, "Enter Song Name (%.2lu/%.2lu):", strlen(create_ctx->name_data), create_ctx->name_data_len - 1);
}

void create_run(SyAppInfo *app_info)
{
    CreateCtx *create_ctx = &g_state->create_ctx;
    
    // Write to name buffer
    size_t text_buffer_len = strlen(app_info->input_info.text_buffer);
    size_t current_pos = strlen(create_ctx->name_data);
    for (size_t i = 0; i < text_buffer_len; ++i)
    {

	if (app_info->input_info.text_buffer[i] != '\b' && current_pos < create_ctx->name_data_len - 1)
	{
	    switch(app_info->input_info.text_buffer[i])
	    {
		case '"':
		case '\'':
		case 27: // escape character
		case ',':
		case '.':
		case ')':
		case '(':
		case ';':
		case '/':
		case '\\':
		case '\t':
		case ' ':
		case '\n':
		    continue;

		default:
		    break;
	    }

	    create_ctx->name_data[current_pos] = app_info->input_info.text_buffer[i];
	    create_ctx->name_data[current_pos + 1] = '\0';
	    ++current_pos;

	    continue;
	}

	if (app_info->input_info.text_buffer[i] == '\b' && current_pos > 0)
	{
	    --current_pos;
	    create_ctx->name_data[current_pos] = '\0';

	    continue;
	}
    }

    if (text_buffer_len != 0)
    {
	sprintf(create_ctx->name_label_data, "Enter Song Name (%.2lu/%.2lu):", strlen(create_ctx->name_data), create_ctx->name_data_len - 1);
    }

    if (app_info->input_info.escape == SyKeyState::released)
    {
	create_stop(app_info);
	g_state->game_mode = GameMode::menu;
	menu_start(app_info);
	return;
    }

    if (app_info->input_info.enter == SyKeyState::released && strlen(create_ctx->name_data) > 0)
    {
	char song_name[255] = {};
	strcpy(song_name, create_ctx->name_data);

	create_stop(app_info);
	g_state->game_mode = GameMode::edit;
	edit_start(app_info, song_name);

	return;
    }
}

void create_stop(SyAppInfo *app_info)
{
    CreateCtx *create_ctx = &g_state->create_ctx;

    app_info->ecs.component<SyDrawInfo>(create_ctx->name)->should_draw = false;
    app_info->ecs.component<SyDrawInfo>(create_ctx->name_label)->should_draw = false;

}



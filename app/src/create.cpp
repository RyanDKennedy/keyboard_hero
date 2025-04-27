#include "create.hpp"

void create_load(SyAppInfo *app_info)
{
    CreateCtx *create_ctx = &g_state->create_ctx;

    create_ctx->text_data_len = 6;
    create_ctx->text_data = (char*)app_info->persistent_arena.alloc(create_ctx->text_data_len);

    create_ctx->text = app_info->ecs.new_entity();
    app_info->ecs.entity_add_component<SyDrawInfo>(create_ctx->text);
    app_info->ecs.entity_add_component<SyUIText>(create_ctx->text);
    SyDrawInfo *text_draw_info = app_info->ecs.component<SyDrawInfo>(create_ctx->text);
    text_draw_info->should_draw = false;
    text_draw_info->asset_metadata_id = g_state->font_asset_metadata_index;
    SyUIText *ui_text = app_info->ecs.component<SyUIText>(create_ctx->text);
    ui_text->alignment = SyTextAlignment::left;
    ui_text->color = glm::vec3(1.f, 1.f, 1.f);
    ui_text->pos = glm::vec2(-1.f, -0.85f);
    ui_text->scale = glm::vec2(0.1f, 0.1f);
    ui_text->text = create_ctx->text_data;
}

void create_start(SyAppInfo *app_info)
{
    CreateCtx *create_ctx = &g_state->create_ctx;
    app_info->ecs.component<SyDrawInfo>(create_ctx->text)->should_draw = true;
}

void create_run(SyAppInfo *app_info)
{
    CreateCtx *create_ctx = &g_state->create_ctx;
    
    size_t text_buffer_len = strlen(app_info->input_info.text_buffer);
    size_t current_pos = strlen(create_ctx->text_data);
    for (size_t i = 0; i < text_buffer_len; ++i)
    {

	if (app_info->input_info.text_buffer[i] != '\b' && current_pos < create_ctx->text_data_len - 1)
	{
	    create_ctx->text_data[current_pos] = app_info->input_info.text_buffer[i];
	    create_ctx->text_data[current_pos + 1] = '\0';
	    ++current_pos;

	    continue;
	}

	if (app_info->input_info.text_buffer[i] == '\b' && current_pos > 0)
	{
	    --current_pos;
	    create_ctx->text_data[current_pos] = '\0';

	    continue;
	}
    }
}

void create_stop(SyAppInfo *app_info)
{
    CreateCtx *create_ctx = &g_state->create_ctx;

}



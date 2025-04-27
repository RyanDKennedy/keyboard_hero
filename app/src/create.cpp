#include "create.hpp"

void create_load(SyAppInfo *app_info)
{
    CreateCtx *create_ctx = &g_state->create_ctx;

    create_ctx->text_data = (char*)app_info->persistent_arena.alloc(256);

    create_ctx->text = app_info->ecs.new_entity();
    app_info->ecs.entity_add_component<SyDrawInfo>(create_ctx->text);
    app_info->ecs.entity_add_component<SyUIText>(create_ctx->text);
    SyDrawInfo *text_draw_info = app_info->ecs.component<SyDrawInfo>(create_ctx->text);
    text_draw_info->should_draw = false;
    text_draw_info->asset_metadata_id = g_state->font_asset_metadata_index;
    SyUIText *ui_text = app_info->ecs.component<SyUIText>(create_ctx->text);
    ui_text->alignment = SyTextAlignment::center;
    ui_text->color = glm::vec3(1.f, 1.f, 1.f);
    ui_text->pos = glm::vec2(0.f, -0.85f);
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
    
    strncat(create_ctx->text_data, app_info->input_info.text_buffer, 256 - strlen(create_ctx->text_data) - 1);
}

void create_stop(SyAppInfo *app_info)
{
    CreateCtx *create_ctx = &g_state->create_ctx;

}



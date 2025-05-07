#include "text_display.hpp"

#include "db.hpp"
#include "edit.hpp"
#include "global.hpp"
#include "menu.hpp"
#include "play.hpp"
#include "render/types/sy_draw_info.hpp"

void text_dpy_load(SyAppInfo *app_info)
{
    TextDpyCtx *text_dpy_ctx = &g_state->text_dpy_ctx;

    text_dpy_ctx->text = app_info->ecs.new_entity();
    SyDrawInfo *draw_info = app_info->ecs.entity_add_component<SyDrawInfo>(text_dpy_ctx->text);
    draw_info->should_draw = false;
    draw_info->asset_metadata_id = g_state->font_asset_metadata_index;

    SyUIText *ui_text = app_info->ecs.entity_add_component<SyUIText>(text_dpy_ctx->text);
    ui_text->text = "Default Text";
    ui_text->alignment = SyTextAlignment::center;
    ui_text->color = glm::vec3(1.0f, 1.0f, 0.0f);
    ui_text->pos = glm::vec2(0.0f, 0.0f);
    ui_text->scale = glm::vec2(0.1f, 0.1f);
}

void text_dpy_start(SyAppInfo *app_info, const char *text, float time)
{
    TextDpyCtx *text_dpy_ctx = &g_state->text_dpy_ctx;

    text_dpy_ctx->time_elapsed = 0.0f;
    text_dpy_ctx->total_time = time;
    text_dpy_ctx->persistent_arena_starting_offset = app_info->persistent_arena.m_current_offset;

    char *text_mem = (char*)app_info->persistent_arena.alloc((strlen(text) + 1) * sizeof(char));
    strcpy(text_mem, text);

    app_info->ecs.component<SyUIText>(text_dpy_ctx->text)->text = text_mem;

    app_info->ecs.component<SyDrawInfo>(text_dpy_ctx->text)->should_draw = true;

}

void text_dpy_run(SyAppInfo *app_info)
{
    TextDpyCtx *text_dpy_ctx = &g_state->text_dpy_ctx;

    text_dpy_ctx->time_elapsed += app_info->delta_time;
    if (text_dpy_ctx->time_elapsed > text_dpy_ctx->total_time ||
	app_info->input_info.escape == SyKeyState::released)
    {
	text_dpy_stop(app_info);
	g_state->game_mode = GameMode::menu;
	menu_start(app_info);
    }

}
void text_dpy_stop(SyAppInfo *app_info)
{
    TextDpyCtx *text_dpy_ctx = &g_state->text_dpy_ctx;

    app_info->persistent_arena.m_current_offset = text_dpy_ctx->persistent_arena_starting_offset;

    app_info->ecs.component<SyDrawInfo>(text_dpy_ctx->text)->should_draw = false;
}

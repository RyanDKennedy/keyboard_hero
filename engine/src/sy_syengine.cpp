#include "sy_syengine.hpp"

#include <stdio.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "components/sy_transform.hpp"

#include "sy_ecs.hpp"
#include "sy_macros.hpp"

#include "render/sy_buffer.hpp"
#include "render/sy_drawing.hpp"
#include "render/sy_render_info.hpp"
#include "render/types/sy_mesh.hpp"
#include "render/types/sy_material.hpp"
#include "render/types/sy_ui_text.hpp"
#include "render/types/sy_draw_info.hpp"
#include "render/types/sy_asset_metadata.hpp"
#include "render/sy_fonts.hpp"

#include "sound/sy_sound.hpp"
#include "sound/types/sy_audio.hpp"
#include "sound/types/sy_audio_info.hpp"
#include "sound/sy_play.hpp"

#include "asset_system/sy_asset_system.hpp"


#ifdef NDEBUG
extern "C"
void app_init(SyAppInfo *app_info);

extern "C"
void app_run(SyAppInfo *app_info);

extern "C"
void app_destroy(SyAppInfo *app_info);
#endif

void engine_init(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state)
{
    SY_OUTPUT_INFO("Starting Engine");

    app_info->stop_game = false;
    app_info->ecs.initialize();
    app_info->delta_time = 0.0;
    app_info->input_info = platform_info->input_info;
    SY_ERROR_COND(app_info->persistent_arena.initialize(4096) != 0, "Failed to allocate data for persistent arena.");
    SY_ERROR_COND(app_info->frame_arena.initialize(4096) != 0, "Failed to allocate data for frame arena.");

    // Register ECS Types
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyMesh);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyAssetMetadata);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyDrawInfo);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyTransform);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyMaterial);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyRenderImage);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyFont);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyUIText);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyAudio);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyAudioInfo);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyAudioState); // FIXME: this doesn't work because the app isn't supposed to be revealed to SyAudioSource but it is needed to create a source


/* AUDIO SYSTEM DESIGN IDEA

   NECESSITIES:
   * toggle between if audio should be paused
   * ability to have many sources
   * source is seperate from buffer data

   asset metadata
   {
    buffer - SyAudio
   }

   


 */

    // Init renderer
    sy_render_info_init(&platform_info->render_info, platform_info->input_info.window_width, platform_info->input_info.window_height);
    app_info->render_info = &platform_info->render_info;

    // Init sound system
    sy_sound_info_init(&engine_state->sound_info);

    SY_OUTPUT_INFO("Starting App");
#ifndef NDEBUG
#define ASSIGN_DEBUG_FUNCTION_POINTER(left, right) left = (decltype(left))(right);
    ASSIGN_DEBUG_FUNCTION_POINTER(app_info->sy_load_asset_from_file, sy_load_asset_from_file);
    platform_info->app_init(app_info);
#else
    app_init(app_info);
#endif

    // stop the game signal
    if (app_info->stop_game == true)
    {
	platform_info->end_engine = true;
    }
}

void engine_run(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state)
{
    app_info->frame_arena.free_all();
    app_info->input_info = platform_info->input_info;
    app_info->delta_time = (double)platform_info->delta_time / 1000000.0;

    SyInputInfo &input = app_info->input_info;

    // close the engine
    if (input.window_should_close)
    {
	platform_info->end_engine = true;
    }

#ifndef NDEBUG
    // THIS IS DLL HOT RELOAD STUFF, SO DON'T INCLUDE IN RELEASE VERSION
    
    // reload dlls for app
    if (input.f1 == SyKeyState::released)
    {
	platform_info->reload_dll = true;
    }

    // Run the app_dll_init function after dll has been reloaded 
    if (platform_info->dll_first_run == true)
    {
	platform_info->app_dll_init(app_info);
	platform_info->dll_first_run = false;
    }

    // run the app
    platform_info->app_run(app_info);

    // if the dll is about to be reloaded then run the app_dll_exit function
    if (platform_info->reload_dll == true)
    {
	platform_info->app_dll_exit(app_info);
    }

#else

    app_run(app_info);

#endif

    // stop the game signal
    if (app_info->stop_game == true)
    {
	platform_info->end_engine = true;
    }

    sy_render_draw(&platform_info->render_info, &platform_info->input_info, &app_info->ecs, &app_info->camera_settings);
    sy_sound_play(&engine_state->sound_info, &app_info->ecs, app_info->camera_settings.active_camera);

}

void engine_destroy(SyPlatformInfo *platform_info, SyAppInfo *app_info, SyEngineState *engine_state)
{
    SY_OUTPUT_INFO("Ending App");
#ifndef NDEBUG
    platform_info->app_destroy(app_info);
#else
    app_destroy(app_info);
#endif

    SY_OUTPUT_INFO("Ending Engine");

    // Cleanup meshes
    size_t mesh_component_index = app_info->ecs.get_type_id<SyMesh>();
    for (size_t i = 0; i < app_info->ecs.m_component_used_arr[mesh_component_index].size(); ++i)
    {
	if (app_info->ecs.is_component_index_used<SyMesh>(i) == true)
	{
	    sy_destroy_mesh_from_index(&platform_info->render_info, &app_info->ecs, i);
	}
    }

    // Cleanup fonts
    size_t font_component_index = app_info->ecs.get_type_id<SyFont>();
    for (size_t i = 0; i < app_info->ecs.m_component_used_arr[font_component_index].size(); ++i)
    {
	if (app_info->ecs.is_component_index_used<SyFont>(i) == true)
	{
	    sy_destroy_font_from_index(&platform_info->render_info, &app_info->ecs, i);
	}
    }

    sy_sound_info_deinit(&engine_state->sound_info);

    sy_render_info_deinit(&platform_info->render_info);

    app_info->ecs.destroy();

    { // Cleanup App Info
	// Arena/Allocation Cleanup
	app_info->persistent_arena.destroy();
	app_info->frame_arena.destroy();
    }


}

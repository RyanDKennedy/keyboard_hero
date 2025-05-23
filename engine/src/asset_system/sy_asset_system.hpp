#pragma once

#include <stdlib.h>

#include "render/sy_render_info.hpp"
#include "render/types/sy_asset_metadata.hpp"
#include "sound/sy_sound.hpp"
#include "sound/types/sy_audio.hpp"
#include "sy_ecs.hpp"


#include "sy_obj_parser.hpp"

size_t sy_load_asset_from_file(void *info, SyEcs *ecs, const char *asset_path, SyAssetType type);

size_t sy_load_mesh_from_obj(SyRenderInfo *render_info, SyEcs *ecs, const char *obj_path);
size_t sy_load_font_from_file(SyRenderInfo *render_info, SyEcs *ecs, const char *font_path);
size_t sy_load_audio_from_file(SySoundInfo *sound_info, SyEcs *ecs, const char *audio_path);

void sy_destroy_mesh_from_index(SyRenderInfo *render_info, SyEcs *ecs, size_t mesh_index);
void sy_destroy_font_from_index(SyRenderInfo *render_info, SyEcs *ecs, size_t font_index);
void sy_destroy_audio_from_index(SySoundInfo *sound_info, SyEcs *ecs, size_t audio_index);

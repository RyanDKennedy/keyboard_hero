#pragma once

#include <stdlib.h>

#include "render/sy_render_info.hpp"
#include "sy_ecs.hpp"

#include "sy_obj_parser.hpp"

size_t sy_load_mesh_from_obj(SyRenderInfo *render_info, SyEcs *ecs, const char *obj_path);

void sy_destroy_mesh_from_index(SyRenderInfo *render_info, SyEcs *ecs, size_t mesh_index);

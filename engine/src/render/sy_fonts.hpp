#pragma once

#include <unordered_map>

#include <vulkan/vulkan_core.h>
#include "glm_include.hpp"

#include "sy_ecs.hpp"
#include "sy_render_info.hpp"


struct SyFontCharacter
{
    glm::uvec2 tex_bottom_left;
    glm::uvec2 tex_top_right;
    glm::vec2 scale;
    glm::vec2 offset;
    float advance;
};

struct SyFont
{
    std::unordered_map<char, SyFontCharacter> character_map;
    size_t texture_index; // index into SyRenderImage with texture
    size_t character_max_width;
    glm::uvec2 texture_dimensions;
    float line_height;
};

// returns an asset metadata info index
SyFont sy_render_create_font(SyRenderInfo *render_info, SyEcs *ecs, const char *font_path, uint32_t texture_width, uint32_t texture_height, uint32_t character_width, const char *characters, uint32_t spacing);

void sy_render_destroy_font(SyRenderInfo *render_info, SyEcs *ecs, SyFont *font);

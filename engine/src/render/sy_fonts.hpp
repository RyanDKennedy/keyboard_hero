#pragma once

#include <unordered_map>

#include <vulkan/vulkan_core.h>
#include "glm_include.hpp"

#include "sy_render_info.hpp"


struct SyFontCharacter
{
    glm::uvec2 tex_bottom_left;
    glm::uvec2 tex_top_right;
};

struct SyFont
{
    std::unordered_map<char, SyFontCharacter> character_map;

    SyRenderImage atlas;

};

SyFont sy_render_create_font(SyRenderInfo *render_info, const char *font_path, uint32_t texture_width, uint32_t texture_height, uint32_t character_width, const char *characters, size_t side_padding);


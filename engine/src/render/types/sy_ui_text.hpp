#pragma once

#include "glm_include.hpp"

enum class SyTextAlignment
{
    left,
    center,
    right
};

struct SyUIText
{
    const char *text;
    glm::vec3 color;
    glm::vec2 pos;
    glm::vec2 scale;
    SyTextAlignment alignment;
};

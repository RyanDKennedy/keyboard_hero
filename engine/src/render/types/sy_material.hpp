#pragma once

#include <stdlib.h>
#include "glm_include.hpp"

struct SyMaterial
{
    alignas(16) glm::vec3 diffuse;
    alignas(16) glm::vec3 specular;
    alignas(16) glm::vec3 ambient;
};

struct SyMaterialComponent
{
    SyMaterial material;
    ssize_t descriptor_set_index; // used by the renderer, users shouldn't set this, if set to 0 then create descriptor set for this material
};

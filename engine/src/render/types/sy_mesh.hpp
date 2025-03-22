#pragma once

#include "render/sy_render_settings.hpp"
#include "render/sy_render_info.hpp"

struct SyMesh
{
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;
    uint32_t index_amt;
};


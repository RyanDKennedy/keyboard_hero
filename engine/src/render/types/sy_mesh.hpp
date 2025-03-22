#pragma once

#include "render/sy_render_settings.hpp"
#include "render/sy_render_info.hpp"

struct SyMesh
{
    VkBuffer vertex_buffer;
    VmaAllocation vertex_buffer_alloc;

    VkBuffer index_buffer;
    VmaAllocation index_buffer_alloc;
    uint32_t index_amt;
};


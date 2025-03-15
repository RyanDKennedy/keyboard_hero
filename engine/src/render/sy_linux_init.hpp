#pragma once

#include "sy_syengine.hpp"
#include "sy_macros.hpp"

#include "sy_linux_window.hpp"
#include "sy_linux_input.hpp"

int sy_render_init(SyXCBInfo *xcb_info, SyRenderInfo *render_info);

int sy_render_deinit(SyRenderInfo *render_info);

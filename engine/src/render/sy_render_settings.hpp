#pragma once

#include "sy_macros.hpp"

#if __linux__
#define VK_USE_PLATFORM_XCB_KHR
#define SY_RENDER_SURFACE_EXTENSION "VK_KHR_xcb_surface"
#endif

#ifndef NDEBUG
inline const char *sy_g_render_vulkan_extensions[] = {"VK_KHR_surface", SY_RENDER_SURFACE_EXTENSION, "VK_EXT_debug_utils"};
inline const int sy_g_render_use_validation_layers = 1;
#else
inline const char *sy_g_render_vulkan_extensions[] = {"VK_KHR_surface", SY_RENDER_SURFACE_EXTENSION};
inline const int sy_g_render_use_validation_layers = 0;
#endif

inline const char *sy_g_render_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};

inline const int sy_g_render_vulkan_extension_amt = SY_ARRLEN(sy_g_render_vulkan_extensions);
inline const int sy_g_render_vulkan_layer_amt = SY_ARRLEN(sy_g_render_validation_layers);

inline const char *sy_g_render_vulkan_device_extensions[] = {"VK_KHR_swapchain"};
inline const int sy_g_render_vulkan_device_extension_amt = SY_ARRLEN(sy_g_render_vulkan_device_extensions);



#include "sy_linux_init.hpp"
#include "render/sy_render_settings.hpp"

void create_instance(SyRenderInfo *render_info);
bool check_validation_layer_support();
bool check_extension_support();


int sy_render_init(SyXCBInfo *xcb_info, SyRenderInfo *render_info)
{
    SY_OUTPUT_INFO("VALIDATION LAYERS %d", sy_g_render_use_validation_layers);

    return 0;
}



#include "render/sy_render.hpp"

void sy_render_init_ecs(SyEcs *ecs)
{
    SY_ECS_REGISTER_TYPE((*ecs), SyRender);
    
}

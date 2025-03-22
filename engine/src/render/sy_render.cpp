#include "sy_render.hpp"

#include "sy_ecs.hpp"
#include "sy_opaque_types.hpp"

void sy_render_init_ecs(SyEcs *ecs)
{
    SY_ECS_REGISTER_TYPE((*ecs), SyMesh);
}



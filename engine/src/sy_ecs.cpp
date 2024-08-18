#include "sy_ecs.hpp"

void SyEcs::initialize()
{
    m_entity_used.initialize<bool>();
    m_entity_data.initialize<SyEntityData>();
}

void SyEcs::destroy()
{
    for (size_t i = 0; i < g_ecs_max_component_types; ++i)
    {
	if (m_component_used_arr[i].m_memory != nullptr)
	{
	    m_component_used_arr[i].destroy();
	    m_component_data_arr[i].destroy();
	}
    }

    m_entity_used.destroy();
    m_entity_data.destroy();
}

SyEntityHandle SyEcs::new_entity()
{
    // Find the lowest unused entity handle
    SyEntityHandle i = 0;
    for (;; ++i)
    {
	if (m_entity_used.get<bool>(i) == false)
	    break;
    }
    m_entity_used.get<bool>(i) = true;
    
    // Set the entity to 0.
    memset(&m_entity_data.get<SyEntityData>(i), 0, sizeof(SyEntityData));

    return i;
}

void SyEcs::destroy_entity(SyEntityHandle entity)
{
    SyEntityData &entity_data = m_entity_data.get<SyEntityData>(entity);

    for (size_t i = 0; i < g_ecs_max_component_types; ++i)
    {
	if (entity_data.mask[i] == true)
	{
	    const size_t type_id = i;

	    // entity_remove_component
	    SY_ASSERT(entity_data.mask[type_id] == true);
	    entity_data.mask[type_id] = false;

	    // release_component
	    const size_t index = entity_data.indices[type_id];
	    m_component_used_arr[type_id].get<bool>(index) = false;
	}

    }
}

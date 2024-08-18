/**
 * @file sy_ecs.hpp
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "sy_macros.hpp"

#include "sy_ambiguous_vector.hpp"

/**
 * @brief The maximum amount of different types of components. Feel free to edit this.
 */
inline const size_t g_ecs_max_component_types = 64;

typedef size_t SyEntityHandle;

/**
 * @brief Holds the information that the entities use to access their components.
 */
struct SyEntityData
{
    bool mask[g_ecs_max_component_types]; /**< A bitmask that shows which components are in use. */
    size_t indices[g_ecs_max_component_types]; /**< An array of indices into the different component arrays. */
};

/**
 * @brief The main resource managing class that controls the ecs.
 */
struct SyEcs
{
    /* These fields follow a common pattern.
     * 1. There is a vector for component/entity data.
     * 2. There is an associated vector of bools which states whether the data in the same index
     *    of the other array is currently being used (1 == used, 0 == unused).
     */

    // Component fields
    SyAmbiguousVector m_component_used_arr[g_ecs_max_component_types];
    SyAmbiguousVector m_component_data_arr[g_ecs_max_component_types];

    // SyEntity fields
    SyAmbiguousVector m_entity_used;
    SyAmbiguousVector m_entity_data;

    /**
     * @brief Initialize ecs.
     */
    void initialize();

    /**
     * @brief Destroy ecs.
     */
    void destroy();

    // Type methods

    /**
     * @brief You are not meant to use this, this is for converting types to unique IDs.
     */
    inline size_t get_new_type_id() const
    {
	static size_t last_id = 0;
	return last_id++;
    }
    
    /**
     * @brief You are not meant to use this, this is for converting types to unique IDs.
     */
    template <typename T>
    inline size_t get_type_id() const
    {
	static const size_t type_id = get_new_type_id();
	return type_id;
    }

    /**
     * @brief This makes the type compatible with the ecs, and adds it as a component.
     */
    template <typename T>
    void register_type()
    {
	size_t type_id = get_type_id<T>();

#ifndef NDEBUG
	if (type_id >= g_ecs_max_component_types)
	{
	    SY_ERROR_OUTPUT("ecs just registered type %lu, this is beyond limit.", type_id);	    
	}


	if (m_component_data_arr[type_id].m_memory == nullptr)
	{
	    m_component_data_arr[type_id].initialize<T>();
	    m_component_used_arr[type_id].initialize<bool>();
	}
	else
	{
	    SY_ERROR_OUTPUT("ecs already registered type %lu", type_id);
	}
#else
	m_component_data_arr[type_id].initialize<T>();
	m_component_used_arr[type_id].initialize<bool>();
#endif


    }

    // Component methods

    /**
     * @brief This gets an unused components index into it's component vector.
     * @return A index for the associated component vector to a unused component. Now marked as used.
     */
    template <typename T>
    size_t get_unused_component()
    {
	const size_t type_id = get_type_id<T>();

	// Get index
	size_t i = 0;
	for (;; ++i)
	{
	    if (m_component_used_arr[type_id].get<bool>(i) == false)
		break;
	}
	
	m_component_used_arr[type_id].get<bool>(i) = true;
	
	// Set the component to 0
	memset(&m_component_data_arr[type_id].get<T>(i), 0, sizeof(T));
	
	return i;
    }

    /**
     * @brief Makes an index into a component vector unused.
     * @index The index into the component vector.
     */
    template <typename T>
    void release_component(size_t index)
    {
	const size_t type_id = get_type_id<T>();
	m_component_used_arr[type_id].get<bool>(index) = false;

	// This is just for catching errors
#ifndef NDEBUG
	memset(&m_component_data_arr[type_id].get<T>(index), 0, sizeof(T));
#endif
    }

    /**
     * @brief Accesses the component that belongs to the entity.
     * @param entity The entity that you want to access the component of.
     * @return A reference to the specified component.
     */
    template<typename T>
    T& component(SyEntityHandle entity)
    {
	SyEntityData &entity_data = m_entity_data.get<SyEntityData>(entity);
	const size_t type_id = get_type_id<T>();
	SY_ASSERT(entity_data.mask[type_id] == true);
	return m_component_data_arr[type_id].get<T>(entity_data.indices[type_id]);
    }

    // SyEntity methods
    
    /**
     * @brief Gets a new unused entity.
     * @return A entity handle to the new entity.
     */
    SyEntityHandle new_entity();

    /**
     * @brief Destroys an entity, making it's ID reusable for the future.
     * @param entity The entity which you want to destroy.
     */
    void destroy_entity(SyEntityHandle entity);

    /**
     * @brief Adds a component to an entity.
     * @param entity The entity which you want to add the component to.
     */
    template <typename T>
    void entity_add_component(SyEntityHandle entity)
    {
	SyEntityData &entity_data = m_entity_data.get<SyEntityData>(entity);
	const size_t type_id = get_type_id<T>();

	SY_ASSERT(entity_data.mask[type_id] == false);
	entity_data.mask[type_id] = true;

	entity_data.indices[type_id] = get_unused_component<T>();
    }

    /**
     * @brief Removes a component from an entity.
     * @param entity The entity that you want the component removed from.
     */
    template <typename T>
    void entity_remove_component(SyEntityHandle entity)
    {
	SyEntityData &entity_data = m_entity_data.get<SyEntityData>(entity);
	const size_t type_id = get_type_id<T>();
	
	SY_ASSERT(entity_data.mask[type_id] == true);
	
	entity_data.mask[type_id] = false;
	release_component<T>(entity_data.indices[type_id]);
    }
};

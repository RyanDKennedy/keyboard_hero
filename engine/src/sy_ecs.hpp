/**
 * @file sy_ecs.hpp
 */

/* TODO:
 * Dynamically loaded compatibility
 * when the type is registered:
 *  check a hashmap to see if the type has been registered before.
 *  if it has hasn't been registered then store it inside a hashmap where key = name of type and value = type id
 *  if it has been registered then retrieve it's type id from the hash map and set that as it's type id
 * have a macro to register types
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sy_macros.hpp"

#include "sy_ambiguous_vector.hpp"

/**
 * @brief The maximum amount of different types of components. Feel free to edit this.
 */
inline const size_t g_sy_ecs_max_component_types = 64;

typedef size_t SyEntityHandle;

/**
 * @brief Holds the information that the entities use to access their components.
 */
struct SyEntityData
{
    bool mask[g_sy_ecs_max_component_types]; /**< A bitmask that shows which components are in use. */
    size_t indices[g_sy_ecs_max_component_types]; /**< An array of indices into the different component arrays. */
};

#define SY_ECS_REGISTER_TYPE(ecs, type_name)\
    ecs.register_type<type_name>(#type_name)

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
    SyAmbiguousVector m_component_used_arr[g_sy_ecs_max_component_types];
    SyAmbiguousVector m_component_data_arr[g_sy_ecs_max_component_types];

    // SyEntity fields
    SyAmbiguousVector m_entity_used;
    SyAmbiguousVector m_entity_data;

    // Types
    size_t m_current_type_id = 0;
    // type id is an index into these fields
    const char *m_registered_type_names[g_sy_ecs_max_component_types];
    size_t m_registered_type_ids[g_sy_ecs_max_component_types];

    

    /**
     * @brief Initialize ecs.
     */
    void initialize()
    {
	m_entity_used.initialize<bool>();
	m_entity_data.initialize<SyEntityData>();
	
	for (size_t i = 0; i < g_sy_ecs_max_component_types; ++i)
	{
	    m_registered_type_names[i] = "0";
	    m_registered_type_ids[i] = g_sy_ecs_max_component_types;
	}
    }
    
    /**
     * @brief Destroy ecs.
     */
    void destroy()
    {
	for (size_t i = 0; i < m_current_type_id; ++i)
	{
	    m_component_used_arr[i].destroy();
	    m_component_data_arr[i].destroy();
	}
	
	m_entity_used.destroy();
	m_entity_data.destroy();
    }
    
    // Type methods

    /**
     * @brief You are not meant to use this, this is for converting types to unique IDs.
     */
    template <typename T>
    size_t get_type_id_with_addr(size_t **type_id_addr)
    {
	static size_t type_id = m_current_type_id++;

	// I return the addr aswell so that you can change it later, however if you choose to change it then you must do "--m_current_type_id"
	// because if you don't then you will have an unused type id
	*type_id_addr = &type_id;

	return type_id;
    }

    template <typename T>
    size_t get_type_id()
    {
#ifndef NDEBUG
	// This makes sure that you have registered the type before you try to use it.
	size_t old_current_type_id = m_current_type_id;
	size_t *addr;
	size_t result =  get_type_id_with_addr<T>(&addr);
	SY_ERROR_COND(old_current_type_id != m_current_type_id, "ECS: You have tried to use a component which you have not registered.");
	return result;

#else
	size_t *addr;
	return get_type_id_with_addr<T>(&addr);
#endif

    }

    /**
     * @brief This makes the type compatible with the ecs, and adds it as a component.
     */
    template <typename T>
    void register_type(const char *type_name)
    {
	// See if type already exists
	bool type_found = false;
	size_t associated_type_id = 0;
	for (size_t i = 0; i < m_current_type_id + 1; ++i)
	{
	    if (strcmp(m_registered_type_names[i], type_name) == 0)
	    {
		type_found = true;
		associated_type_id = m_registered_type_ids[i];
		break;
	    }
	}

	// If the type already exists set the get_type_id<>() static variable to the already existing id
	if (type_found == true)
	{
	    size_t *addr;
	    get_type_id_with_addr<T>(&addr);
	    *addr = associated_type_id;
	    --m_current_type_id; // you do this as instructed by the get_type_id_with_addr() function
	    SY_OUTPUT_DEBUG("ECS: re-registered type %lu - %s", get_type_id<T>(), type_name);
	    return;
	}

	// If the type doesn't already exist get a new type id and register it with the already registered types and initialize it's data
	// NOTE: i use get_type_id_with_addr instead of get_type_id because get_type_id has checking if it hasn't registered that type yet.
	size_t *addr;
	associated_type_id = get_type_id_with_addr<T>(&addr);
	SY_ERROR_COND(associated_type_id >= g_sy_ecs_max_component_types, "ECS: just registered type %s - %lu, this is beyond limit. To fix this increase g_sy_ecs_max_component_types.", type_name, associated_type_id);

	m_registered_type_names[associated_type_id] = type_name;
	m_registered_type_ids[associated_type_id] = associated_type_id;

	m_component_data_arr[associated_type_id].initialize<T>();
	m_component_used_arr[associated_type_id].initialize<bool>();
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
	SY_ASSERT(entity_data.mask[type_id] == true && "ECS: You forgot to add the component to your entity.");
	return m_component_data_arr[type_id].get<T>(entity_data.indices[type_id]);
    }

    // SyEntity methods
    
    /**
     * @brief Gets a new unused entity.
     * @return A entity handle to the new entity.
     */
    SyEntityHandle new_entity()
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

    /**
     * @brief Destroys an entity, making it's ID reusable for the future.
     * @param entity The entity which you want to destroy.
     */
    void destroy_entity(SyEntityHandle entity)
    {
	SyEntityData &entity_data = m_entity_data.get<SyEntityData>(entity);
	
	for (size_t i = 0; i < g_sy_ecs_max_component_types; ++i)
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

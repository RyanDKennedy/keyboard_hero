#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "sy_macros.hpp"

/**
 * @brief A vector class which chooses to not store the contained type as part of it's type, and instead relies on you to know the type when you are accessing the vector.
 */
struct SyAmbiguousVector
{
    void *m_memory = nullptr;
    size_t m_element_size;
    size_t m_alloc_length;
    size_t m_filled_length;

    /**
     * @brief Initializes the vector and requires you to state what type this vector is going to actually contain.
     */
    template <typename T>
    void initialize()
    {
	m_element_size = sizeof(T);
	m_filled_length = 0;
	
	m_memory = calloc(2, m_element_size);
	SY_ERROR_COND(m_memory == NULL, "errno %s", strerror(errno));
	m_alloc_length = 2;
    };

    /**
     * @brief Destroys the vector.
     */
    void destroy()
    {
	free(m_memory);
    }

    /**
     * @brief Copies the data into a new patch of memory which can be a different size.
     * @param size The size of the new memory.
     */
    void reallocate(size_t size)
    {
	size_t old_alloc_length =  m_alloc_length;
	SY_ASSERT(m_memory != NULL);
	m_memory = reallocarray(m_memory, size, m_element_size);
	m_alloc_length = size;

	void *addr = (void*)((uint8_t*)m_memory + (old_alloc_length * m_element_size));
	memset(addr, 0, ((int)size - (int)old_alloc_length) * m_element_size);
    }

    /**
     * @brief Adds an element to the back of the array.
     * @param element The element that you want to add to the back of the array.
     */
    template <typename T> 
    void push_back(T element)
    {
	// Check if we are out of memory, if so allocate more
	++m_filled_length;
	if (m_filled_length > m_alloc_length)
	{
	    reallocate(m_alloc_length * 2);
	}
	
	const size_t fill_index = size() - 1;
	
	memcpy((uint8_t*)m_memory + (fill_index * m_element_size), &element, m_element_size);
    }


    /**
     * @brief Gets an index into the array. If the index is out of bounds, then the vector allocates enough space and fills it with elements for the index to be in bounds.
     * @param index The index which you want to access.
     * @return A reference to the requested element.
     */
    template <typename T>
    T& get(size_t index)
    {
	// Allocate blank elements if needed
	if (index >= m_filled_length)
	{
	    for (size_t i = m_filled_length; i < index + 1; ++i)
	    {
		T a = {};
		push_back<T>(a);
	    }
	}

	T *addr = (T*)((uint8_t*)m_memory + (index * m_element_size));
	return *addr;
    }

    /**
     * @brief Returns the amount of elements currently inside of the vector.
     * @return The amount of elements currently inside of the vector.
     */
    size_t size()
    {
	return m_filled_length;
    }

};

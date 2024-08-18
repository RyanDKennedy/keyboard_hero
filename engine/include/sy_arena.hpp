#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sy_macros.hpp"

/**
 * @brief A custom implementation of an arena allocator.
 * @details Memory gets set to 0 after being allocated.
 */
struct SyArena
{

    void *m_buffer; /**< A pointer to the start of the data. */
    size_t m_buffer_size; /**< The amount of data that it is possible to store inside the arena. */
    size_t m_current_offset; /**< The amount of data currently allocated and offset to where you should allocate new data. */
    
    /**
     * @brief Initializes the data for the arena. ex: mapping pages.
     * @param size The max amount of data that you want to be allocated from the arena.
     * @return Returns 0 on success. Non-zero on error.
     */
    int initialize(size_t size);

    /**
     * @brief Frees the data that the arena was using. ex: unmapping pages.
     */
    void destroy();

    /**
     * @brief Allocates aligned memory
     * @param size The amount of memory you want to allocate.
     * @param align The number you want the memory aligned on.
     * @return A pointer the the start of the allocated memory. NULL on error.
     */
    void* alloc_align(size_t size, size_t align);

    /**
     * @brief Allocates memory. Currently returns aligned memory.
     * @param size The amount of memory you want to allocate.
     * @return A point to the start of the newly allocated memory. NULL on error.
     */
    void* alloc(size_t size);

    /**
     * @brief Frees all of the memory allocated by the arena
     */
    void free_all();

private: 

    /**
     * @brief Aligns a pointer based on an alignment
     * @param ptr The pointer that you want aligned
     * @param align The alignment that you want the pointer aligned to
     * @return The aligned pointer.
     */
    uintptr_t align_forward(uintptr_t ptr, size_t align);

    /**
     * @brief Checks if a pointer is a power of 2.
     * @return true if ptr is a power of 2, and false otherwise.
     */
    bool is_power_of_two(uintptr_t ptr);

};

#include "sy_arena.hpp"

int SyArena::initialize(size_t size)
{
    m_buffer = (void*)malloc(size);
    m_buffer_size = size;
    m_current_offset = 0;

    if (m_buffer == NULL)
	return 1;

    return 0;
}

void SyArena::destroy()
{
    free(m_buffer);
}

void* SyArena::alloc_align(size_t size, size_t align)
{
    // Calculate offset into m_buffer for allocation
    uintptr_t current_ptr = (uintptr_t)m_buffer + (uintptr_t)m_current_offset;
    uintptr_t offset = align_forward(current_ptr, align) - (uintptr_t)m_buffer;

    // See if the allocation will fit
    SY_ASSERT(offset + size <= m_buffer_size);

    void *ptr = (uint8_t*)m_buffer + offset;
    m_current_offset = offset + size;
    memset(ptr, 0, size);
    return ptr;
}

void* SyArena::alloc(size_t size)
{
    return alloc_align(size, sizeof(void*) * 2);
}

void SyArena::free_all()
{
    m_current_offset = 0;
}

bool SyArena::is_power_of_two(uintptr_t ptr)
{
    return (ptr & (ptr - 1)) == 0;
}

uintptr_t SyArena::align_forward(uintptr_t ptr, size_t align)
{
    SY_ASSERT(is_power_of_two(align)); // alignment has to be power of 2

    uintptr_t a = (uintptr_t)align;

    // (p % a) but quicker since a is power of 2.
    const uintptr_t modulo = ptr & (a-1);

    // if not already aligned then move to next alignment
    if (modulo != 0)
    {
	ptr += a - modulo;
    }

    return ptr;

}



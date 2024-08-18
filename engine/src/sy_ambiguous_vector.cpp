#include "sy_ambiguous_vector.hpp"

SyAmbiguousVector::SyAmbiguousVector()
{
    m_memory = nullptr;
}

void SyAmbiguousVector::destroy()
{
    free(m_memory);
}

void SyAmbiguousVector::reallocate(size_t size)
{
    m_memory = realloc(m_memory, size * m_element_size);
    m_alloc_length = size;
}

size_t SyAmbiguousVector::size() const
{
    return m_filled_length;
}

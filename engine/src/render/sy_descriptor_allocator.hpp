#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

struct SyDescriptorAllocator
{
    VkDescriptorPool m_descriptor_pool;

    void init_pool(VkDevice logical_device, uint32_t max_sets, uint32_t descriptors_per_set);
    void destroy_pool(VkDevice logical_device);
    void clear_descriptors(VkDevice logical_device);

    VkDescriptorSet allocate(VkDevice logical_device, VkDescriptorSetLayout layout);

};

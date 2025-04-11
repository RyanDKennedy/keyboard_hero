#include "sy_descriptor_allocator.hpp"

#include <stdlib.h>
#include <vulkan/vulkan_core.h>
#include "sy_macros.hpp"


void SyDescriptorAllocator::init_pool(VkDevice logical_device, uint32_t max_sets, uint32_t descriptors_per_set)
{
    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = max_sets * descriptors_per_set;
    
    VkDescriptorPoolCreateInfo pool_create_info;
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.pNext = NULL;
    pool_create_info.flags = 0;
    pool_create_info.poolSizeCount = 1;
    pool_create_info.pPoolSizes = &pool_size;
    pool_create_info.maxSets = max_sets;
    
    SY_ERROR_COND(vkCreateDescriptorPool(logical_device, &pool_create_info, NULL, &m_descriptor_pool) != VK_SUCCESS, "RENDER: Failed to create descriptor pool.");
}

void SyDescriptorAllocator::clear_descriptors(VkDevice logical_device)
{
    SY_ERROR_COND(vkResetDescriptorPool(logical_device, m_descriptor_pool, 0) != VK_SUCCESS, "RENDER: Failed to clear descriptor set.");
}

void SyDescriptorAllocator::destroy_pool(VkDevice logical_device)
{
    vkDestroyDescriptorPool(logical_device, m_descriptor_pool, NULL);
}

VkDescriptorSet SyDescriptorAllocator::allocate(VkDevice logical_device, VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.pSetLayouts = &layout;
    allocate_info.descriptorSetCount = 1;
    allocate_info.descriptorPool = m_descriptor_pool;
    
    VkDescriptorSet result;

    VkResult status;
    SY_ERROR_COND((status = vkAllocateDescriptorSets(logical_device, &allocate_info, &result)) != VK_SUCCESS, "RENDER: Failed to allocate descriptor sets. out of pool mem: %d", status == VK_ERROR_OUT_OF_POOL_MEMORY);

    return result;
}

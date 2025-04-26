#include "sy_resources.hpp"

#include <stdlib.h>
#include <vulkan/vulkan_core.h>

#include "render/sy_render_info.hpp"
#include "sy_render_defines.hpp"
#include "sy_pipeline.hpp"
#include "sy_macros.hpp"



void sy_render_create_depth_resources(SyRenderInfo *render_info)
{
    render_info->depth_images = (VkImage*)calloc(render_info->swapchain_images_amt, sizeof(VkImage));
    render_info->depth_image_allocations = (VmaAllocation*)calloc(render_info->swapchain_images_amt, sizeof(VmaAllocation));
    render_info->depth_image_views = (VkImageView*)calloc(render_info->swapchain_images_amt, sizeof(VkImageView));

    VkFormat format;
    { // Find supported image format
	VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT};

	bool found_format = false;;

	for (int i = 0; i < SY_ARRLEN(candidates); ++i)
	{
	    VkFormat current_format = candidates[i];
	    VkFormatProperties props;
	    vkGetPhysicalDeviceFormatProperties(render_info->physical_device, current_format, &props);
	    
	    if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	    {
		format = current_format;
		found_format = true;
		break;
	    }
	}
	SY_ERROR_COND(found_format == false, "RENDER: Failed to find an available format for the depth buffer images.");
    }


    for (size_t i = 0; i < render_info->swapchain_images_amt; ++i)
    {
	VkImageCreateInfo image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.pNext = NULL;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = format;
	image_create_info.extent = VkExtent3D{.width = render_info->swapchain_image_extent.width, .height = render_info->swapchain_image_extent.height, .depth = 1};
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VmaAllocationCreateInfo alloc_create_info = {};
	alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	alloc_create_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	vmaCreateImage(render_info->vma_allocator, &image_create_info, &alloc_create_info, &render_info->depth_images[i], &render_info->depth_image_allocations[i], NULL);

	VkImageViewCreateInfo image_view_create_info = {};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.pNext = NULL;
	image_view_create_info.flags = 0;
	image_view_create_info.image = render_info->depth_images[i];
	image_view_create_info.format = format;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.levelCount = 1;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount = 1;

	SY_ERROR_COND(vkCreateImageView(render_info->logical_device, &image_view_create_info, NULL, &render_info->depth_image_views[i]) != VK_SUCCESS, "Failed to create depth image view %lu.", i);
    }

    
    render_info->depth_format = format;
    
}

SyRenderImage sy_render_create_texture_image(SyRenderInfo *render_info, void *data, VkExtent2D extent, VkFormat format, VkImageUsageFlags usage)
{
    size_t pixel_size;
    if (format == VK_FORMAT_R8G8B8A8_UNORM)
	pixel_size = 4;
    else if (format == VK_FORMAT_R8_UNORM)
	pixel_size = 1;
    else
    {
	SY_ERROR("format not supported by function");
    }

    size_t buffer_size = extent.width * extent.height * pixel_size;

    // Create Staging Buffer
    VkBuffer staging_buffer;
    VmaAllocation staging_buffer_alloc;
    {
	VkBufferCreateInfo buffer_info = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .pNext = NULL, .flags = 0};
	buffer_info.size = buffer_size;
	buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	
	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
	
	SY_ERROR_COND(vmaCreateBuffer(render_info->vma_allocator, &buffer_info, &alloc_info, &staging_buffer, &staging_buffer_alloc, nullptr) != VK_SUCCESS, "Failed to create staging buffer");
    }

    // Copy the vertices into the staging buffer
    vmaCopyMemoryToAllocation(render_info->vma_allocator, data, staging_buffer_alloc, 0, buffer_size);

    SyRenderImage image = sy_render_create_image(render_info, extent.width, extent.height, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    sy_render_transition_image(render_info, &image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkCommandBuffer command_buffer;
    {
	VkCommandBufferAllocateInfo command_buffer_allocate_info;
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.pNext = NULL;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = 1;
	command_buffer_allocate_info.commandPool = render_info->command_pool;
	
	SY_ERROR_COND(vkAllocateCommandBuffers(render_info->logical_device, &command_buffer_allocate_info, &command_buffer) != VK_SUCCESS, "RENDER: Failed to allocate command buffer.");
	
	// Record Command Buffer
	VkCommandBufferBeginInfo begin_info;
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin_info.pInheritanceInfo = NULL;
	
	SY_ERROR_COND(vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS, "RENDER: Failed to start command buffer.");
    }

    VkBufferImageCopy copy_region = {};
    copy_region.bufferOffset = 0;
    copy_region.bufferRowLength = 0;
    copy_region.bufferImageHeight = 0;
    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageExtent = VkExtent3D{.width = extent.width, .height = extent.height, .depth = 1};

    vkCmdCopyBufferToImage(command_buffer, staging_buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

    {
	SY_ERROR_COND(vkEndCommandBuffer(command_buffer) != VK_SUCCESS, "RENDER: Failed to end command buffer.");
	
	// Submit Buffer
	VkSubmitInfo submit_info;
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;
	submit_info.waitSemaphoreCount = 0;
	submit_info.pWaitSemaphores = NULL;
	submit_info.pWaitDstStageMask = NULL;
	submit_info.signalSemaphoreCount = 0;
	submit_info.pSignalSemaphores = NULL;
	
	SY_ERROR_COND(vkQueueSubmit(render_info->graphics_queue, 1, &submit_info, NULL) != VK_SUCCESS, "RENDER: Failed to submit command buffer.");
	
	vkQueueWaitIdle(render_info->graphics_queue);
	
	// Cleanup
	vkFreeCommandBuffers(render_info->logical_device, render_info->command_pool, 1, &command_buffer);
    }

    sy_render_transition_image(render_info, &image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vmaDestroyBuffer(render_info->vma_allocator, staging_buffer, staging_buffer_alloc);

    return image;
}

SyRenderImage sy_render_create_image(SyRenderInfo *render_info, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags image_usage)
{
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = format;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = image_usage;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VmaAllocationCreateInfo alloc_create_info = {};
    alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    alloc_create_info.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    
    VkImage image;
    VmaAllocation alloc;
    vmaCreateImage(render_info->vma_allocator, &image_create_info, &alloc_create_info, &image, &alloc, NULL);
    
    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.pNext = NULL;
    image_view_create_info.flags = 0;
    image_view_create_info.image = image;
    image_view_create_info.format = format;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    VkImageView image_view;

    SY_ERROR_COND(vkCreateImageView(render_info->logical_device, &image_view_create_info, NULL, &image_view) != VK_SUCCESS, "Failed to create image view.");

    SyRenderImage result;
    result.image = image;
    result.alloc = alloc;
    result.image_view = image_view;

    return result;
}

void sy_render_transition_image(SyRenderInfo *render_info, SyRenderImage *image, VkImageLayout old_layout, VkImageLayout new_layout)
{
    VkCommandBuffer command_buffer;
    {
	// Create Command Buffer
	VkCommandBufferAllocateInfo command_buffer_allocate_info;
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.pNext = NULL;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = 1;
	command_buffer_allocate_info.commandPool = render_info->command_pool;
	
	
	SY_ERROR_COND(vkAllocateCommandBuffers(render_info->logical_device, &command_buffer_allocate_info, &command_buffer) != VK_SUCCESS, "RENDER: Failed to allocate command buffer for image transition.");
	
	// Record Command Buffer
	VkCommandBufferBeginInfo begin_info;
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.pNext = NULL;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	begin_info.pInheritanceInfo = NULL;
	
	SY_ERROR_COND(vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS, "RENDER: Failed to start command buffer for image transition.");
    }

    VkImageMemoryBarrier image_barrier = {};
    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_barrier.pNext = NULL;

    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    image_barrier.oldLayout = old_layout;
    image_barrier.newLayout = new_layout;

    VkImageSubresourceRange subresource_range = {};
    subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource_range.baseMipLevel = 0;
    subresource_range.levelCount = VK_REMAINING_MIP_LEVELS;
    subresource_range.baseArrayLayer = 0;
    subresource_range.layerCount = VK_REMAINING_ARRAY_LAYERS;

    image_barrier.subresourceRange = subresource_range;
    image_barrier.image = image->image;

    VkPipelineStageFlags src_stage;
    VkPipelineStageFlags dst_stage;
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
	image_barrier.srcAccessMask = 0;
	image_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
	image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
	SY_ERROR("transition image - not valid new layout and old layout.");
    }
    
    vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &image_barrier);

    {
	SY_ERROR_COND(vkEndCommandBuffer(command_buffer) != VK_SUCCESS, "RENDER: Failed to end command buffer for image transition.");
	
	// Submit Buffer
	VkSubmitInfo submit_info;
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pNext = NULL;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;
	submit_info.waitSemaphoreCount = 0;
	submit_info.pWaitSemaphores = NULL;
	submit_info.pWaitDstStageMask = NULL;
	submit_info.signalSemaphoreCount = 0;
	submit_info.pSignalSemaphores = NULL;
	
	SY_ERROR_COND(vkQueueSubmit(render_info->graphics_queue, 1, &submit_info, NULL) != VK_SUCCESS, "RENDER: Failed to submit command buffer for image transition.");
	
	vkQueueWaitIdle(render_info->graphics_queue);
	
	// Cleanup
	vkFreeCommandBuffers(render_info->logical_device, render_info->command_pool, 1, &command_buffer);
    }
}

void sy_render_create_allocator(SyRenderInfo *render_info)
{
    VmaAllocatorCreateInfo vma_allocator_create_info = {};
    vma_allocator_create_info.flags = 0;
    vma_allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_0;
    vma_allocator_create_info.physicalDevice = render_info->physical_device;
    vma_allocator_create_info.device = render_info->logical_device;
    vma_allocator_create_info.instance = render_info->instance;
    vma_allocator_create_info.pVulkanFunctions = NULL;
    vmaCreateAllocator(&vma_allocator_create_info, &render_info->vma_allocator);
}

void sy_render_create_pipelines(SyRenderInfo *render_info)
{
    // Create Pipeline Layouts
    {
	VkDescriptorSetLayout layouts[] = {render_info->frame_descriptor_set_layout, render_info->material_descriptor_set_layout, render_info->object_descriptor_set_layout};
	
	VkPipelineLayoutCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.pushConstantRangeCount = 0;
	create_info.pPushConstantRanges = NULL;
	create_info.setLayoutCount = SY_ARRLEN(layouts);
	create_info.pSetLayouts = layouts;
	
	vkCreatePipelineLayout(render_info->logical_device, &create_info, NULL, &render_info->single_color_pipeline_layout);
    }

    { // text pipeline layout
	VkDescriptorSetLayout layouts[] = {render_info->character_map_descriptor_set_layout, render_info->character_information_descriptor_set_layout, render_info->text_buffer_descriptor_set_layout};
	
	VkPipelineLayoutCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.pushConstantRangeCount = 0;
	create_info.pPushConstantRanges = NULL;
	create_info.setLayoutCount = SY_ARRLEN(layouts);
	create_info.pSetLayouts = layouts;
	
	vkCreatePipelineLayout(render_info->logical_device, &create_info, NULL, &render_info->text_pipeline_layout);
    }

    { // Create single color pipeline
	VkVertexInputBindingDescription binding_description;
	binding_description.binding = 0;
	binding_description.stride = sizeof(float) * 3;
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;    

	VkVertexInputAttributeDescription attr[1];
	attr[0].binding = 0;
	attr[0].location = 0;
	attr[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attr[0].offset = 0;

	SyPipelineCreateInfo create_info;
	create_info.vertex_shader_path = "single_color/vertex.spv";
	create_info.fragment_shader_path = "single_color/fragment.spv";
	create_info.vertex_input_binding_descriptions = &binding_description;
	create_info.vertex_input_binding_descriptions_amt = 1;
	create_info.vertex_input_attribute_descriptions = attr;
	create_info.vertex_input_attribute_descriptions_amt = SY_ARRLEN(attr);
	create_info.render_type = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	create_info.subpass_number = 0;
	create_info.pipeline_layout = render_info->single_color_pipeline_layout;

	render_info->single_color_pipeline = sy_render_create_pipeline(render_info, &create_info);
    }

    { // Create text pipeline
	VkVertexInputBindingDescription binding_descriptions[1];
	binding_descriptions[0].binding = 0;
	binding_descriptions[0].stride = sizeof(float) * 2;
	binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;    

	VkVertexInputAttributeDescription attr[1];
	attr[0].binding = 0;
	attr[0].location = 0;
	attr[0].format = VK_FORMAT_R32G32_SFLOAT;
	attr[0].offset = 0;

	SyPipelineCreateInfo create_info = {};
	create_info.vertex_shader_path = "text/vertex.spv";
	create_info.fragment_shader_path = "text/fragment.spv";
	create_info.vertex_input_binding_descriptions = binding_descriptions;
	create_info.vertex_input_attribute_descriptions = attr;
	create_info.vertex_input_attribute_descriptions_amt = SY_ARRLEN(attr);
	create_info.vertex_input_binding_descriptions_amt = SY_ARRLEN(binding_descriptions);
	create_info.render_type = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	create_info.subpass_number = 0;
	create_info.pipeline_layout = render_info->text_pipeline_layout;

	render_info->text_pipeline = sy_render_create_pipeline(render_info, &create_info);
    }

}

/* FIXME:
void sy_render_create_descriptor_pool(SyRenderInfo *render_info)
{
    VkDescriptorPoolSize pool_size;
    pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = render_info->max_descriptor_sets_amt;
    
    VkDescriptorPoolCreateInfo pool_create_info;
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.pNext = NULL;
    pool_create_info.flags = 0;
    pool_create_info.poolSizeCount = 1;
    pool_create_info.pPoolSizes = &pool_size;
    pool_create_info.maxSets = render_info->max_descriptor_sets_amt;
    
    SY_ERROR_COND(vkCreateDescriptorPool(render_info->logical_device, &pool_create_info, NULL, &render_info->descriptor_pool) != VK_SUCCESS, "RENDER: Failed to create descriptor pool.");
    
    render_info->descriptor_sets = (SyDescriptorSetDataGroup*)calloc(render_info->max_descriptor_sets_amt, sizeof(SyDescriptorSetDataGroup));
    render_info->descriptor_sets_used = (bool*)calloc(render_info->max_descriptor_sets_amt, sizeof(bool));
    
}
*/

void sy_render_create_sync_objects(SyRenderInfo *render_info)
{
    render_info->image_available_semaphores = (VkSemaphore*)calloc(SY_RENDER_MAX_FRAMES_IN_FLIGHT, sizeof(VkSemaphore));
    render_info->render_finished_semaphores = (VkSemaphore*)calloc(SY_RENDER_MAX_FRAMES_IN_FLIGHT, sizeof(VkSemaphore));
    render_info->in_flight_fences = (VkFence*)calloc(SY_RENDER_MAX_FRAMES_IN_FLIGHT, sizeof(VkFence));

    VkSemaphoreCreateInfo image_available_semaphore_create_info;
    image_available_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    image_available_semaphore_create_info.pNext = NULL;
    image_available_semaphore_create_info.flags = 0;
    
    VkSemaphoreCreateInfo render_finished_semaphore_create_info;
    render_finished_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    render_finished_semaphore_create_info.pNext = NULL;
    render_finished_semaphore_create_info.flags = 0;
    
    VkFenceCreateInfo fence_create_info;
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.pNext = NULL;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    for (size_t i = 0; i < SY_RENDER_MAX_FRAMES_IN_FLIGHT; ++i)
    {
	VkResult result1 = vkCreateSemaphore(render_info->logical_device, &image_available_semaphore_create_info, NULL, &render_info->image_available_semaphores[i]);
	VkResult result2 = vkCreateSemaphore(render_info->logical_device, &render_finished_semaphore_create_info, NULL, &render_info->render_finished_semaphores[i]);
	VkResult result3 = vkCreateFence(render_info->logical_device, &fence_create_info, NULL, &render_info->in_flight_fences[i]);

	SY_ERROR_COND( result1 != VK_SUCCESS || result2 != VK_SUCCESS || result3 != VK_SUCCESS, "RENDER: Failed to create semaphores or fence.");
    }

}

void sy_render_create_command_buffers(SyRenderInfo *render_info)
{
    render_info->command_buffers_amt = SY_RENDER_MAX_FRAMES_IN_FLIGHT;
    render_info->command_buffers = (VkCommandBuffer*)calloc(render_info->command_buffers_amt, sizeof(VkCommandBuffer));

    VkCommandBufferAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.commandPool = render_info->command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // primary or secondary command buffer
    allocate_info.commandBufferCount = (uint32_t)render_info->command_buffers_amt;

    SY_ERROR_COND(vkAllocateCommandBuffers(render_info->logical_device, &allocate_info, render_info->command_buffers) != VK_SUCCESS, "RENDER: Failed to allocate command buffers.");
}

void sy_render_create_command_pool(SyRenderInfo *render_info)
{
    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = NULL;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = render_info->graphics_queue_family_index;
    
    SY_ERROR_COND(vkCreateCommandPool(render_info->logical_device, &command_pool_create_info, NULL, &render_info->command_pool) != VK_SUCCESS, "RENDER: Failed to create command pool.");
}

void sy_render_create_descriptor_set_layouts(SyRenderInfo *render_info)
{
    { // frame data layout
	VkDescriptorSetLayoutBinding ubo_layout_binding;
	ubo_layout_binding.binding = 0; // the binding of the uniform inside of the shader
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	ubo_layout_binding.pImmutableSamplers = NULL; // for image sampling

	VkDescriptorSetLayoutBinding bindings[] = {ubo_layout_binding};
	uint32_t bindings_amt = SY_ARRLEN(bindings);

	VkDescriptorSetLayoutCreateInfo layout_create_info;
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.pNext = NULL;
	layout_create_info.flags = 0;
	layout_create_info.bindingCount = bindings_amt;
	layout_create_info.pBindings = bindings;
	
	SY_ERROR_COND(vkCreateDescriptorSetLayout(render_info->logical_device, &layout_create_info, NULL, &render_info->frame_descriptor_set_layout) != VK_SUCCESS, "RENDER: Failed to create descriptor set layout - frame data layout.");
    }

    { // material layout
	VkDescriptorSetLayoutBinding ubo_layout_binding;
	ubo_layout_binding.binding = 0; // the binding of the uniform inside of the shader
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	ubo_layout_binding.pImmutableSamplers = NULL; // for image sampling

	VkDescriptorSetLayoutBinding bindings[] = {ubo_layout_binding};
	uint32_t bindings_amt = SY_ARRLEN(bindings);

	VkDescriptorSetLayoutCreateInfo layout_create_info;
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.pNext = NULL;
	layout_create_info.flags = 0;
	layout_create_info.bindingCount = bindings_amt;
	layout_create_info.pBindings = bindings;
	
	SY_ERROR_COND(vkCreateDescriptorSetLayout(render_info->logical_device, &layout_create_info, NULL, &render_info->material_descriptor_set_layout) != VK_SUCCESS, "RENDER: Failed to create descriptor set layout - material layout.");
    }

    { // object layout
	VkDescriptorSetLayoutBinding ubo_layout_binding;
	ubo_layout_binding.binding = 0; // the binding of the uniform inside of the shader
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	ubo_layout_binding.pImmutableSamplers = NULL; // for image sampling

	VkDescriptorSetLayoutBinding bindings[] = {ubo_layout_binding};
	uint32_t bindings_amt = SY_ARRLEN(bindings);

	VkDescriptorSetLayoutCreateInfo layout_create_info;
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.pNext = NULL;
	layout_create_info.flags = 0;
	layout_create_info.bindingCount = bindings_amt;
	layout_create_info.pBindings = bindings;
	
	SY_ERROR_COND(vkCreateDescriptorSetLayout(render_info->logical_device, &layout_create_info, NULL, &render_info->object_descriptor_set_layout) != VK_SUCCESS, "RENDER: Failed to create descriptor set layout - object layout.");
    }

    { // character_map layout
	VkDescriptorSetLayoutBinding ubo_layout_binding;
	ubo_layout_binding.binding = 0; // the binding of the uniform inside of the shader
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	ubo_layout_binding.pImmutableSamplers = NULL; // for image sampling

	VkDescriptorSetLayoutBinding bindings[] = {ubo_layout_binding};
	uint32_t bindings_amt = SY_ARRLEN(bindings);

	VkDescriptorSetLayoutCreateInfo layout_create_info;
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.pNext = NULL;
	layout_create_info.flags = 0;
	layout_create_info.bindingCount = bindings_amt;
	layout_create_info.pBindings = bindings;
	
	SY_ERROR_COND(vkCreateDescriptorSetLayout(render_info->logical_device, &layout_create_info, NULL, &render_info->character_map_descriptor_set_layout) != VK_SUCCESS, "RENDER: Failed to create descriptor set layout.");
    }

    { // character_information layout
	VkDescriptorSetLayoutBinding ubo_layout_binding;
	ubo_layout_binding.binding = 0; // the binding of the uniform inside of the shader
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
	ubo_layout_binding.pImmutableSamplers = NULL; // for image sampling

	VkDescriptorSetLayoutBinding bindings[] = {ubo_layout_binding};
	uint32_t bindings_amt = SY_ARRLEN(bindings);

	VkDescriptorSetLayoutCreateInfo layout_create_info;
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.pNext = NULL;
	layout_create_info.flags = 0;
	layout_create_info.bindingCount = bindings_amt;
	layout_create_info.pBindings = bindings;
	
	SY_ERROR_COND(vkCreateDescriptorSetLayout(render_info->logical_device, &layout_create_info, NULL, &render_info->character_information_descriptor_set_layout) != VK_SUCCESS, "RENDER: Failed to create descriptor set layout.");
    }

    { // text buffer layout
	VkDescriptorSetLayoutBinding ssbo_layout_binding;
	ssbo_layout_binding.binding = 0; // the binding of the uniform inside of the shader
	ssbo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	ssbo_layout_binding.descriptorCount = 1;
	ssbo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	ssbo_layout_binding.pImmutableSamplers = NULL; // for image sampling

	VkDescriptorSetLayoutBinding bindings[] = {ssbo_layout_binding};
	uint32_t bindings_amt = SY_ARRLEN(bindings);

	VkDescriptorSetLayoutCreateInfo layout_create_info;
	layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_create_info.pNext = NULL;
	layout_create_info.flags = 0;
	layout_create_info.bindingCount = bindings_amt;
	layout_create_info.pBindings = bindings;
	
	SY_ERROR_COND(vkCreateDescriptorSetLayout(render_info->logical_device, &layout_create_info, NULL, &render_info->text_buffer_descriptor_set_layout) != VK_SUCCESS, "RENDER: Failed to create descriptor set layout.");
    }

}

VkRenderPass sy_render_create_simple_render_pass(SyRenderInfo *render_info)
{
    VkAttachmentDescription color_attachment;
    color_attachment.format = render_info->swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // what to do when the image first gets loaded and ready to draw
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // what to do when finished writing to image 
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // same as loadOp but we don't care about stencil
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // same as storeOp but we don't care about stencil
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // what layout is image before render pass
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // what layout to output image as after render pass
    color_attachment.flags = 0;
    
    VkAttachmentDescription depth_attachment;
    depth_attachment.format = render_info->depth_format;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // what to do when the image first gets loaded and ready to draw
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // what to do when finished writing to image 
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // same as loadOp but we don't care about stencil
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // same as storeOp but we don't care about stencil
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // what layout is image before render pass
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // what layout to output image as after render pass
    depth_attachment.flags = 0;

    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0; // what index our referenced color attachment is stored at
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // what layout we want it stored at during the subpass

    VkAttachmentReference depth_attachment_reference;
    depth_attachment_reference.attachment = 1; // what index our referenced depth attachment is stored at
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // what layout we want it stored at during the subpass

    VkSubpassDescription subpass;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // specify it is graphics subpass because future may support compute subpass
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference; // layout(location = 0) out ... in frag shader refers to index 0 of this array to write to
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depth_attachment_reference;
    subpass.flags = 0;

    // Create subpass dependency so that the implicit subpass where the image format gets converted waits until we have an image
    VkSubpassDependency subpass_dependency;

    // subpass 0 relies on VK_SUBPASS_EXTERNAL AKA image conversion
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;

    // wait on the output of the image conversion
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = 0;

    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    subpass_dependency.dependencyFlags = 0;

    VkAttachmentDescription attachments[] = {color_attachment, depth_attachment};

    VkRenderPassCreateInfo render_pass_create_info;
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.pNext = NULL;
    render_pass_create_info.flags = 0;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.attachmentCount = SY_ARRLEN(attachments);
    render_pass_create_info.pAttachments = attachments; // NOTE: subpass contains references to attachments, this contains actual attachments
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    VkRenderPass result;

    SY_ERROR_COND(vkCreateRenderPass(render_info->logical_device, &render_pass_create_info, NULL, &result) != VK_SUCCESS, "RENDER: Failed to create render pass.");

    return result;
}

void sy_render_create_framebuffers(SyRenderInfo *render_info)
{
    render_info->swapchain_framebuffers_amt = render_info->swapchain_image_views_amt;
    render_info->swapchain_framebuffers = (VkFramebuffer*)calloc(render_info->swapchain_framebuffers_amt, sizeof(VkFramebuffer));

    // Create a framebuffer for each image view
    for (uint32_t i = 0; i < render_info->swapchain_framebuffers_amt; ++i)
    {
	VkImageView attachments[] = { render_info->swapchain_image_views[i], render_info->depth_image_views[i] };

	VkFramebufferCreateInfo framebuffer_create_info;
	framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_create_info.pNext = NULL;
	framebuffer_create_info.flags = 0;
	framebuffer_create_info.attachmentCount = SY_ARRLEN(attachments);
	framebuffer_create_info.pAttachments = attachments;
	framebuffer_create_info.width = render_info->swapchain_image_extent.width;
	framebuffer_create_info.height = render_info->swapchain_image_extent.height;
	framebuffer_create_info.layers = 1;
	framebuffer_create_info.renderPass = render_info->render_pass;

	SY_ERROR_COND(vkCreateFramebuffer(render_info->logical_device, &framebuffer_create_info, NULL, &render_info->swapchain_framebuffers[i]) != VK_SUCCESS, "RENDER: Failed to create framebuffer.");
    }



}

/* FIXME
size_t sy_render_create_descriptor_set(SyRenderInfo *render_info, size_t uniform_size, void *data, VkDescriptorSetLayout descriptor_layout, uint32_t layout_binding)
{
    // find unused index
    int index = -1;
    for (int i = 0; i < render_info->max_descriptor_sets_amt; ++i)
    {
	if (render_info->descriptor_sets_used[i] == false)
	{
	    render_info->descriptor_sets_used[i] = true;
	    index = i;
	    break;
	}
    }
    SY_ERROR_COND(index == -1, "RENDER: Ran out of descriptor sets.");
	
    // Allocate buffers, and copy data to them
    for (int i = 0; i < SY_RENDER_MAX_FRAMES_IN_FLIGHT; ++i)
    {
	VkBufferCreateInfo buffer_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	buffer_info.size = uniform_size;
	buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	
	VmaAllocationCreateInfo alloc_info = {0};
	alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
	alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
	
	vmaCreateBuffer(render_info->vma_allocator, &buffer_info, &alloc_info, &render_info->descriptor_sets[index].uniform_buffer[i], &render_info->descriptor_sets[index].uniform_buffer_allocation[i], nullptr);

	vmaCopyMemoryToAllocation(render_info->vma_allocator, data, render_info->descriptor_sets[index].uniform_buffer_allocation[i], 0, uniform_size);
    }

    // Allocate descriptor sets
    
    // Initialize layouts array
    uint32_t layout_amt = SY_RENDER_MAX_FRAMES_IN_FLIGHT;
    VkDescriptorSetLayout *layouts = (VkDescriptorSetLayout*)calloc(layout_amt, sizeof(VkDescriptorSetLayout));
    for (int i = 0; i < layout_amt; ++i)
    {
	layouts[i] = descriptor_layout;
    }
    
    VkDescriptorSetAllocateInfo allocate_info;
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.pNext = NULL;
    allocate_info.pSetLayouts = layouts;
    allocate_info.descriptorSetCount = layout_amt;
    allocate_info.descriptorPool = render_info->descriptor_pool;
    
    VkResult result;
    SY_ERROR_COND((result = vkAllocateDescriptorSets(render_info->logical_device, &allocate_info, render_info->descriptor_sets[index].descriptor_set)) != VK_SUCCESS, "RENDER: Failed to allocate descriptor sets. out of pool mem: %d", result == VK_ERROR_OUT_OF_POOL_MEMORY);

    // Populate every descriptor set
    for (size_t i = 0; i < SY_RENDER_MAX_FRAMES_IN_FLIGHT; ++i)
    {
	VkDescriptorBufferInfo buffer_info;
	buffer_info.buffer = render_info->descriptor_sets[index].uniform_buffer[i];
	buffer_info.offset = 0;
	buffer_info.range = uniform_size;
	
	VkWriteDescriptorSet descriptor_write;
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.pNext = NULL;
	descriptor_write.dstSet = render_info->descriptor_sets[index].descriptor_set[i];
	descriptor_write.dstBinding = layout_binding;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pBufferInfo = &buffer_info;
	descriptor_write.pImageInfo = NULL;
	descriptor_write.pTexelBufferView = NULL;
	
	vkUpdateDescriptorSets(render_info->logical_device, 1, &descriptor_write, 0, NULL);
    }

    // Cleanup
    free(layouts);

    return index;
}
*/

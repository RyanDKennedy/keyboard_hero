#include "sy_syengine.hpp"

#include <stdio.h>
#include <vulkan/vulkan_core.h>

#include "render/sy_buffer.hpp"
#include "render/sy_drawing.hpp"
#include "render/sy_render_info.hpp"
#include "render/types/sy_material.hpp"
#include "sy_ecs.hpp"
#include "sy_macros.hpp"

#include "render/sy_render.hpp"
#include "render/sy_resources.hpp"
#include "render/sy_physical_device.hpp"
#include "render/sy_logical_device.hpp"
#include "render/sy_swapchain.hpp"
#include "sy_utils.hpp"

#ifdef NDEBUG
extern "C"
void app_init(SyAppInfo *app_info);

extern "C"
void app_run(SyAppInfo *app_info);

extern "C"
void app_destroy(SyAppInfo *app_info);
#endif

void renderer_init(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    platform_info->render_info.current_frame = 0;
    sy_render_create_physical_device(&platform_info->render_info);
    {
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(platform_info->render_info.physical_device, &props);
	SY_OUTPUT_INFO("using device %s", props.deviceName);
    }
    sy_render_create_logical_device(&platform_info->render_info);
    sy_render_create_swapchain(&platform_info->render_info, platform_info->input_info.window_width, platform_info->input_info.window_height);
    sy_render_create_command_pool(&platform_info->render_info);
    platform_info->render_info.render_pass = sy_render_create_simple_render_pass(&platform_info->render_info);
    sy_render_create_swapchain_framebuffers(&platform_info->render_info);
    sy_render_create_command_buffers(&platform_info->render_info);
    sy_render_create_sync_objects(&platform_info->render_info);
    sy_render_create_descriptor_set_layouts(&platform_info->render_info);

    VmaAllocatorCreateInfo vma_allocator_create_info = {};
    vma_allocator_create_info.flags = 0;
    vma_allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_0;
    vma_allocator_create_info.physicalDevice = platform_info->render_info.physical_device;
    vma_allocator_create_info.device = platform_info->render_info.logical_device;
    vma_allocator_create_info.instance = platform_info->render_info.instance;
    vma_allocator_create_info.pVulkanFunctions = NULL;
    vmaCreateAllocator(&vma_allocator_create_info, &platform_info->render_info.vma_allocator);

    { // Create Pipeline Layouts
	VkDescriptorSetLayout layouts[] = {platform_info->render_info.frame_data_descriptor_set_layout, platform_info->render_info.material_descriptor_set_layout};
	
	VkPipelineLayoutCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.pushConstantRangeCount = 0;
	create_info.pPushConstantRanges = NULL;
	create_info.setLayoutCount = SY_ARRLEN(layouts);
	create_info.pSetLayouts = layouts;
	
	vkCreatePipelineLayout(platform_info->render_info.logical_device, &create_info, NULL, &platform_info->render_info.single_color_pipeline_layout);
    }

    { // Create single color pipeline
	VkVertexInputBindingDescription binding_description;
	binding_description.binding = 0;
	binding_description.stride = sizeof(float) * 2;
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;    

	VkVertexInputAttributeDescription attr[1];
	attr[0].binding = 0;
	attr[0].location = 0;
	attr[0].format = VK_FORMAT_R32G32_SFLOAT;
	attr[0].offset = 0;

	SyPipelineCreateInfo create_info;
	create_info.vertex_shader_path = "single_color/vertex.spv";
	create_info.fragment_shader_path = "single_color/fragment.spv";
	create_info.vertex_input_binding_description = binding_description;
	create_info.vertex_input_attribute_descriptions = attr;
	create_info.vertex_input_attribute_descriptions_amt = 1;
	create_info.render_type = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	create_info.subpass_number = 0;
	create_info.pipeline_layout = platform_info->render_info.single_color_pipeline_layout;

	platform_info->render_info.single_color_pipeline = sy_render_create_pipeline(&platform_info->render_info, &create_info);
    }

    // init render types in ecs
    sy_render_init_ecs(&app_info->ecs);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyMesh);
    SY_ECS_REGISTER_TYPE(app_info->ecs, SyMaterialComponent);

    { // Create descriptor pool
	VkDescriptorPoolSize pool_size;
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = platform_info->render_info.max_descriptor_sets_amt;
	
	VkDescriptorPoolCreateInfo pool_create_info;
	pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_create_info.pNext = NULL;
	pool_create_info.flags = 0;
	pool_create_info.poolSizeCount = 1;
	pool_create_info.pPoolSizes = &pool_size;
	pool_create_info.maxSets = platform_info->render_info.max_descriptor_sets_amt;
	
	SY_ERROR_COND(vkCreateDescriptorPool(platform_info->render_info.logical_device, &pool_create_info, NULL, &platform_info->render_info.descriptor_pool) != VK_SUCCESS, "RENDER: Failed to create descriptor pool.");

	platform_info->render_info.descriptor_sets = (SyDescriptorSetDataGroup*)calloc(platform_info->render_info.max_descriptor_sets_amt, sizeof(SyDescriptorSetDataGroup));
	platform_info->render_info.descriptor_sets_used = (bool*)calloc(platform_info->render_info.max_descriptor_sets_amt, sizeof(bool));

    }

    { // Create the frame data descriptor sets / uniform buffers

	FrameData data;
	data.test = 1.0f;
	data.test1 = 0.0f;
	data.test2 = 0.0f;

	platform_info->render_info.frame_data_descriptor_index = sy_render_create_descriptor_set(&platform_info->render_info, sizeof(FrameData), &data, platform_info->render_info.frame_data_descriptor_set_layout, 0);

    }

    {

	int material_type_id = app_info->ecs.get_type_id<SyMaterialComponent>();

	size_t component_index = app_info->ecs.get_unused_component<SyMaterialComponent>();
	SyMaterialComponent *material_comp = &app_info->ecs.m_component_data_arr[material_type_id].get<SyMaterialComponent>(component_index);

	material_comp->descriptor_set_index = -1;
	material_comp->material.ambient[0] = 0.0f;
	material_comp->material.ambient[1] = 0.0f;
	material_comp->material.ambient[2] = 0.0f;
	material_comp->material.diffuse = {1.0f, 0.0f, 1.0f};
	material_comp->material.specular[0] = 0.0f;
	material_comp->material.specular[1] = 0.0f;
	material_comp->material.specular[2] = 0.0f;

	float vertex_data[] =
	    {
		-0.5f, -0.5f,
		0.5f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f
	    };
	
	uint32_t index_data[] =
	    {
		0, 3, 2, 2, 1, 0
	    };

	SyEntityHandle square = app_info->ecs.new_entity();
	app_info->ecs.entity_assign_component<SyMaterialComponent>(square, component_index);
	app_info->ecs.entity_add_component<SyMesh>(square);
	SyMesh *mesh = app_info->ecs.component<SyMesh>(square);
	
	sy_render_create_vertex_buffer(&platform_info->render_info, SY_ARRLEN(vertex_data), sizeof(float) * 2, vertex_data, &mesh->vertex_buffer, &mesh->vertex_buffer_alloc);    

	mesh->index_amt = SY_ARRLEN(index_data);
	sy_render_create_index_buffer(&platform_info->render_info, SY_ARRLEN(index_data), index_data, &mesh->index_buffer, &mesh->index_buffer_alloc);
    }

    SY_OUTPUT_INFO("finished render init");

}

void renderer_cleanup(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    // Cleanup meshes
    size_t mesh_type_id = app_info->ecs.get_type_id<SyMesh>();
    for (size_t i = 0; i < app_info->ecs.m_entity_used.m_filled_length; ++i)
    {
	if (app_info->ecs.m_entity_used.get<bool>(i) == true)
	{
	    if (app_info->ecs.m_entity_data.get<SyEntityData>(i).mask[mesh_type_id] == true)
	    {
		SyMesh *mesh = app_info->ecs.component<SyMesh>(i);

		vmaDestroyBuffer(platform_info->render_info.vma_allocator, mesh->vertex_buffer, mesh->vertex_buffer_alloc);
		vmaDestroyBuffer(platform_info->render_info.vma_allocator, mesh->index_buffer, mesh->index_buffer_alloc);
	    }
	}
    }

    // Cleanup descriptor sets
    for (int i = 0; i < platform_info->render_info.max_descriptor_sets_amt; ++i)
    {
	if (platform_info->render_info.descriptor_sets_used[i] == true)
	{
	    for (int j = 0; j < platform_info->render_info.max_frames_in_flight; ++j)
	    {
		vmaDestroyBuffer(platform_info->render_info.vma_allocator, platform_info->render_info.descriptor_sets[i].uniform_buffer[j], platform_info->render_info.descriptor_sets[i].uniform_buffer_allocation[j]);
	    }
	}
    }
    free(platform_info->render_info.descriptor_sets);
    free(platform_info->render_info.descriptor_sets_used);
    vkDestroyDescriptorPool(platform_info->render_info.logical_device, platform_info->render_info.descriptor_pool, NULL);

    vkDestroyPipeline(platform_info->render_info.logical_device, platform_info->render_info.single_color_pipeline, NULL);

    vkDestroyPipelineLayout(platform_info->render_info.logical_device, platform_info->render_info.single_color_pipeline_layout, NULL);

    vmaDestroyAllocator(platform_info->render_info.vma_allocator);

    vkDestroyDescriptorSetLayout(platform_info->render_info.logical_device, platform_info->render_info.frame_data_descriptor_set_layout, NULL);
    vkDestroyDescriptorSetLayout(platform_info->render_info.logical_device, platform_info->render_info.material_descriptor_set_layout, NULL);

    free(platform_info->render_info.command_buffers);

    for (int i = 0; i < platform_info->render_info.max_frames_in_flight; ++i)
    {
	vkDestroySemaphore(platform_info->render_info.logical_device, platform_info->render_info.image_available_semaphores[i], NULL);
	vkDestroySemaphore(platform_info->render_info.logical_device, platform_info->render_info.render_finished_semaphores[i], NULL);
	vkDestroyFence(platform_info->render_info.logical_device, platform_info->render_info.in_flight_fences[i], NULL);
    }
    free(platform_info->render_info.image_available_semaphores);
    free(platform_info->render_info.render_finished_semaphores);
    free(platform_info->render_info.in_flight_fences);
    
    for (int i = 0; i < platform_info->render_info.swapchain_framebuffers_amt; ++i)
    {
	vkDestroyFramebuffer(platform_info->render_info.logical_device, platform_info->render_info.swapchain_framebuffers[i], NULL);
    }
    free(platform_info->render_info.swapchain_framebuffers);

    vkDestroyRenderPass(platform_info->render_info.logical_device, platform_info->render_info.render_pass, NULL);

    vkDestroyCommandPool(platform_info->render_info.logical_device, platform_info->render_info.command_pool, NULL); // command buffers are freed when command pool is freed

    sy_render_destroy_swapchain(&platform_info->render_info);
    vkDestroyDevice(platform_info->render_info.logical_device, NULL);
}


void engine_init(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    SY_OUTPUT_INFO("Starting Engine");

    app_info->stop_game = false;

    // ECS Init
    app_info->ecs.initialize();

    // Arena/Allocation Init
    SY_ERROR_COND(app_info->persistent_arena.initialize(4096) != 0, "Failed to allocate data for persistent arena.");
    SY_ERROR_COND(app_info->frame_arena.initialize(4096) != 0, "Failed to allocate data for frame arena.");
    app_info->global_mem_size = 2048;
    app_info->global_mem = app_info->persistent_arena.alloc(app_info->global_mem_size);

    app_info->delta_time = 0.0;

    renderer_init(platform_info, app_info);

#ifndef NDEBUG
    platform_info->app_init(app_info);
#else
    app_init(app_info);
#endif


    // stop the game signal
    if (app_info->stop_game == true)
    {
	platform_info->end_engine = true;
    }
}

void engine_run(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    app_info->frame_arena.free_all();
    app_info->input_info = platform_info->input_info;
    app_info->delta_time = (double)platform_info->delta_time / 1000000.0;

    SyInputInfo &input = app_info->input_info;

    // close the engine
    if (input.window_should_close)
    {
	platform_info->end_engine = true;
    }

#ifndef NDEBUG
    // THIS IS DLL HOT RELOAD STUFF, SO DON'T INCLUDE IN RELEASE VERSION
    
    // reload dlls for app
    if (input.f1)
    {
	platform_info->reload_dll = true;
    }


    // Run the app_dll_init function after dll has been reloaded 
    if (platform_info->dll_first_run == true)
    {
	platform_info->app_dll_init(app_info);
	platform_info->dll_first_run = false;
    }

    // run the app
    platform_info->app_run(app_info);

    // if the dll is about to be reloaded then run the app_dll_exit function
    if (platform_info->reload_dll == true)
    {
	platform_info->app_dll_exit(app_info);
    }
#else

    app_run(app_info);

#endif

    // NOTE: The input below this is stuff responding to data received by running the app

    // stop the game signal
    if (app_info->stop_game == true)
    {
	platform_info->end_engine = true;
    }

    sy_render_draw(&platform_info->render_info, &platform_info->input_info, &app_info->ecs);

}

void engine_destroy(SyPlatformInfo *platform_info, SyAppInfo *app_info)
{
    vkDeviceWaitIdle(platform_info->render_info.logical_device);

    SY_OUTPUT_INFO("Ending Engine");

#ifndef NDEBUG
    platform_info->app_destroy(app_info);
#else
    app_destroy(app_info);
#endif

    renderer_cleanup(platform_info, app_info);


    { // Cleanup App Info
	// Arena/Allocation Cleanup
	app_info->persistent_arena.destroy();
	app_info->frame_arena.destroy();
    }

    app_info->ecs.destroy();
}

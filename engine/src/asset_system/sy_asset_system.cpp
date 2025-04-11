#include "sy_asset_system.hpp"

#include "render/sy_buffer.hpp"
#include "render/sy_resources.hpp"
#include "render/types/sy_mesh.hpp"
#include "render/types/sy_draw_info.hpp"
#include "render/types/sy_asset_metadata.hpp"

size_t sy_load_mesh_from_obj(SyRenderInfo *render_info, SyEcs *ecs, const char *obj_path)
{
    size_t mesh_component_index;
    {
	// Create component
	mesh_component_index = ecs->get_unused_component<SyMesh>();
	SyMesh *mesh_comp = ecs->component_from_index<SyMesh>(mesh_component_index);
	
	// Create and fill buffers
	uint32_t *index_data = NULL;
	float *vertex_data = NULL;
	size_t index_data_size = 0;
	size_t vertex_data_size = 0;
	sy_parse_obj(obj_path, &vertex_data, &vertex_data_size, &index_data, &index_data_size);
	mesh_comp->index_amt = index_data_size;
	sy_render_create_vertex_buffer(render_info, vertex_data_size, sizeof(float) * 3, vertex_data, &mesh_comp->vertex_buffer, &mesh_comp->vertex_buffer_alloc);
	sy_render_create_index_buffer(render_info, index_data_size, index_data, &mesh_comp->index_buffer, &mesh_comp->index_buffer_alloc);
	free(vertex_data);
	free(index_data);
    }

    size_t asset_metadata_index;
    {
	asset_metadata_index = ecs->get_unused_component<SyAssetMetadata>();
	SyAssetMetadata *asset_metadata = ecs->component_from_index<SyAssetMetadata>(asset_metadata_index);

	strncpy(asset_metadata->name, obj_path, SY_ASSET_METADATA_NAME_BUFFER_SIZE);
	asset_metadata->asset_type = SyAssetType::mesh;
	asset_metadata->asset_component_index = mesh_component_index;
	asset_metadata->children_amt = 0;
    }

    return asset_metadata_index;
}

void sy_destroy_mesh_from_index(SyRenderInfo *render_info, SyEcs *ecs, size_t mesh_index)
{
    vkDeviceWaitIdle(render_info->logical_device);
    SyMesh *mesh = ecs->component_from_index<SyMesh>(mesh_index);
    vmaDestroyBuffer(render_info->vma_allocator, mesh->vertex_buffer, mesh->vertex_buffer_alloc);
    vmaDestroyBuffer(render_info->vma_allocator, mesh->index_buffer, mesh->index_buffer_alloc);
}

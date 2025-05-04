#include "sy_asset_system.hpp"

#include "render/sy_buffer.hpp"
#include "render/sy_resources.hpp"
#include "render/sy_fonts.hpp"
#include "render/types/sy_mesh.hpp"
#include "render/types/sy_draw_info.hpp"
#include "render/types/sy_asset_metadata.hpp"

size_t sy_load_asset_from_file(SyRenderInfo *render_info, SyEcs *ecs, const char *asset_path, SyAssetType type)
{
    switch (type)
    {
	case SyAssetType::mesh:
	{
	    return sy_load_mesh_from_obj(render_info, ecs, asset_path);
	}

	case SyAssetType::font:
	{
	    return sy_load_font_from_file(render_info, ecs, asset_path);
	}

	default:
	    SY_ERROR("UNSUPPORTED ASSET TYPE PASSED");
    }


}

size_t sy_load_font_from_file(SyRenderInfo *render_info, SyEcs *ecs, const char *font_path)
{
    size_t font_component_index;
    {
	font_component_index = ecs->get_unused_component<SyFont>();
	SyFont *font = ecs->component_from_index<SyFont>(font_component_index);
	*font = sy_render_create_font(render_info, ecs, font_path, 512, 512, 64, " \tabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,./<>?;:'\"[]{}\\|-_=+`~1234567890!@#$%^&*()", 2);
    }

    size_t asset_metadata_index;
    {
	asset_metadata_index = ecs->get_unused_component<SyAssetMetadata>();
	SyAssetMetadata *metadata = ecs->component_from_index<SyAssetMetadata>(asset_metadata_index);

	metadata->asset_component_index = font_component_index;
	metadata->asset_type = SyAssetType::font;
	metadata->children_amt = 0;
	strncpy(metadata->name, font_path, SY_ASSET_METADATA_NAME_BUFFER_SIZE);
    }
    
    return asset_metadata_index;
}

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
	SY_ERROR_COND(sy_parse_obj(obj_path, &vertex_data, &vertex_data_size, &index_data, &index_data_size) != 0);

	sy_render_create_vertex_buffer(render_info, vertex_data_size * sizeof(vertex_data[0]), (uint8_t*)vertex_data, &mesh_comp->vertex_buffer, &mesh_comp->vertex_buffer_alloc);

	mesh_comp->index_amt = index_data_size;
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

size_t sy_load_audio_from_file(SySoundInfo *sound_info, SyEcs *ecs, const char *audio_path)
{
    size_t audio_index;
    {
	audio_index = ecs->get_unused_component<SyAudio>();
	SyAudio *audio = ecs->component_from_index<SyAudio>(audio_index);
	alGenBuffers(1, &audio->buffer);
	
	ALsizei size;
	ALsizei freq;
	ALenum format;
	ALvoid *data;
	ALboolean loop = AL_FALSE;
	alutLoadWAVFile((ALbyte*)audio_path, &format, &data, &size, &freq, &loop);
	alBufferData(audio->buffer, format, data, size, freq);
	alutUnloadWAV(format, data, size, freq);
    }

    size_t asset_metadata_index;
    {
	asset_metadata_index = ecs->get_unused_component<SyAssetMetadata>();
	SyAssetMetadata *asset_metadata = ecs->component_from_index<SyAssetMetadata>(asset_metadata_index);

	strncpy(asset_metadata->name, audio_path, SY_ASSET_METADATA_NAME_BUFFER_SIZE);
	asset_metadata->asset_type = SyAssetType::audio;
	asset_metadata->asset_component_index = audio_index;
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
    ecs->release_component<SyMesh>(mesh_index);
}

void sy_destroy_font_from_index(SyRenderInfo *render_info, SyEcs *ecs, size_t font_index)
{
    vkDeviceWaitIdle(render_info->logical_device);
    SyFont *font = ecs->component_from_index<SyFont>(font_index);
    sy_render_destroy_font(render_info, ecs, font);
    ecs->release_component<SyFont>(font_index);
}

void sy_destroy_audio_from_index(SySoundInfo *sound_info, SyEcs *ecs, size_t audio_index)
{
    SyAudio *audio = ecs->component_from_index<SyAudio>(audio_index);
    alDeleteBuffers(1, &audio->buffer);
    ecs->release_component<SyAudio>(audio_index);
}

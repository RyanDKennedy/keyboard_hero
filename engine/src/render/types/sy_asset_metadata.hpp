#pragma once

#include <stdlib.h>
#include <stdint.h>

enum class SyAssetType
{
    mesh,
    model,
    texture,
    font,
    audio,
};

#define SY_ASSET_METADATA_MAX_CHILDREN 5
#define SY_ASSET_METADATA_NAME_BUFFER_SIZE 16

struct SyAssetMetadata
{
    size_t children[SY_ASSET_METADATA_MAX_CHILDREN]; // array of indices into SyAssetMetadata component that are children of this asset
    char name[SY_ASSET_METADATA_NAME_BUFFER_SIZE];
    size_t asset_component_index; // index into component array based on asset_type. ex: if asset_type is mesh then this is index into SyMesh component
    SyAssetType asset_type; // type of asset that this refers to
    uint8_t children_amt; // the amount of used indices starting at index 0 in children array
};

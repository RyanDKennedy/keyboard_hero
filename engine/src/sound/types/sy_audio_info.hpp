#pragma once

#include "glm_include.hpp"

// other properties like position, velocity, and rotation should be put in a SyTransform component
struct SyAudioInfo
{
    // action flags
    bool should_stop;
    bool should_play;

    // properties
    bool loop;
    float gain;
    float pitch;

    bool needs_audio_state_generated;

    size_t audio_asset_metadata_id;
    size_t audio_state_id;
};

#pragma once

#include "glm_include.hpp"

// other properties like position, velocity, and rotation should be put in a SyTransform component
struct SyAudioProperties
{
    // action flags
    bool should_stop;
    bool should_play;

    // properties
    bool loop;
    float seconds_in;
    float gain;
};

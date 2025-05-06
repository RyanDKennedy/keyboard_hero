#pragma once

#include <stdlib.h>
#include <AL/al.h>

struct SyAudio
{
    ALuint buffer;
};

struct SyAudioSource
{
    ALuint source;
    size_t audio_component_index;
};

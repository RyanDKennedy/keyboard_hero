#include "sy_sound.hpp"
#include "sound/types/sy_audio.hpp"
#include "sy_macros.hpp"

#include <AL/al.h>
#include <stdlib.h>

void sy_sound_info_init(SySoundInfo *sound_info)
{
    sound_info->device = alcOpenDevice(NULL);
    SY_ERROR_COND(sound_info->device == NULL, "Failed to open device");

    sound_info->context = alcCreateContext(sound_info->device, NULL);
    SY_ERROR_COND(alcMakeContextCurrent(sound_info->context) != ALC_TRUE, "Failed to make context current");

}

void sy_sound_info_deinit(SySoundInfo *sound_info)
{
    alcMakeContextCurrent(NULL);
    alcDestroyContext(sound_info->context);
    alcCloseDevice(sound_info->device);
}

size_t sy_sound_create_audio_state(SySoundInfo *sound_info, SyEcs *ecs)
{
    size_t component = ecs->get_unused_component<SyAudioState>();

    SyAudioState *state = ecs->component_from_index<SyAudioState>(component);
    alGenSources(1, &state->source);

    return component;
}

void sy_sound_destroy_audio_state(SySoundInfo *sound_info, SyEcs *ecs, size_t audio_state_index)
{
    alDeleteSources(1, &ecs->component_from_index<SyAudioState>(audio_state_index)->source);
}


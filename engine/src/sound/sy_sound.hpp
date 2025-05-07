#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include <AL/alut.h>

#include <stdlib.h>

#include "sy_ecs.hpp"

struct SySoundInfo
{
    ALCdevice *device;
    ALCcontext *context;

};

void sy_sound_info_init(SySoundInfo *sound_info);
void sy_sound_info_deinit(SySoundInfo *sound_info);

size_t sy_sound_create_audio_state(SySoundInfo *sound_info, SyEcs *ecs);
void sy_sound_destroy_audio_state(SySoundInfo *sound_info, SyEcs *ecs, size_t audio_state_index);

#pragma once

#include <AL/al.h>
#include <AL/alc.h>

#include <AL/alut.h>

struct SySoundInfo
{
    ALCdevice *device;
    ALCcontext *context;

};

void sy_sound_info_init(SySoundInfo *sound_info);
void sy_sound_info_deinit(SySoundInfo *sound_info);

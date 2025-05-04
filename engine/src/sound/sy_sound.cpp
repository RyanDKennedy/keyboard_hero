#include "sy_sound.hpp"
#include "sy_macros.hpp"

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

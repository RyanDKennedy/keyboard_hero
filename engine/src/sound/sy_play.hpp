#pragma once

#include "sy_sound.hpp"
#include "types/sy_audio.hpp"
#include "types/sy_audio_info.hpp"

#include "sy_ecs.hpp"

void sy_sound_play(SySoundInfo *sound_info, SyEcs *ecs, SyEntityHandle player);

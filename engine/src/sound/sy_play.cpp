#include "sy_play.hpp"
#include "render/types/sy_asset_metadata.hpp"

#include "sound/sy_sound.hpp"
#include "sy_macros.hpp"

void sy_sound_play(SySoundInfo *sound_info, SyEcs *ecs, SyEntityHandle player)
{
    // TODO: set listener attributes

    alListener3f(AL_POSITION, 0, 0, 1.0f);
    alListener3f(AL_VELOCITY, 0, 0, 0);

    for (size_t i = 0; i < ecs->m_entity_used.size(); ++i)
    {

	if (ecs->entity_has_component<SyAudioInfo>(i) == true)
	{
	    SyAudioInfo *audio_info = ecs->component<SyAudioInfo>(i);
	    if (audio_info->needs_audio_state_generated)
	    {
		audio_info->audio_state_id = sy_sound_create_audio_state(sound_info, ecs);
		audio_info->needs_audio_state_generated = false;
	    }

	    if (ecs->component<SyAudioInfo>(i)->should_play == true)
	    {
		audio_info->should_play = false;
		
		SyAudioState *audio_state = ecs->component_from_index<SyAudioState>(audio_info->audio_state_id);
		SyAssetMetadata *buf_metadata = ecs->component_from_index<SyAssetMetadata>(audio_info->audio_asset_metadata_id);
		
		SY_ASSERT(buf_metadata->asset_type == SyAssetType::audio);
		
		SyAudio *audio = ecs->component_from_index<SyAudio>(buf_metadata->asset_component_index);
		
		alSourcef(audio_state->source, AL_PITCH, audio_info->pitch);
		alSourcef(audio_state->source, AL_GAIN, audio_info->gain);
		alSource3f(audio_state->source, AL_POSITION, 0, 0, 0);
		alSource3f(audio_state->source, AL_VELOCITY, 0, 0, 0);
		alSourcei(audio_state->source, AL_LOOPING, (audio_info->loop)? AL_TRUE : AL_FALSE);
		alSourcei(audio_state->source, AL_BUFFER, audio->buffer);
		alSourcePlay(audio_state->source);
	    }

	    if (ecs->component<SyAudioInfo>(i)->should_stop == true)
	    {
		audio_info->should_stop = false;
		
		SyAudioState *audio_state = ecs->component_from_index<SyAudioState>(audio_info->audio_state_id);

		alSourceStop(audio_state->source);
	    }

	}
	
	

    }


}

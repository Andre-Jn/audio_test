#include <stdio.h>
#include "vcsystem_audio.h"

int main()
{
	/* audio */
	status_t status = OSA_SOK;
	vcsystem_audio_object_t audio_obj;
	vcsystem_audio_handle audio_hdl = &audio_obj;
	vcsystem_audio_params_t audio_prm;

	audio_prm.m_capture_params.m_channel_nums	= 2;
	audio_prm.m_capture_params.m_sample_rate 	= 44100;
	audio_prm.m_capture_params.m_audio_volume 	= 2;
	audio_prm.m_playout_params.m_channel_nums	= 2;
	audio_prm.m_playout_params.m_sample_rate 	= 44100;
	audio_prm.m_playout_params.m_audio_volume 	= 2;

	status = vcsystem_audio_params_init(audio_hdl,&audio_prm);
	status = vcsystem_audio_init(audio_hdl);	
	status = vcsystem_audio_start(audio_hdl,1);	
	/* audio */
	while(1);

	/* audio */
	status = vcsystem_audio_stop(audio_hdl,1);	
	status = vcsystem_audio_deinit(audio_hdl);	
	/* audio */

	return 0;
}

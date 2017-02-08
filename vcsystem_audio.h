/** ============================================================================
 *
 *  vcsystem_audio.h
 *
 *  Author     : xkf
 *
 *  Date       : Feb 21, 2013
 *
 *  Description: 
 *  ============================================================================
 */

#if !defined (__LSM_VCSYSTEM_AUDIO_H)
#define __LSM_VCSYSTEM_AUDIO_H

/*  --------------------- Include system headers ---------------------------- */

/*  --------------------- Include user headers   ---------------------------- */

#include "osa_que.h"
#include "osa_thr.h"
#include "vcsystem_audio_pri.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 *  --------------------- Macro definition -------------------------------------
 */

/** ============================================================================
 *  @Macro:         Macro name
 *
 *  @Description:   Description of this macro.
 *  ============================================================================
 */

typedef int status_t;

/*
 *  --------------------- Data type definition ---------------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Structure name
 *
 *  @Description:   Description of the structure.
 *
 *  @Field:         Field1 member
 *
 *  @Field:         Field2 member
 *  ----------------------------------------------------------------------------
 */
typedef enum
{
	IAUDIO_1_0 = 0,         /**< Mono. */
	IAUDIO_2_0 = 1,         /**< Stereo. */
	IAUDIO_11_0 = 2,        /**< Dual Mono.
							 *
							 *   @sa    IAUDIO_DualMonoMode
							 */
	IAUDIO_3_0 = 3,         /**< Left, Right, Center. */
	IAUDIO_2_1 = 4,         /**< Left, Right, Sur. */
	IAUDIO_3_1 = 5,         /**< Left, Right, Center, Sur. */
	IAUDIO_2_2 = 6,         /**< Left, Right, SurL, SurR. */
	IAUDIO_3_2 = 7,         /**< Left, Right, Center, SurL, SurR. */
	IAUDIO_2_3 = 8,         /**< Left, Right, SurL, SurR, surC. */
	IAUDIO_3_3 = 9,         /**< Left, Right, Center, SurL, SurR, surC. */
	IAUDIO_3_4 =10          /**< Left, Right, Center, SurL, SurR, sideL, sideR.
	*/
} IAUDIO_ChannelMode;

typedef enum {
    /**
     *  Left channel data followed by right channel data.
     *  Note, for single channel (mono), right channel data will be the same
     *  as the left channel data.
     */
    IAUDIO_BLOCK = 0,

    /**
     *  Left and right channel data interleaved.
     *  Note, for single channel (mono), right channel data will be the same
     *  as the left channel data.
     */
    IAUDIO_INTERLEAVED = 1
} IAUDIO_PcmFormat;

typedef struct vcsystem_audcapdis_params_tag {
    unsigned int                m_channel_nums;
    unsigned int                m_sample_rate;
    unsigned int                m_audio_volume;
} vcsystem_audcapdis_params_t;


typedef struct vcsystem_audio_params_tag  {
    vcsystem_audcapdis_params_t m_capture_params;
    vcsystem_audcapdis_params_t m_playout_params;
} vcsystem_audio_params_t;

typedef struct vcsystem_audio_callbacks_tag {
} vcsystem_audio_callbacks_t;

typedef struct vcsystem_audio_capenc_tag {
    unsigned int                m_shared_region;
    unsigned int                m_in_bufsize;
    unsigned char *             m_in_buf;
    unsigned int                m_out_bufsize;
    unsigned char *             m_out_buf;

} vcsystem_audio_encoder_t;

typedef struct vcsystem_audio_decdis_tag {
    unsigned int                m_shared_region;
    unsigned int                m_in_bufsize;
    unsigned char *             m_in_buf;
    unsigned int                m_out_bufsize;
    unsigned char *             m_out_buf;

} vcsystem_audio_decoder_t;


#define AUDFRM_MAX_FRAME_PTR                             (8)

typedef struct AudFrm_Buf {
    Int8 *audBuf;
    Int32 len;
    UInt32  muteFlag;
    UInt64 timestamp;
} AudFrm_Buf;

typedef struct AudFrm_BufList {
    UInt32 numFrames;
    AudFrm_Buf * frames[AUDFRM_MAX_FRAME_PTR];
} AudFrm_BufList;


typedef struct vcsystem_audio_object_tag  {
    volatile unsigned int       m_aud_working;

    vcsystem_audio_params_t     m_aud_params;

    snd_pcm_t *                 m_audcap_hdl;
    snd_pcm_t *                 m_audpla_hdl;

    OSA_ThrHndl                 m_cap_thd;
    OSA_ThrHndl                 m_pla_thd;

    unsigned char             * m_audio_buffer;
    unsigned char             * m_swap_buf;
    unsigned int                m_swap_bufsize;
    unsigned char             * m_play_buf;
    unsigned int                m_play_bufsize;

    AudFrm_Buf                  m_capture_frames[VCSYSTEM_AUDIO_MAX_QUEUE_LEN];
    OSA_QueHndl                 m_capture_free_que;
    OSA_QueHndl                 m_capture_busy_que;

	//unsigned char             * m_decoded_buffer;
	AudFrm_Buf                  m_playout_frames[VCSYSTEM_AUDIO_MAX_QUEUE_LEN];
	OSA_QueHndl                 m_playout_free_que;
    OSA_QueHndl                 m_playout_busy_que;
    
    unsigned int                m_frame_id;
    unsigned int                m_frame_count;
} vcsystem_audio_object_t, *vcsystem_audio_handle;

/*
 *  --------------------- Public function declaration --------------------------
 */
status_t
vcsystem_audio_params_init(vcsystem_audio_handle hdl,
        const vcsystem_audio_params_t *p);

status_t
vcsystem_audio_init(vcsystem_audio_handle hdl);

status_t
vcsystem_audio_reinit(vcsystem_audio_handle hdl, const vcsystem_audio_params_t *p);

status_t
vcsystem_audcap_start(vcsystem_audio_handle hdl);

status_t
vcsystem_audcap_stop(vcsystem_audio_handle hdl);

status_t
vcsystem_audcap_control(vcsystem_audio_handle hdl, int cmd, void *arg);

status_t
vcsystem_auddis_start(vcsystem_audio_handle hdl);

status_t
vcsystem_auddis_stop(vcsystem_audio_handle hdl);

status_t
vcsystem_auddis_control(vcsystem_audio_handle hdl, int cmd, void *arg);

status_t
vcsystem_audio_deinit(vcsystem_audio_handle hdl);


////////////////////////////////////////////////////////////////////////////////

#if 0
                        Audio_systemInit()
                                |
                                |
             ------------------------------------------------------------------------------------
             |                                                                                  |
 Start       |                                                                                  |
------> Audio Capture                                                                     Audio Encode
             |                                                                                  |
             |                                                                                  |
        Set Cap Params                                                                    Set Enc Params
 (chl_num, sample_rate, volume)                                           (enc_type, chl_num, sample_rate, bit_rate)
             |                                                                                  |
             |                                                                                  |
    Create Capture Thread ---------->                                      <---------- Create Encode Thread
             |                      |                                      |                    |
             |                      |                                      |                    |
<-------  Return        Init Audio Capture Device               Create Audio Encode Alg      Return
                                    |                                      |
                                    |                                      |
                       ----> Read Audio Frame               ---->  Read Audio Frame
                       |            |                       |              |
                       |            |                       |              |
                       --------------                       |     Encode Auido Frame
                                    |                       |              |
                                    |                       |              |
                       DeInit Auido Capture Device          ----------------
                                    |                                      |
                                    |                                      |
                                   End                          Delete Audio Encode Alg
 Stop
------> Audio Capture
             |
             |
   Delete Capture Thread
             |
             |
            End



            -------------------------------------------------------------------------------------
                                |
                                |
                      Audio_systemDeInit()

#endif

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__LSM_VCSYSTEM_AUDIO_H) */

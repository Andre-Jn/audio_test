/** ============================================================================
 *
 *  vcsystem_audio_pri.h
 *
 *  Author     : xkf
 *
 *  Date       : Feb 25, 2013
 *
 *  Description: 
 *  ============================================================================
 */

#if !defined (__LSM_VCSYSTEM_AUDIO_PRI_H)
#define __LSM_VCSYSTEM_AUDIO_PRI_H

/*  --------------------- Include system headers ---------------------------- */
#include <alsa/asoundlib.h>

/*  --------------------- Include user headers   ---------------------------- */
//#include <mcfw/interfaces/link_api/avsync_hlos.h>

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
#define VCSYSTEM_ALSA_CAPTURE_DEVICE        ("plughw:0,0")
#define VCSYSTEM_ALSA_PLAYOUT_DEVICE        ("plughw:0,0")

#define VCSYSTEM_AUDIO_SAMPLE_LEN           (2)

//#define VCSYSTEM_AUDIO_MAX_IN_SAMPLES       (128)
#define VCSYSTEM_AUDIO_MAX_IN_SAMPLES       (256)

#define VCSYSTEM_AUDIO_BUFFER_SIZE          ((8192 * 2) >> 2)
#define VCSYSTEM_AUDIO_MAX_BUFFER_SIZE      (10 * 1024)   
#define VCSYSTEM_AUDIO_MAX_QUEUE_LEN        (2)

#define VCSYSTEM_AUDIO_TASK_PRI             (17)
#define VCSYSTEM_AUDIO_TASK_STACK_SIZE      (10 * 1024)

#define VCSYSTEM_AUDIO_MAX_INBUFFER_SIZE    (4 * 1024)
#define VCSYSTEM_AUDIO_MAX_OUTBUFFER_SIZE   (4 * 1024)

#define VCSYSTEM_AUDIO_MAX_CHANNELS         (4)

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

/*
 *  --------------------- Public function declaration --------------------------
 */

#if defined(__cplusplus)
}
#endif  /* defined(__cplusplus) */

#endif  /* if !defined (__LSM_VCSYSTEM_AUDIO_PRI_H) */

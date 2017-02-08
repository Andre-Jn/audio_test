/** ============================================================================
 *
 *  vcsystem_audio.c
 *
 *  Author     : xkf
 *
 *  Date       : Feb 21, 2013
 *
 *  Description: 
 *  ============================================================================
 */

/*  --------------------- Include system headers ---------------------------- */
#include <stdio.h>
#include<math.h>

/*  --------------------- Include user headers   ---------------------------- */

#include "vcsystem_audio.h"
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
#define VCSYSTEM_AUDIO_SWAP_BUFFER_SIZE (1 * 1024 * 1024)

#define VCSYSTEM_AUDIO_MIN_CAPUTRE_SIZE (4096)
//#define VCSYSTEM_AUDIO_MIN_CAPUTRE_SIZE (2048)

#define VCSYSTEM_AUDIO_GET_HANDLE(hdl)  ((vcsystem_audio_handle)(hdl))


#define DEBUG_PRINT_ERROR_AND_RETURN(str, err, hdl)             \
do {                                                            \
    if ((hdl) != NULL) {                                        \
        snd_pcm_close((hdl));                                   \
        (hdl) = NULL;                                           \
        return OSA_EFAIL;                                   \
    }                                                           \
} while (0); 


/*
 *  --------------------- Structure definition ---------------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Structure name
 *
 *  @Description:   Description of the structure.
 *
 *  @Field:         Field1 member
 *
 *  @Field          Field2 member
 *  ----------------------------------------------------------------------------
 */

/*
 *  --------------------- Global variable definition ---------------------------
 */

/** ----------------------------------------------------------------------------
 *  @Name:          Variable name
 *
 *  @Description:   Description of the variable.
 * -----------------------------------------------------------------------------
 */
static unsigned int glb_audio_playback_size = 0;

static snd_pcm_uframes_t glb_audio_playback_frame_nums = 0;

/*
 *  --------------------- Local function forward declaration -------------------
 */
static inline
unsigned int vcsystem_audio_is_running(vcsystem_audio_handle hdl)
{
    return (hdl->m_aud_working == 1);
}

static inline
unsigned int get_current_time_to_msec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return ((unsigned int)tv.tv_sec*1000 + tv.tv_usec/1000);
}

static inline
void* vcsystem_allocbuf( unsigned int bufsize, Bool fromSharedRegion)
{
		return malloc(bufsize);
}
static inline
void vcsystem_freebuf(void *buf, unsigned int bufsize, Bool fromSharedRegion)
{
		free(buf);
}

static status_t
vcsystem_audio_env_init(vcsystem_audio_handle hdl);

static status_t
vcsystem_audio_env_deinit(vcsystem_audio_handle hdl);

static status_t
vcsystem_audcap_device_init(vcsystem_audio_handle hdl, unsigned int channels,
        unsigned int sample_rate, unsigned int drv_buf_size);
static status_t
vcsystem_audcap_device_deinit(vcsystem_audio_handle hdl);
static status_t
vcsystem_audpla_device_init(vcsystem_audio_handle hdl, unsigned int channels,
        unsigned int sample_rate, unsigned int dev_id);
static status_t
vcsystem_audpla_device_deinit(vcsystem_audio_handle hdl);


static unsigned int
vcsystem_audio_get_backend_delay(vcsystem_audio_handle hdl);

static void *
vcsystem_audcap_thread_fxn(void *arg);
static void *
vcsystem_audpla_thread_fxn(void *arg);
/*
*		求音频数据的平均音量
*/

double volume_handle(char *buf,int buf_size)
{
				double sumVolume = 0.0;
				double avgVolume = 0.0;
				double volume = 0.0;
				int i;
				int v1,v2,temp;
				int value,value2;
				for(i = 0;i < buf_size;i+=2)
				{
					v1 = buf[i] & 0xFF;
          v2 = buf[i + 1] & 0xFF;
          temp = v1 + (v2 << 8);
          if (temp >= 0x8000) {
          		temp = 0xffff - temp;
          }
					sumVolume += fabs(temp);
				}
				avgVolume = sumVolume / buf_size / 2;
      	volume = log10(1 + avgVolume) * 10;

			// 25300 //25250 0.012 //26200 0.01316
			//26450 0.01316 //26900 0.014556	
				value  = volume * 1000;
				if(value > 25250){
						value2 = (value - 25250) * 0.012;
						if(value2 > 100)
								value2 = 100;
				}
				else{
						value2 = 0;
				}
			//	printf("--------------%d\n",value);
				printf("---- * 10 --------%d\n",value2);
  return volume;
}

/*
*	---------------------     fft        -------------/
*/
#if 1
static void fft4(char *x, char *y, int n)// n=(4)m;
{
	int i,j,k,m,i1,i2,i3,n1,n2;
	int a,b,c,e,r1,r2,r3,r4,s1,s2,s3,s4;
	int co1,co2,co3,si1,si2,si3;
	
	for(j=1,i=1;i<10;i++)
	{
		m=j;
		j=4*j;
		if(j==n)
		break;
	}
	
	n2=n;
	
	for(k=1;k<=m; k++)
	{
		n1=n2;
		n2=n2/4;
		e=6.28318530718/n1;
		a=0;
		
		for(j=0;j<n2;j++)
		{
			b=a+a;
			c=a+b;
			
			co1=cos(a);
			co2=cos(b);
			co3=cos(c);
			
			si1=sin(a);
			si2=sin(b);
			si3=sin(c);
			
			a=(j+1)*e;
			
			for(i=j;i<n;i=i+n1)
			{
				i1 = i+n2;
				i2= i1+n2;
				i3= i2 + n2;
				
				r1=x[i] + x[i2];
				r3=x[i] - x[i2];
				s1=y[i]+y[i2];
				s3=y[i]- y[i2];
				
				
				r2=x[i1] + x[i3];
				r4=x[i1]-x[i3];
				s2=y[i1]+y[i3];
				s4=y[i1]- y[i3];
				
				x[i]= r1-r2;
				r2=r1-r2;
				r1=r3-s4;
				r3=r3+s4;
				
				y[i]=s1+s2;
				s2=s1-s2;
				s1=s3+r4;
				s3=s3-r4;
				
				x[i1]=co1*r3+si1*s3;
				y[i1]=co1*s3-si1*r3;
				
				x[i2]=co2*r2+si2*s2;
				y[i2]=co2*s2-si2*r2;
				
				x[i3]=co3*r1+si3*s1;
				y[i3]=co3*s1-si3*r1;
				
			
			}
		
		}
	}

	n1 =n-1;
	
	for(j=0,i=0;i<n1;i++)
	{
		if(i<j)
		{
			r1=x[j];
			s1=y[j];
			x[j]=x[i];
			y[j]=y[i];
			x[i]=r1;
			y[i]=s1;
		
		}
		
		k=n/4;
		while(3*k<(j+1))
		{
			j=j-3*k;
			k=k/4;
		}
		j=j+k;
	}

}
#endif
void integer_to_str(unsigned int integer, char *str)
{
  unsigned int i=integer;
  unsigned char temp=0;
  unsigned char str_len=0;
  char str_buf[10]={0};
  for(i;i>0;i=i/10)
  {
    str_buf[temp]=i%10+'0';
    temp++;
  }
  //printf("str_buf=%s\n",str_buf);
  
  str_len=strlen(str_buf);
  while(str_len-1>=0)
  {
    *str++=*(str_buf+str_len-1);
    str_len--;
  }
}
 
void fft4_result_to_str(char *a,char *b, unsigned int n,char *str)
{
		unsigned int k=0;
		unsigned int tempc=0;	
		unsigned int i=0,j=0;


		tempc=sqrt(a[n]*a[n] + b[n]*b[n]);
		tempc=((unsigned long int)(tempc));
		tempc=tempc%100;
		integer_to_str(tempc,str);
	

}

/*
 *  --------------------- Public function definition ---------------------------
 */
status_t
vcsystem_audio_params_init(vcsystem_audio_handle hdl, const vcsystem_audio_params_t *p)
{
	status_t status = OSA_SOK;

	printf("vcsystem_audio_params_init: Enter(hdl=0x%x, p=0x%x)\n", hdl, p);

	if (hdl == NULL || p == NULL) {
		printf("vcsystem_audio_params_init: Invalid arguments.\n");
		return OSA_EFAIL;
	}

	/*
	 *  Initialize audio params.
	 */
	memcpy(&hdl->m_aud_params, p, sizeof(hdl->m_aud_params));

	printf("vcsystem_audio_params_init: Leave(status=0x%x)\n", status);

    return status;
}

status_t
vcsystem_audio_init(vcsystem_audio_handle hdl)
{
    status_t status = OSA_SOK;

    printf("vcsystem_audio_init: Enter(hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        printf("vcsystem_audio_init: Invalid arguments.\n");
        return OSA_EFAIL;
    }

    hdl->m_frame_id    = 0;
    hdl->m_frame_count = 0;

    /*
     *  Init audio env.
     */
    status = vcsystem_audio_env_init(hdl);
    if (OSA_ISERROR(status)) {
        return status;
    }

    /*
     *  Open audio capture device and playout device.
     */
    status = vcsystem_audcap_device_init(hdl,
                hdl->m_aud_params.m_capture_params.m_channel_nums,
                hdl->m_aud_params.m_capture_params.m_sample_rate,
                VCSYSTEM_AUDIO_BUFFER_SIZE);

    if (OSA_ISERROR(status)) {
        return status;
    }

    status = vcsystem_audpla_device_init(hdl,
                hdl->m_aud_params.m_playout_params.m_channel_nums,
                hdl->m_aud_params.m_playout_params.m_sample_rate,
                0);
    if (OSA_ISERROR(status)) {
        return status;
    }

	printf("vcsystem_audio_init: Leave(status=0x%x)\n", status);

    return status;
}

status_t
vcsystem_audio_start(vcsystem_audio_handle hdl, unsigned int is_encoding)
{
    status_t status = OSA_SOK;

    printf("vcsystem_audio_start: Enter(hdl=0x%x\n", hdl);

    if (hdl == NULL) {
        printf("vcsystem_audio_start: Invalid arguments.\n");
        return OSA_EFAIL;
    }

    /*
     *  Start audio capture and display.
     */
    hdl->m_aud_working = 1;

    status |= OSA_thrCreate(&hdl->m_cap_thd,
                vcsystem_audcap_thread_fxn,
                VCSYSTEM_AUDIO_TASK_PRI,
                VCSYSTEM_AUDIO_TASK_STACK_SIZE,
                (void *)hdl);

    status |= OSA_thrCreate(&hdl->m_pla_thd,
                vcsystem_audpla_thread_fxn,
                VCSYSTEM_AUDIO_TASK_PRI,
                VCSYSTEM_AUDIO_TASK_STACK_SIZE,
                (void *)hdl);

    printf("vcsystem_audio_start: Leave(status=0x%x)\n", status);

    return status;
}

status_t
vcsystem_audio_stop(vcsystem_audio_handle hdl, unsigned int is_encoding)
{
    status_t status = OSA_SOK;

    printf("vcsystem_audio_stop: Enter(hdl=0x%x\n", hdl);

    if (hdl == NULL) {
        printf("vcsystem_audio_stop: Invalid arguments.\n");
        return OSA_EFAIL;
    }

    /*
     *  Start audio capture and display.
     */
    hdl->m_aud_working = 0;
    usleep(100000);

    OSA_thrDelete(&hdl->m_cap_thd);

    OSA_thrDelete(&hdl->m_pla_thd);

    printf("vcsystem_audio_stop: Leave(status=0x%x)\n", status);

    return status;
}

status_t
vcsystem_audio_reinit(vcsystem_audio_handle hdl, const vcsystem_audio_params_t *p)
{
    status_t status = OSA_SOK;

    printf("vcsystem_audio_reinit: Enter(hdl=0x%x, p=0x%x)\n", hdl, p);

    if (hdl == NULL || p == NULL) {
        printf("vcsystem_audio_reinit: Invalid arguments.\n");
        return OSA_EFAIL;
    }

    printf("vcsystem_audio_reinit: Leave(status=0x%x)\n", status);

    return status;
}


status_t
vcsystem_audio_deinit(vcsystem_audio_handle hdl)
{
    status_t status = OSA_SOK;

    printf("vcsystem_audio_deinit: Enter(hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        printf("vcsystem_audio_deinit: Invalid arguments.\n");
        return OSA_EFAIL;
    }

    /*
     *  De-initialize audio capture and playout.
     */

    status |= vcsystem_audpla_device_deinit(hdl);

    status |= vcsystem_audcap_device_deinit(hdl);

    /*
     *  Finalize audio env.
     */
    status = vcsystem_audio_env_deinit(hdl);

    printf("vcsystem_audio_deinit: Leave(status=0x%x)\n", status);

    return status;
}


/*
 *  --------------------- Local function definition ----------------------------
 */
static status_t
vcsystem_audio_env_init(vcsystem_audio_handle hdl)
{
    int i;
    status_t status = OSA_SOK;

    printf("vcsystem_audio_env_init: Enter(hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        printf("vcsystem_audio_env_init: Invalid arguments.\n");
        return OSA_EFAIL;
    }

    /*
     *  Init audio env.
     */

	hdl->m_audio_buffer = NULL;
	hdl->m_audio_buffer = vcsystem_allocbuf(VCSYSTEM_AUDIO_SWAP_BUFFER_SIZE,FALSE);
	if (hdl->m_audio_buffer == NULL) {
		printf("vcsystem_audio_env_init: Failed to allocate audio buffer.\n");
        return OSA_EFAIL;
    }

    hdl->m_capture_frames[0].audBuf = hdl->m_audio_buffer + VCSYSTEM_AUDIO_MAX_BUFFER_SIZE * VCSYSTEM_AUDIO_MAX_QUEUE_LEN * 0;
    hdl->m_playout_frames[0].audBuf = hdl->m_audio_buffer + VCSYSTEM_AUDIO_MAX_BUFFER_SIZE * VCSYSTEM_AUDIO_MAX_QUEUE_LEN * 3;

    hdl->m_swap_buf     = hdl->m_audio_buffer + VCSYSTEM_AUDIO_SWAP_BUFFER_SIZE / 2;
    hdl->m_swap_bufsize = VCSYSTEM_AUDIO_SWAP_BUFFER_SIZE / 4;

    hdl->m_play_buf     = hdl->m_swap_buf + hdl->m_swap_bufsize;
    hdl->m_play_bufsize = VCSYSTEM_AUDIO_SWAP_BUFFER_SIZE / 4;

    for (i = 1; i < OSA_ARRAYSIZE(hdl->m_capture_frames); i++) {
        hdl->m_capture_frames[i].audBuf = hdl->m_capture_frames[i-1].audBuf + VCSYSTEM_AUDIO_MAX_BUFFER_SIZE;
    }
    for (i = 1; i < OSA_ARRAYSIZE(hdl->m_playout_frames); i++) {
        hdl->m_playout_frames[i].audBuf = hdl->m_playout_frames[i-1].audBuf + VCSYSTEM_AUDIO_MAX_BUFFER_SIZE;
    }

    OSA_queCreate(&hdl->m_capture_free_que,
            VCSYSTEM_AUDIO_MAX_QUEUE_LEN);
    OSA_queCreate(&hdl->m_capture_busy_que,
            VCSYSTEM_AUDIO_MAX_QUEUE_LEN);

    OSA_queCreate(&hdl->m_playout_free_que,
            VCSYSTEM_AUDIO_MAX_QUEUE_LEN);
    OSA_queCreate(&hdl->m_playout_busy_que,
            VCSYSTEM_AUDIO_MAX_QUEUE_LEN);

    for (i = 0; i < OSA_ARRAYSIZE(hdl->m_capture_frames); i++) {
        OSA_quePut(&hdl->m_capture_free_que,
                (Int32)&hdl->m_capture_frames[i], OSA_TIMEOUT_NONE);
    }

    for (i = 0; i < OSA_ARRAYSIZE(hdl->m_playout_frames); i++) {
        OSA_quePut(&hdl->m_playout_free_que,
                (Int32)&hdl->m_playout_frames[i], OSA_TIMEOUT_NONE);
    }

    printf("vcsystem_audio_env_init: Leave(status=0x%x)\n", status);

    return status;
}

static status_t
vcsystem_audio_env_deinit(vcsystem_audio_handle hdl)
{
    status_t status = OSA_SOK;

    printf("vcsystem_audio_env_deinit: Enter(hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        printf("vcsystem_audio_env_deinit: Invalid arguments.\n");
        return OSA_EFAIL;
    }

    /*
     *  Finalize audio env.
     */
    OSA_queDelete(&hdl->m_capture_free_que);
    OSA_queDelete(&hdl->m_capture_busy_que);
    
    OSA_queDelete(&hdl->m_playout_free_que);
    OSA_queDelete(&hdl->m_playout_busy_que);

	if (hdl->m_audio_buffer != NULL) {
		vcsystem_freebuf(hdl->m_audio_buffer, VCSYSTEM_AUDIO_SWAP_BUFFER_SIZE,FALSE);
		hdl->m_audio_buffer = NULL;
    }


    printf("vcsystem_audio_env_deinit: Leave(status=0x%x)\n", status);

    return status;
}

static status_t
vcsystem_audcap_device_init(vcsystem_audio_handle hdl, unsigned int channels,
        unsigned int sample_rate, unsigned int drv_buf_size)
{
    int err;
    snd_pcm_hw_params_t *hw_params = NULL;
    status_t status = OSA_SOK;

    printf("vcsystem_audcap_device_init: Enter(hdl=0x%x, "
            "channels=%d, sample_rate=%d, dvr_buf_size=%d)\n",
            hdl, channels, sample_rate, drv_buf_size);

    if (hdl == NULL) {
        printf("vcsystem_audcap_device_init: Invalid arguments.\n");
        return OSA_EFAIL;
    }

    if ((err = snd_pcm_open(&hdl->m_audcap_hdl,
                    VCSYSTEM_ALSA_CAPTURE_DEVICE, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audcap_device_init: %s\n", err, hdl->m_audcap_hdl);
    }

    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audcap_device_init: %s\n", err, hdl->m_audcap_hdl);
    }

    if ((err = snd_pcm_hw_params_any(hdl->m_audcap_hdl, hw_params)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audcap_device_init: %s\n", err, hdl->m_audcap_hdl);
    }

    if ((err = snd_pcm_hw_params_set_access(hdl->m_audcap_hdl,
                    hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audcap_device_init: %s\n", err, hdl->m_audcap_hdl);
    }

    if ((err = snd_pcm_hw_params_set_format(hdl->m_audcap_hdl,
                    hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audcap_device_init: %s\n", err, hdl->m_audcap_hdl);
    }

    if ((err = snd_pcm_hw_params_set_rate_near(hdl->m_audcap_hdl,
                    hw_params, &sample_rate, 0)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audcap_device_init: %s\n", err, hdl->m_audcap_hdl);
    }
    printf("vcsystem_audcap_device_init: Return sample rate = %d, channels=%d\n", sample_rate, channels);

    if ((err = snd_pcm_hw_params_set_channels(hdl->m_audcap_hdl, 
                    hw_params, channels)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audcap_device_init: %s\n", err, hdl->m_audcap_hdl);
    }

    if ((err = snd_pcm_hw_params_set_buffer_size(hdl->m_audcap_hdl,
                    hw_params, drv_buf_size)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audcap_device_init: %s\n", err, hdl->m_audcap_hdl);
    }


    if ((err = snd_pcm_hw_params(hdl->m_audcap_hdl, hw_params)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audcap_device_init: %s\n", err, hdl->m_audcap_hdl);
    }

    snd_pcm_hw_params_free(hw_params);
    if ((err = snd_pcm_prepare(hdl->m_audcap_hdl)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audcap_device_init: %s\n", err, hdl->m_audcap_hdl);
    }

    printf("vcsystem_audcap_device_init: Leave(status=0x%x)\n", status);

    return status;
}

static status_t
vcsystem_audcap_device_deinit(vcsystem_audio_handle hdl)
{
    status_t status = OSA_SOK;

    printf("vcsystem_audcap_device_deinit: Enter(hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        printf("vcsystem_audcap_device_deinit: Invalid arguments.\n");
        return OSA_EFAIL;
    }

    if (hdl->m_audcap_hdl != NULL) {
        snd_pcm_drain(hdl->m_audcap_hdl);
        snd_pcm_close(hdl->m_audcap_hdl);
        hdl->m_audcap_hdl = NULL;
    }

    printf("vcsystem_audcap_device_deinit: Leave(status=0x%x)\n", status);

    return status;
}

static status_t
vcsystem_audcap_device_reinit(vcsystem_audio_handle hdl)
{
    status_t status = OSA_SOK;

    status |= vcsystem_audcap_device_deinit(hdl);
    status |= vcsystem_audcap_device_init(hdl,
                hdl->m_aud_params.m_capture_params.m_channel_nums,
                hdl->m_aud_params.m_capture_params.m_sample_rate,
                VCSYSTEM_AUDIO_BUFFER_SIZE);
    return status;
}

static status_t
vcsystem_audpla_device_init(vcsystem_audio_handle hdl, unsigned int channels,
        unsigned int sample_rate, unsigned int dev_id)
{
    int err;
    snd_pcm_hw_params_t *params = NULL;
    status_t status = OSA_SOK;

    printf("vcsystem_audpla_device_init: Enter(hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        printf("vcsystem_audpla_device_init: Invalid arguments.\n");
        return OSA_EFAIL;
    }

    err = snd_pcm_open(&hdl->m_audpla_hdl, VCSYSTEM_ALSA_PLAYOUT_DEVICE,
            SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audpla_device_init: %s\n", err, hdl->m_audpla_hdl);
    }

    if ((err = snd_pcm_hw_params_malloc(&params)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audpla_device_init: %s\n", err, hdl->m_audpla_hdl);
    }

    if ((err = snd_pcm_hw_params_any(hdl->m_audpla_hdl, params)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audpla_device_init: %s\n", err, hdl->m_audpla_hdl);
    }

    if ((err = snd_pcm_hw_params_set_access(hdl->m_audpla_hdl,
                    params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audpla_device_init: %s\n", err, hdl->m_audpla_hdl);
    }

    if ((err = snd_pcm_hw_params_set_format(hdl->m_audpla_hdl,
                    params, SND_PCM_FORMAT_S16_LE)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audpla_device_init: %s\n", err, hdl->m_audpla_hdl);
    }

    if ((err = snd_pcm_hw_params_set_rate_near(hdl->m_audpla_hdl,
                    params, &sample_rate, 0)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audpla_device_init: %s\n", err, hdl->m_audpla_hdl);
    }

    if ((err = snd_pcm_hw_params_set_channels(hdl->m_audpla_hdl,
                    params, channels)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audpla_device_init: %s\n", err, hdl->m_audpla_hdl);
    }

    /* Use a buffer large enough to hold one period */
    /* set the buffer time */

    if ((err = snd_pcm_hw_params(hdl->m_audpla_hdl, params)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audpla_device_init: %s\n", err, hdl->m_audpla_hdl);
    }

    snd_pcm_hw_params_get_period_size(params,
            &glb_audio_playback_frame_nums, 0);

    /* 2 bytes/sample, 2 channel */
    glb_audio_playback_size = glb_audio_playback_frame_nums * 
                            channels * VCSYSTEM_AUDIO_SAMPLE_LEN; 

    printf("vcsystem_audpla_device_init: pbsize = %d, frame_nums=%d\n", glb_audio_playback_size, glb_audio_playback_frame_nums);

    snd_pcm_hw_params_free(params);

    ////////////////////////////////////////////////////////////////////////////
    /*
     *  Sw Params.
     */
#if 0

    snd_pcm_sw_params_t *swparams = NULL;
    snd_pcm_sw_params_malloc(&swparams);

    /* get the current swparams */
    err = snd_pcm_sw_params_current(hdl->m_audpla_hdl, swparams);
    if (err < 0) {
        printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
        return err;
    }
#if 0
    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    err = snd_pcm_sw_params_set_start_threshold(hdl->m_audpla_hdl, swparams, (buffer_size / period_size) * period_size);
    if (err < 0) {
        printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* allow the transfer when at least period_size samples can be processed */
    /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
    err = snd_pcm_sw_params_set_avail_min(hdl->m_audpla_hdl, swparams, period_event ? buffer_size : period_size);
    if (err < 0) {
        printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* enable period events when requested */
    if (period_event) {
        err = snd_pcm_sw_params_set_period_event(hdl->m_audpla_hdl, swparams, 1);
        if (err < 0) {
            printf("Unable to set period event: %s\n", snd_strerror(err));
            return err;
        }
    }
#endif
    snd_pcm_uframes_t val;

    

    val = 64;
    err =  snd_pcm_sw_params_set_silence_threshold(hdl->m_audpla_hdl, swparams, 1);
    if (err < 0) {
        printf("Unable to set selence threshold for playback: %s\n", snd_strerror(err));
        return err;
    }

    err =  snd_pcm_sw_params_set_silence_size(hdl->m_audpla_hdl, swparams, val);
    if (err < 0) {
        printf("Unable to set selence size for playback: %s\n", snd_strerror(err));
        return err;
    }

    err =  snd_pcm_sw_params_get_silence_threshold(swparams, &val);
    if (err < 0) {
        printf("Unable to get selence threshold for playback: %s\n", snd_strerror(err));
        return err;
    }
    printf("vcsystem_audpla_device_init: silence_threshold = %d\n", val );

    err =  snd_pcm_sw_params_get_silence_size(swparams, &val);
    if (err < 0) {
        printf("Unable to set selence size for playback: %s\n", snd_strerror(err));
        return err;
    }
    printf("vcsystem_audpla_device_init: silence_size = %d\n", val);

    /* write the parameters to the playback device */
    err = snd_pcm_sw_params(hdl->m_audpla_hdl, swparams);
    if (err < 0) {
        printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    snd_pcm_sw_params_free(swparams);

#endif

    if ((err = snd_pcm_prepare(hdl->m_audpla_hdl)) < 0) {
        DEBUG_PRINT_ERROR_AND_RETURN("vcsystem_audpla_device_init: %s\n", err, hdl->m_audpla_hdl);
    }

    printf("vcsystem_audpla_device_init: Leave(status=0x%x)\n", status);

    return status;
}

static status_t
vcsystem_audpla_device_deinit(vcsystem_audio_handle hdl)
{
    status_t status = OSA_SOK;

    printf("vcsystem_audpla_device_deinit: Enter(hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        printf("vcsystem_audpla_device_deinit: Invalid arguments.\n");
        return OSA_EFAIL;
    }

    if (hdl->m_audpla_hdl != NULL) {
        snd_pcm_drain(hdl->m_audpla_hdl);
        snd_pcm_close(hdl->m_audpla_hdl);
        hdl->m_audpla_hdl = NULL;
    }

    printf("vcsystem_audpla_device_deinit: Leave(status=0x%x)\n", status);

    return status;
}

static status_t
vcsystem_audpla_device_reinit(vcsystem_audio_handle hdl)
{
    status_t status = OSA_SOK;

    status |= vcsystem_audpla_device_deinit(hdl);
    status |= vcsystem_audpla_device_init(hdl,
                hdl->m_aud_params.m_playout_params.m_channel_nums,
                hdl->m_aud_params.m_playout_params.m_sample_rate,
                0);
    return status;
}

static status_t
vcsystem_audio_record(vcsystem_audio_handle hdl, 
        void *buf, unsigned sample_nums)
{
    int err;
    status_t status = OSA_SOK;
		//内存从声卡里读取声音数据
    err = snd_pcm_readi(hdl->m_audcap_hdl, buf, sample_nums);
    if (err < 0 || err != sample_nums) {

        /*
         *  TODO: Error.
         */
        printf("vcsystem_audio_record: err=%d, Reason: %s\n",
                err, snd_strerror(err));
        if (strcmp(snd_strerror(err), "Success")) {
            status = OSA_EFAIL;
        }
    }

    return status;
}

static status_t
vcsystem_audio_playout(vcsystem_audio_handle hdl,
        const unsigned char *buf, unsigned int sample_size)
{
    int retval = 0;
    unsigned int sample_played = 0;
    status_t status = OSA_SOK;

    while (sample_size >= glb_audio_playback_size) {
        retval = snd_pcm_writei(hdl->m_audpla_hdl,
                buf + sample_played, glb_audio_playback_frame_nums);

        if (retval == -EPIPE) {
            /* EPIPE means underrun */
            printf("vcsystem_audio_playout: err=%d, Reason: %s\n", 
                    retval, snd_strerror(retval));
            snd_pcm_prepare(hdl);
            break;
        } else if (retval < 0) {
            break;
        }

        sample_played += glb_audio_playback_size;
        sample_size   -= glb_audio_playback_size; 
    }

    if (sample_size > 0) {
        if (strcmp(snd_strerror(retval), "Success")) {
            status = OSA_EFAIL;
        }
    }

    return status;
}
static unsigned int
vcsystem_audio_get_backend_delay(vcsystem_audio_handle hdl)
{
    int                 status;
    snd_pcm_sframes_t   delay;
    unsigned int        aud_be_delay = 0;

    status = snd_pcm_delay(hdl->m_audpla_hdl, &delay);
    if (status == 0) {
        aud_be_delay = (delay * 1000) / 
            hdl->m_aud_params.m_playout_params.m_sample_rate;
    }

    return aud_be_delay;
}

static void
vcsystem_audio_queue_cleanup(OSA_QueHndl *from, OSA_QueHndl *to)
{
    AudFrm_Buf * p_aud_buf = NULL;
        
    while (OSA_queGetQueuedCount(from)) {
        OSA_queGet(from, (Int32)&p_aud_buf, OSA_TIMEOUT_NONE);
        OSA_quePut(to  , (Int32)&p_aud_buf, OSA_TIMEOUT_NONE);
    }
}

static void *
vcsystem_audcap_thread_fxn(void *arg)
{	
    AudFrm_Buf * p_aud_buf       = NULL;
    status_t status     = OSA_SOK;
    vcsystem_audio_handle hdl    = NULL;

    unsigned int samples_num     = VCSYSTEM_AUDIO_MAX_IN_SAMPLES;
    unsigned int samples_size    = 0;
    unsigned int accum_drop_size = 0;
    unsigned int accum_data_size = 0;
    unsigned char * buf          = NULL;
	
	FILE *fp = NULL;
	fp = fopen("/home/root/audio_cap.pcm","wb+");	
    
	hdl = VCSYSTEM_AUDIO_GET_HANDLE(arg);

    printf("vcsystem_audcap_thread_fxn: Enter(hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        printf("vcsystem_audcap_thread_fxn: Invalid arguments.\n");
        return NULL;
    }

    samples_size = samples_num * 
                   hdl->m_aud_params.m_capture_params.m_channel_nums *
                   VCSYSTEM_AUDIO_SAMPLE_LEN;

		//hdl->m_aud_working == 1
    while (vcsystem_audio_is_running(hdl)) {

        /*
         *  Get audio frame data to capture queue.
         */
        if (accum_data_size == 0) {
						//允许线程以无竞争的方式等待特定条件的发生
            status = OSA_queGet(&hdl->m_capture_free_que,
                    (Int32 *)(&p_aud_buf), OSA_TIMEOUT_NONE);
            if (OSA_ISERROR(status)) {
                if (accum_drop_size + samples_size < hdl->m_swap_bufsize) {
                    status = vcsystem_audio_record(hdl, 
                            hdl->m_swap_buf + accum_drop_size, samples_num);
                    if (!OSA_ISERROR(status)) {
                        accum_drop_size += samples_size;
                    }
                }
                continue;

            } else {
                if (accum_drop_size > 0) {
	//									printf("--------accum_drop_size Is > 0\n");
                    if (accum_drop_size > (VCSYSTEM_AUDIO_MAX_BUFFER_SIZE - samples_size)) {
                        accum_drop_size = VCSYSTEM_AUDIO_MAX_BUFFER_SIZE - samples_size;
                    }
                    memcpy(p_aud_buf->audBuf, hdl->m_swap_buf, accum_drop_size);
                    accum_data_size += accum_drop_size;
                    accum_drop_size  = 0;
                }
            }
        }
				// 内存从声卡里读取数据
//				printf("accum_data_size ------------%d\n",accum_data_size);
        status = vcsystem_audio_record(hdl,
                        p_aud_buf->audBuf + accum_data_size, samples_num);

        if (OSA_ISERROR(status)) {
            printf("vcsystem_audcap_thread_fxn: Failed to record audio.\n");
            status = vcsystem_audcap_device_reinit(hdl);
        } else {

            accum_data_size += samples_size;

            if (accum_data_size < VCSYSTEM_AUDIO_MIN_CAPUTRE_SIZE) {
                continue;
            }

            p_aud_buf->len = accum_data_size;
        }

				//获取当前时间并转化为毫秒
        p_aud_buf->timestamp = get_current_time_to_msec();
				//将p_aud_buf->audBuf里的数据写到/home/root/audio_cap.pcm里面
				status |= fwrite( p_aud_buf->audBuf, p_aud_buf->len,1,fp);

        status = OSA_quePut(&hdl->m_capture_busy_que,
                            (Int32)p_aud_buf, OSA_TIMEOUT_FOREVER);
    //  printf("vcsystem_audcap_thread_fxn: Accum audio data %d.\n", accum_data_size);
        accum_data_size = 0;
	
			//	 volume_handle(char *buf,int buf_size)
				long long aa;
       //	 p_aud_buf->timestamp = get_current_time_to_msec();
			 // 	printf("-------%ld\n",p_aud_buf->timestamp);
				volume_handle(p_aud_buf->audBuf,p_aud_buf->len);
		//		aa = get_current_time_to_msec();
		//		printf("---aa----%ld\n",aa);


#if 0
        p_aud_buf->timestamp = get_current_time_to_msec();
				printf("-------%d\n",p_aud_buf->timestamp);
				char y[4096]={0};
				char fft4_result_str[16]={0};
				fft4(p_aud_buf->audBuf,y,4096);
				fft4_result_to_str(p_aud_buf->audBuf,y, 1,fft4_result_str);
			//	printf("fft4_result_str==%s\n",fft4_result_str);
				aa = get_current_time_to_msec();
				int z =aa-p_aud_buf->timestamp;
				printf("---aa----%d\n",aa);
#endif 
#if 0
				double sumVolume = 0.0;
				double avgVolume = 0.0;
				double volume = 0.0;
				int i;
				int v1,v2,temp;
				for(i = 0;i < p_aud_buf->len;i+=2)
				{
					v1 = p_aud_buf->audBuf[i] & 0xFF;
          v2 = p_aud_buf->audBuf[i + 1] & 0xFF;
          temp = v1 + (v2 << 8);
          if (temp >= 0x8000) {
          		temp = 0xffff - temp;
          }
					sumVolume += fabs(temp);
		//			sumVolume += fabs(p_aud_buf->audBuf[i]);
				}
				avgVolume = sumVolume / p_aud_buf->len / 2;
      	volume = log10(1 + avgVolume) * 10;

				printf("volume------------------------%lf\n",volume);

#endif 
    }
    printf("vcsystem_audcap_thread_fxn: Leave(status=0x%x)\n", status);

    return ((void *)status);
}

static void *
vcsystem_audpla_thread_fxn(void *arg)
{
    AudFrm_Buf * p_aud_buf    = NULL;
    status_t status  = OSA_SOK;
    vcsystem_audio_handle hdl = NULL;

    unsigned char * buf       = NULL;
    unsigned int    data_size = 0;
    unsigned int    accum_data_size = VCSYSTEM_AUDIO_MIN_CAPUTRE_SIZE;
    AudFrm_BufList  freeBufList;
    unsigned int    mute_flag;
    int             is_first_frame_received = 0;

    unsigned long long cur_timestamp = 0; //get_current_time_to_msec();

    hdl = VCSYSTEM_AUDIO_GET_HANDLE(arg);

    printf("vcsystem_audpla_thread_fxn: Enter(hdl=0x%x)\n", hdl);

    if (hdl == NULL) {
        printf("vcsystem_audpla_thread_fxn: Invalid arguments.\n");
        return NULL;
    }

    while (vcsystem_audio_is_running(hdl)) {
        /*
         *  Get audio frame data to playout queue.
         */
        status = OSA_queGet(&hdl->m_capture_busy_que,
                            (Int32 *)(&p_aud_buf), OSA_TIMEOUT_NONE);

        if (!OSA_ISERROR(status)) {
            memcpy(hdl->m_play_buf, p_aud_buf->audBuf, p_aud_buf->len);
            accum_data_size = p_aud_buf->len;
            cur_timestamp   = p_aud_buf->timestamp;

            status = OSA_quePut(&hdl->m_capture_free_que,
                    (Int32)p_aud_buf, OSA_TIMEOUT_FOREVER);

            if (!is_first_frame_received) {
                is_first_frame_received = 1;
            }
        }

        if (!is_first_frame_received) {
            usleep(50000);
            continue;
        }

        buf       = hdl->m_play_buf;
        data_size = accum_data_size;
        
        //cur_timestamp = get_current_time_to_msec();

        status = vcsystem_audio_playout(hdl, buf, data_size);

        if (OSA_ISERROR(status)) {
            printf("vcsystem_audpla_thread_fxn: Failed to playout audio.\n");
            status = vcsystem_audpla_device_reinit(hdl);
        }

        //hdl->m_frame_count += 1;

#if 0
				char y[4096]={0};
				char fft4_result_str[16]={0};
				fft4(buf,y,4096);
				fft4_result_to_str(buf,y, 1,fft4_result_str);
				printf("fft4_result_str==%s\n",fft4_result_str);
#endif 
    }

    vcsystem_audio_queue_cleanup(&hdl->m_capture_busy_que,
            &hdl->m_capture_free_que);

    printf("vcsystem_audpla_thread_fxn: Leave(status=0x%x)\n", status);

    return ((void *)status);
}



#if defined(__cplusplus)
}
#endif

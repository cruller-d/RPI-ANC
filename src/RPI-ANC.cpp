#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <alsa/asoundlib.h>

#include "latency_Estimator/Latency_Estimator.h"

#define BUFSIZE 4096
int             restarting;
int             nchannels = 2;
int             buffer_size = BUFSIZE;
int             sample_rate = 16000;
int             bits = 16;

short in_buf[BUFSIZE * 2];
short out_buf[BUFSIZE * 2];

//char           *snd_device_in  = "plughw:0,0";
//char           *snd_device_out = "plughw:0,0";
snd_pcm_t      *playback_handle;
snd_pcm_t      *capture_handle;

int configure_alsa_audio (snd_pcm_t *device, int channels) {
	snd_pcm_hw_params_t * hw_params;
	int err;
	snd_pcm_uframes_t frames;
	unsigned fragments = 2;

	/* allocate memory for hardware parameter structure */
	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
	    fprintf (stderr, "cannot allocate parameter structure (%s)\n", snd_strerror(err));
	    return 1;
	}//
	/* fill structure from current audio parameters */
	if ((err = snd_pcm_hw_params_any(device, hw_params)) < 0) {
	    fprintf (stderr, "cannot initialize parameter structure (%s)\n", snd_strerror(err));
	    return 1;
	}

	/* set access type, sample rate, sample format, channels */
	if ((err = snd_pcm_hw_params_set_access(device, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
	    fprintf (stderr, "cannot set access type: %s\n", snd_strerror(err));
	    return 1;
	}
	// bits = 16
	if ((err = snd_pcm_hw_params_set_format(device, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
	    fprintf (stderr, "cannot set sample format: %s\n", snd_strerror(err));
	    return 1;
	}
	unsigned sampling_rate = sample_rate;
	int dir = 0;
	if ((err = snd_pcm_hw_params_set_rate_near(device, hw_params, &sampling_rate, &dir)) < 0) {
	    fprintf (stderr, "cannot set sample rate: %s\n", snd_strerror(err));
	    return 1;
	}
	if (sampling_rate != sample_rate) {
	    fprintf(stderr, "Could not set requested sample rate, asked for %d got %d\n", sample_rate, sampling_rate);
	    sample_rate = sampling_rate;
	}
	if ((err = snd_pcm_hw_params_set_channels(device, hw_params, channels)) < 0) {
	    fprintf (stderr, "cannot set channel count: %s\n", snd_strerror(err));
	    return 1;
	}
    if ((err = snd_pcm_hw_params_set_periods_near(device, hw_params, &fragments, &dir)) < 0) {
        fprintf(stderr, "Error setting # fragments to %d: %s\n", fragments,
                snd_strerror(err));
        return 1;
    }
    int frame_size = channels * (bits / 8);
    frames = buffer_size / frame_size * fragments;
    if ((err = snd_pcm_hw_params_set_buffer_size_near(device, hw_params, &frames)) < 0) {
        fprintf(stderr, "Error setting buffer_size %d frames: %s\n", frames,
                snd_strerror(err));
        return 1;
    }
    if (buffer_size != frames * frame_size / fragments) {
        fprintf(stderr, "Could not set requested buffer size, asked for %d got %d\n", buffer_size, frames * frame_size / fragments);
        buffer_size = frames * frame_size / fragments;
    }
    if ((err = snd_pcm_hw_params(device, hw_params)) < 0) {
        fprintf(stderr, "Error setting HW params: %s\n",
                snd_strerror(err));
        return 1;
    }
    return 0;
}

short l_buf[BUFSIZE];
short r_buf[BUFSIZE];
LatencyEstimator le;
bool MasterTick(short * output, short* input, int frames) {
	for (int i = 0; i < frames; ++i) {
		char* i_byte = (char*)(input + 2 * i);
  	short sample_right = i_byte[0] + (i_byte[1] << 8);
  	short sample_left = i_byte[2] + (i_byte[3] << 8);
  	l_buf[i] = sample_left;
  	r_buf[i] = sample_right;
  	char* o_byte = (char*)(output + 2 * i);
  	o_byte[0] = sample_right & 0xff;
  	o_byte[1] = (sample_right >> 8) & 0xff;
  	o_byte[2] = sample_left & 0xff;
  	o_byte[3] = (sample_left >> 8) & 0xff;
  }
	le.PushSignals(l_buf, r_buf, 512);
  return true;
}

int main (int argc, char * argv[]) {
  int err;//
  if ((err = snd_pcm_open(&playback_handle, argv[1], SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    fprintf(stderr, "cannot open output audio device %s: %s\n", argv[1], snd_strerror(err));
    exit(1);
  }
  if ((err = snd_pcm_open(&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf(stderr, "cannot open input audio device %s: %s\n", argv[1], snd_strerror(err));
    exit(1);
  }
  printf("pcm open\n");

  configure_alsa_audio(playback_handle,  nchannels);
  configure_alsa_audio(capture_handle, nchannels);
  printf("alsa audio configured\n");

  int frames, inframes, outframes, frame_size;
  int fragments = 2;

  while (1) {
    frame_size = nchannels * (bits / 8);
    frames = buffer_size / frame_size;

    if (restarting) {
      restarting = 0;
      /* drop any output we might got and stop */
      snd_pcm_drop(capture_handle);
      snd_pcm_drop(playback_handle);
      /* prepare for use */
      snd_pcm_prepare(capture_handle);
      snd_pcm_prepare(playback_handle);

      /* fill the whole output buffer */
      for (int i = 0; i < fragments; i += 1) {
      		snd_pcm_writei(playback_handle, out_buf, frames);
      }
    }

    while ((inframes = snd_pcm_readi(capture_handle, in_buf, frames)) < 0) {
      if (inframes == -EAGAIN)
	continue;
      // by the way, writing to terminal emulator is costly if you use
      // bad emulators like gnome-terminal, so don't do this.
      fprintf(stderr, "Input buffer overrun\n");
      restarting = 1;
      snd_pcm_prepare(capture_handle);
    }
    if (inframes != frames)
      fprintf(stderr, "Short read from capture device: %d, expecting %d\n", inframes, frames);

    // now processes the frames
    MasterTick(out_buf, in_buf, inframes);
    //do_something(rdbuf, inframes);

    while ((outframes = snd_pcm_writei(playback_handle, out_buf, inframes)) < 0) {
      if (outframes == -EAGAIN)
        continue;
      fprintf(stderr, "Output buffer underrun\n");
      restarting = 1;
      snd_pcm_prepare(playback_handle);
    }
    if (outframes != inframes) {
      fprintf(stderr, "Short write to playback device: %d, expecting %d\n", outframes, frames);
    }
  }
  return 0;
}

//
//	short buf[4096 * 4];
//
//	int
//	playback_callback (snd_pcm_sframes_t nframes)
//	{
//		int err;
//
//		printf ("playback callback called with %d frames\n", (int)(nframes));
//
//		/* ... fill buf with data ... */
//
//		if ((err = snd_pcm_writei (playback_handle, buf, nframes)) < 0) {
//			fprintf (stderr, "write failed (%s)\n", snd_strerror (err));
//		}
//
//		return err;
//	}
//
//	int main (int argc, char *argv[])
//	{
//
//		snd_pcm_hw_params_t *hw_params;
//		snd_pcm_sw_params_t *sw_params;
//		snd_pcm_sframes_t frames_to_deliver;
//		int nfds;
//		int err;
//		struct pollfd *pfds;
//
//		if ((err = snd_pcm_open (&playback_handle, argv[1], SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
//			fprintf (stderr, "cannot open audio device %s (%s)\n",
//				 argv[1],
//				 snd_strerror (err));
//			exit (1);
//		}
//		printf("audio device opened\n");
//
//		if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
//			fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		printf("hw_params space allocated\n");
//
//		if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
//			fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		printf("hw param structure initialized\n");
//
//		if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
//			fprintf (stderr, "cannot set access type (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		printf("access type set\n");
//
//		if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, (snd_pcm_format_t)(SND_PCM_FORMAT_S16_LE))) < 0) {
//			fprintf (stderr, "cannot set sample format (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		printf("sample format set\n");
//
//		unsigned sampling_rate = 44100;
//		int direction = 0;
//		if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &sampling_rate, &direction)) < 0) {
//			fprintf (stderr, "cannot set sample rate (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		printf("sample rate set\n");
//
//		if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 2)) < 0) {
//			fprintf (stderr, "cannot set channel count (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		printf("channel count set\n");
//
//		if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
//			fprintf (stderr, "cannot set parameters (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		printf("parameters set\n");
//
//		snd_pcm_hw_params_free (hw_params);
//		printf("hw_params freed\n");
//
//		/* tell ALSA to wake us up whenever 4096 or more frames
//		   of playback data can be delivered. Also, tell
//		   ALSA that we'll start the device ourselves.
//		*/
//
//		if ((err = snd_pcm_sw_params_malloc (&sw_params)) < 0) {
//			fprintf (stderr, "cannot allocate software parameters structure (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		if ((err = snd_pcm_sw_params_current (playback_handle, sw_params)) < 0) {
//			fprintf (stderr, "cannot initialize software parameters structure (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		if ((err = snd_pcm_sw_params_set_avail_min (playback_handle, sw_params, 4096)) < 0) {
//			fprintf (stderr, "cannot set minimum available count (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		if ((err = snd_pcm_sw_params_set_start_threshold (playback_handle, sw_params, 0U)) < 0) {
//			fprintf (stderr, "cannot set start mode (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		if ((err = snd_pcm_sw_params (playback_handle, sw_params)) < 0) {
//			fprintf (stderr, "cannot set software parameters (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//
//		/* the interface will interrupt the kernel every 4096 frames, and ALSA
//		   will wake up this program very soon after that.
//		*/
//
//		if ((err = snd_pcm_prepare (playback_handle)) < 0) {
//			fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
//				 snd_strerror (err));
//			exit (1);
//		}
//		printf("everything seems to be in place, starting loop\n");
//		while (1) {
//
//			/* wait till the interface is ready for data, or 1 second
//			   has elapsed.
//			*/
//
//			if ((err = snd_pcm_wait (playback_handle, 1000)) < 0) {
//			        fprintf (stderr, "poll failed (%s)\n", strerror (errno));
//			        break;
//			}
//
//			/* find out how much space is available for playback data */
//
//			if ((frames_to_deliver = snd_pcm_avail_update (playback_handle)) < 0) {
//				if (frames_to_deliver == -EPIPE) {
//					fprintf (stderr, "an xrun occured\n");
//					break;
//				} else {
//					fprintf (stderr, "unknown ALSA avail update return value (%d)\n",
//						 frames_to_deliver);
//					break;
//				}
//			}
//
//			frames_to_deliver = frames_to_deliver > 4096 ? 4096 : frames_to_deliver;
//
//			/* deliver the data */
//
//			if (playback_callback (frames_to_deliver) != frames_to_deliver) {
//			        fprintf (stderr, "playback callback failed\n");
//				break;
//			}
//		}
//
//		snd_pcm_close (playback_handle);
//		exit (0);
//		return 0;
//	}

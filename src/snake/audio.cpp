#include "./audio.h"
#include <thread>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>

#define SAMPLE_RATE 44100
#define CHANNELS 2
#define FSIZE 2*CHANNELS

Audio::~Audio() {
    forceQuit = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}


unsigned long Audio::get_file_size(const char *path) {
	unsigned long filesize = 0;
	struct stat statbuff;
	if (stat(path, &statbuff) > -1) {
		filesize = statbuff.st_size;
	}
	return filesize;
}

void Audio::stop() {
    forceQuit = true;
}

bool Audio::init() {
    int i;
    int err;
    snd_pcm_hw_params_t *hw_params;

    err = snd_pcm_open(&_soundDevice, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cout << "Init: cannot open default audio device " << " (" << snd_strerror (err) << ")" << std::endl;
        return false;
    }

    // Allocate the hardware parameter structure.
    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        std::cout << "Init: cannot allocate hardware parameter structure (" << snd_strerror(err) << ")" << std::endl;
        return false;
    }

    if ((err = snd_pcm_hw_params_any(_soundDevice, hw_params)) < 0) {
        std::cout << "Init: cannot initialize hardware parameter structure (" << snd_strerror(err) << ")" << std::endl;
        return false;
    }

    // Enable resampling.
    unsigned int resample = 1;
    err = snd_pcm_hw_params_set_rate_resample(_soundDevice, hw_params, resample);
    if (err < 0) {
        std::cout << "Init: Resampling setup failed for playback: " << snd_strerror(err) << std::endl;
        return err;
    }

    // Set access to RW interleaved.
    if ((err = snd_pcm_hw_params_set_access(_soundDevice, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        std::cout << "Init: cannot set access type (" << snd_strerror(err) << ")" << std::endl;
        return false;
    }

    if ((err = snd_pcm_hw_params_set_format(_soundDevice, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        std::cout << "Init: cannot set sample format (" << snd_strerror(err) << ")" << std::endl;
        return false;
    }

    // Set channels to stereo (2).
    if ((err = snd_pcm_hw_params_set_channels(_soundDevice, hw_params, 2)) < 0) {
        std::cout << "Init: cannot set channel count (" << snd_strerror(err) << ")" << std::endl;
        return false;
    }

    // Set sample rate.
    unsigned int actualRate = 44100;
    if ((err = snd_pcm_hw_params_set_rate_near(_soundDevice, hw_params, &actualRate, 0)) < 0) {
        std::cout << "Init: cannot set sample rate to 44100. (" << snd_strerror(err) << ")"  << std::endl;
        return false;
    }
    if (actualRate < 44100) {
        std::cout << "Init: sample rate does not match requested rate. (" << "44100 requested, " << actualRate << " acquired)" << std::endl;
    }

    // Apply the hardware parameters that we've set.
    if ((err = snd_pcm_hw_params(_soundDevice, hw_params)) < 0) {
        std::cout << "Init: cannot set parameters (" << snd_strerror(err) << ")" << std::endl;
        return false;
    }

    // Get the buffer size.
    snd_pcm_uframes_t bufferSize;
    snd_pcm_hw_params_get_buffer_size(hw_params, &bufferSize);
    // If we were going to do more with our sound device we would want to store
    // the buffer size so we know how much data we will need to fill it with.
    //std::cout << "Init: Buffer size = " << bufferSize << " frames." << std::endl;

    // Display the bit size of samples.
    //std::cout << "Init: Significant bits for linear samples = " << snd_pcm_hw_params_get_sbits(hw_params) << std::endl;

    // Free the hardware parameters now that we're done with them.
    snd_pcm_hw_params_free(hw_params);

    // Prepare interface for use.
    if ((err = snd_pcm_prepare(_soundDevice)) < 0) {
        std::cout << "Init: cannot prepare audio interface for use (" << snd_strerror (err) << ")" << std::endl;
        return false;
    }

	snd_pcm_hw_params_get_period_size(hw_params, &frames, 0);
	// 1 frame = channels * sample_size.
	frame_size = frames * FSIZE; /* 2 bytes/sample, 1 channels */

    return true;
}

bool Audio::loadFile(const char* filename) {
    unsigned long filesize = get_file_size(filename);
    if (filesize <= 0) {
        return false;
    }
    if (filesize % frame_size == 0) {
        frames_count = filesize / frame_size;
    }
    else {
        frames_count = filesize / frame_size + 1;
    }

	int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return false;
    }

    buffers = (char**)malloc(sizeof(char*) * frames_count);
    for (int i = 0; i < frames_count; ++i) {
        buffers[i] = (char*)malloc(frame_size);
		read(fd, buffers[i], frame_size);
    }

	close(fd);
    return true;
}

bool Audio::play(const char* filename, bool loop) {
    if (isPlaying) {
        //std::cout << "Is playing\n";
        return false;
    }
    isPlaying = true;

    if (!init()) {
        return false;
    }

    if (!loadFile(filename)) {
        return false;
    }

    std::thread t(thread_play, loop, _soundDevice, frames, frames_count,
                  &buffers, &forceQuit, &isPlaying);
    t.detach();

	return true;
}

bool Audio::playOnce(const char* filename) {
    return play(filename, false);
}

bool Audio::playLoop(const char* filename) {
    return play(filename, true);
}

void Audio::thread_play(bool loop, snd_pcm_t* _soundDevice,
                        snd_pcm_uframes_t frames, int frames_count,
                        char*** p_buffers, bool* p_force_quit, bool* p_is_playing)
{
    *p_force_quit = false;
    *p_is_playing = true;
    int rc = 0, err = 0;
    do {
        if (*p_force_quit) {
            break;
        }
        for (int i = 1; i < frames_count; ++i) {
            if (*p_force_quit) {
                break;
            }
            rc = snd_pcm_writei(_soundDevice, (*p_buffers)[i], frames);
            if (rc == -EPIPE) {
                fprintf(stderr, "underrun occurred\n");
                err = snd_pcm_prepare(_soundDevice);
                if( err <0){
                    fprintf(stderr, "can not recover from underrun: %s\n",snd_strerror(err));
                }
            } 
            else if (rc < 0) {
                fprintf(stderr,"error from writei: %s\n", snd_strerror(rc));
            }  
            else if (rc != (int)frames) {
                fprintf(stderr,"short write, write %d frames\n", rc);
            }
        }
    } while (loop);

    for (int i = 0; i < frames_count; ++i) {
        free((*p_buffers)[i]);
    }
    free(*p_buffers);
    *p_buffers = nullptr;

	snd_pcm_drain(_soundDevice);
    snd_pcm_close(_soundDevice);

    *p_is_playing = false;
    *p_force_quit = false;
}


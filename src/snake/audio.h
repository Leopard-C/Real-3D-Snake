#pragma once

#include <alsa/asoundlib.h>

class Audio {
public:
    ~Audio();

    bool playOnce(const char* filename);
    bool playLoop(const char* filename);

    void stop();

private:
    bool init();

    unsigned long get_file_size(const char *path);
    bool loadFile(const char* filename);
    bool play(const char* filename, bool loop);

    static void thread_play(bool loop, snd_pcm_t* _soundDevice, 
                            snd_pcm_uframes_t frames, int frames_count,
                            char*** p_buffers, bool* p_force_quit, bool* p_is_playing);

private:
    snd_pcm_t *_soundDevice;
    snd_pcm_uframes_t frames;
    int frames_count;
    int frame_size;
    char** buffers;

    bool forceQuit = false;
    bool isPlaying = false;
};


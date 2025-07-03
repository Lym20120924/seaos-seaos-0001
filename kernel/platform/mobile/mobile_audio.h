#ifndef MOBILE_AUDIO_H
#define MOBILE_AUDIO_H

#include <stdint.h>

#define MOBILE_AUDIO_PROFILE_PHONE       0
#define MOBILE_AUDIO_PROFILE_MULTIMEDIA  1
#define MOBILE_AUDIO_PROFILE_GAMING      2

void mobile_audio_init(void);
void mobile_audio_set_profile(int profile);
int mobile_audio_get_profile(void);

int mobile_audio_init_playback(void);
int mobile_audio_init_capture(void);
int mobile_audio_init_voice_processing(void);

void mobile_audio_enable_jack_detection(int enable);

#endif
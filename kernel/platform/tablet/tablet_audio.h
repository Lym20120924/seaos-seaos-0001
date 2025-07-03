#ifndef TABLET_AUDIO_H
#define TABLET_AUDIO_H

#include <stdint.h>

#define TABLET_AUDIO_PROFILE_MULTIMEDIA  0
#define TABLET_AUDIO_PROFILE_VOICE       1
#define TABLET_AUDIO_PROFILE_GAMING      2

void tablet_audio_init(void);
void tablet_audio_set_profile(int profile);
int tablet_audio_get_profile(void);

int tablet_audio_init_playback(void);
int tablet_audio_init_capture(void);

void tablet_audio_set_volume(int volume);
int tablet_audio_get_volume(void);
void tablet_audio_mute(int mute);
int tablet_audio_is_muted(void);

void tablet_audio_enable_jack_detection(int enable);
int tablet_audio_is_headphones_connected(void);

#endif
#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

/* Play a single note at given frequency (Hz). frequency=0 stops the speaker. */
void pc_speaker_play_note(uint32_t frequency);
void pc_speaker_stop(void);

/* Start a song. frequencies and durations are arrays of size length.
   Durations are in milliseconds. */
void audio_play_song(const uint32_t* frequencies, const uint32_t* durations, uint32_t length);

/* To be called from the IRQ0 timer_handler */
void audio_tick(void);

/* Play the iconic DOOM E1M1 theme */
void audio_play_doom_theme(void);

#endif

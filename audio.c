#include "audio.h"
#include "io.h"
#include "timer.h"

/* Sequencer state */
static const uint32_t* current_freqs = 0;
static const uint32_t* current_durs = 0;
static uint32_t song_length = 0;
static uint32_t current_note_idx = 0;
static uint32_t ticks_remaining = 0;
static int is_playing = 0;

void pc_speaker_play_note(uint32_t frequency) {
    if (frequency == 0) {
        pc_speaker_stop();
        return;
    }

    uint32_t div = 1193180 / frequency;
    
    /* Set PIT to the desired frequency */
    outb(0x43, 0xb6);
    outb(0x42, (uint8_t) (div) );
    outb(0x42, (uint8_t) (div >> 8));
    
    /* Play the sound using the PC speaker */
    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void pc_speaker_stop(void) {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void audio_play_song(const uint32_t* frequencies, const uint32_t* durations, uint32_t length) {
    current_freqs = frequencies;
    current_durs = durations;
    song_length = length;
    current_note_idx = 0;
    
    if (length > 0) {
        ticks_remaining = current_durs[0] / 10; /* Assuming 100Hz timer (1 tick = 10ms) */
        pc_speaker_play_note(current_freqs[0]);
        is_playing = 1;
    }
}

void audio_tick(void) {
    if (!is_playing) return;
    
    if (ticks_remaining > 0) {
        ticks_remaining--;
    }
    
    if (ticks_remaining == 0) {
        current_note_idx++;
        if (current_note_idx >= song_length) {
            is_playing = 0;
            pc_speaker_stop();
            return;
        }
        
        ticks_remaining = current_durs[current_note_idx] / 10;
        pc_speaker_play_note(current_freqs[current_note_idx]);
    }
}

/* DOOM E1M1: At Doom's Gate */
#define E2  82
#define E3  164
#define D3  146
#define C3  130
#define Bb2 116
#define B2  123
#define P   0  /* Pause */

static const uint32_t doom_freqs[] = {
    E2, P, E2, P, E3, P, E2, P, E2, P, D3, P, E2, P, E2, P, C3, P, E2, P, E2, P, Bb2, P, E2, P, E2, P, B2, C3, P
};

static const uint32_t doom_durs[] = {
    110, 20, 110, 20, 110, 20, 110, 20, 110, 20, 110, 20, 110, 20, 110, 20, 110, 20, 110, 20, 110, 20, 110, 20, 110, 20, 110, 20, 130, 130, 400
};

void audio_play_doom_theme(void) {
    audio_play_song(doom_freqs, doom_durs, sizeof(doom_freqs) / sizeof(doom_freqs[0]));
}

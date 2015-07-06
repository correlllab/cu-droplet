#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "scheduler.h"

#define MAX_NOTES_PER_SONG 128
#define NOTE_SWITCH_DUR_MS 10

typedef struct{
	uint16_t dur;
	uint8_t note;
} sound_str;

uint8_t song_playing;


void speaker_init();
void play_song(uint8_t* notes, uint16_t* durs, uint8_t song_len, uint8_t pauses);

#ifdef AUDIO_DROPLET

	void emit_sound(uint16_t period, uint32_t duration);
	void start_sound(uint8_t note);
	void switch_sound();
	void brief_pause();
	void stop_sound();

#endif


#define NOTE_B7		62
#define NOTE_AS7	66
#define NOTE_A7		70
#define NOTE_GS7	74
#define NOTE_G7		79
#define NOTE_FS7	83
#define NOTE_F7		88
#define NOTE_E7		94
#define NOTE_DS7	99
#define NOTE_D7		105
#define NOTE_CS7	112
#define NOTE_C7		118
#define NOTE_B6		126
#define NOTE_AS6	133
#define NOTE_A6		141
#define NOTE_GS6	149
#define NOTE_G6		158
#define NOTE_FS6	168
#define NOTE_F6		178
#define NOTE_E6		189
#define NOTE_DS6	200
#define NOTE_D6		212
#define NOTE_CS6	224
#define NOTE_C6		238
#define NOTE_B5		252
#define NOTE_AS5	267
#define NOTE_A5		283
#define NOTE_GS5	300
#define NOTE_G5		318
#define NOTE_FS5	337
#define NOTE_F5		357
#define NOTE_E5		378
#define NOTE_DS5	401
#define NOTE_D5		425
#define NOTE_CS5	450
#define NOTE_C5		477
#define NOTE_B4		505
#define NOTE_AS4	535
#define NOTE_A4		567
#define NOTE_GS4	601
#define NOTE_G4		637
#define NOTE_FS4	675
#define NOTE_F4		715
#define NOTE_E4		757
#define NOTE_DS4	803
#define NOTE_D4		850
#define NOTE_CS4	901
#define NOTE_C4		955
#define NOTE_B3		1011
#define NOTE_AS3	1072
#define NOTE_A3		1135
#define NOTE_GS3	1203
#define NOTE_G3		1275
#define NOTE_FS3	1350
#define NOTE_F3		1431
#define NOTE_E3		1516
#define NOTE_DS3	1606
#define NOTE_D3		1702
#define NOTE_CS3	1803
#define NOTE_C3		1910
#define NOTE_B2		2024
#define NOTE_AS2	2144
#define NOTE_A2		2272
#define NOTE_GS2	2407
#define NOTE_G2		2550
#define NOTE_FS2	2702
#define NOTE_F2		2862
#define NOTE_E2		3033
#define NOTE_DS2	3213
#define NOTE_D2		3404
#define NOTE_CS2	3607
#define NOTE_C2		3821

extern const uint16_t note_values[6][12]; // defined in speaker.c

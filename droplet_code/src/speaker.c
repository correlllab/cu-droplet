#include "speaker.h"

#ifdef AUDIO_DROPLET
	const uint16_t note_values[6][12] = {{NOTE_C2, NOTE_CS2, NOTE_D2, NOTE_DS2, NOTE_E2, NOTE_F2, NOTE_FS2, NOTE_G2, NOTE_GS2, NOTE_A2, NOTE_AS2, NOTE_B2},
	{NOTE_C3, NOTE_CS3, NOTE_D3, NOTE_DS3, NOTE_E3, NOTE_F3, NOTE_FS3, NOTE_G3, NOTE_GS3, NOTE_A3, NOTE_AS3, NOTE_B3},
	{NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4},
	{NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5},
	{NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6},
	{NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7}};

	uint8_t local_notes[MAX_NOTES_PER_SONG];
	uint16_t local_durs[MAX_NOTES_PER_SONG];
	uint8_t curr_song_len;
	uint8_t auto_pause;
	volatile uint16_t curr_song_idx;

void start_sound(uint8_t note)
{
	TCC0.CNT = 0;	
	if(note==0xFF)
	{
		TCC0.CCA = 0;
		TCC0.CCB = 0;
		PORTC.OUTCLR = PIN0_bm;
	}
	else
	{
		TCC0.CCA = note_values[note>>4][note&0x0F];
		TCC0.CCB = note_values[note>>4][note&0x0F];
		PORTC.OUTSET = PIN0_bm;
	}
	TCC0.CTRLB |= TC0_CCBEN_bm;
}

void switch_sound()
{
	if(curr_song_idx>=curr_song_len)
	{
		song_playing=0;
		stop_sound();
		return;
	}	
	uint8_t note = local_notes[curr_song_idx];
	uint16_t dur = local_durs[curr_song_idx];
	curr_song_idx++;
	TCC0.CNT = 0;	
	if(note==0xFF)
	{
		TCC0.CCA = 0;
		TCC0.CCB = 0;
		PORTC.OUTCLR = PIN0_bm;
	}
	else
	{
		TCC0.CCA = note_values[note>>4][note&0x0F];
		TCC0.CCB = note_values[note>>4][note&0x0F];
		PORTC.OUTSET = PIN0_bm;
		if(auto_pause) schedule_task(dur-NOTE_SWITCH_DUR_MS, brief_pause, NULL);			
	}
	schedule_task(dur, switch_sound, NULL);
}

void brief_pause()
{
	PORTC.OUTCLR = PIN0_bm;
}

void stop_sound()
{
	//printf("P:%lu\r\n",get_time());	
	TCC0.CTRLB = TC_WGMODE_FRQ_gc;
	PORTC.OUTCLR = PIN0_bm;
	PORTC.PIN0CTRL = 0;
	PORTC.PIN1CTRL = 0;
}

#endif	

void speaker_init()
{
	#ifdef AUDIO_DROPLET
	PORTC.DIRSET =  PIN0_bm | PIN1_bm;
	TCC0.CTRLA = TC_CLKSEL_DIV64_gc;
	TCC0.CTRLB = TC_WGMODE_FRQ_gc;
	PORTC.PIN1CTRL = PORT_INVEN_bm;
	PORTC.PIN0CTRL = PORT_INVEN_bm;

	song_playing = 0;
	#else
	printf_P(PSTR("ERROR: speaker_init called, but this Droplet doesn't have a speaker.\r\n"));
	#endif
}

void play_song(uint8_t* notes, uint16_t* durs, uint8_t song_len, uint8_t pauses)
{
	#ifdef AUDIO_DROPLET
	if(song_playing) return; //do nothing
	auto_pause = pauses;
	song_playing = 1;
	curr_song_idx=1;
	curr_song_len=song_len;
	for(uint8_t i=0;i<song_len;i++)
	{
		local_durs[i]=durs[i];
		local_notes[i]=notes[i];
	}
	start_sound(notes[0]);
	if(auto_pause) schedule_task(durs[0]-NOTE_SWITCH_DUR_MS, brief_pause, NULL);
	schedule_task(durs[0], switch_sound, NULL);
	#else
	printf_P(PSTR("ERROR: play_song called, but this Droplet doesn't have a speaker.\r\n"));
	#endif
}

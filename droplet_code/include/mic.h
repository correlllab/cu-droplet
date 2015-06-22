#ifndef MIC_H
#define MIC_H

#ifdef AUDIO_DROPLET

#include <avr/io.h>
#include <avr/interrupt.h>
#include "scheduler.h"
#include "delay_x.h"

void mic_init();
int16_t get_mic_reading();
void mic_recording(uint16_t length, uint16_t sample_rate);

#endif

#endif
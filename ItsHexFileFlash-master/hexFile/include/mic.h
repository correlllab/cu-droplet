#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "scheduler.h"
#include "delay_x.h"

static const char FORMATTED_PRINT_STRING[] PROGMEM = "%4d, ";

void mic_init();
int16_t get_mic_reading();

void mic_recording(uint16_t* recording, uint16_t recording_len, uint16_t sample_rate);
void unpackMicRecording(uint16_t* unpacked, uint16_t unpacked_len, uint16_t* packed, uint16_t packed_len);
void print_recording(uint16_t* recording, uint16_t array_len);

#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "scheduler.h"
#include "delay_x.h"

static const char FORMATTED_PRINT_STRING[] PROGMEM = "%4d, ";

void mic_init();
int16_t get_mic_reading();
void mic_recording(uint16_t length, uint16_t sample_rate);

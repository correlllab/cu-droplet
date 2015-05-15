#ifndef MIC_H
#define MIC_H

#define OFFSET (13<<4)

#include <avr/io.h>
#include <avr/interrupt.h>
#include "scheduler.h"

void mic_init();
int16_t get_mic_reading();
void mic_recording(uint16_t length);

#endif
#pragma once

#include "droplet_base.h"

void micInit(void);

#ifdef AUDIO_DROPLET
#include "scheduler.h"
#include "delay_x.h"

void enableMicInterrupt(void);
void disableMicInterrupt(void);

void micRecording(uint16_t* recording, uint16_t recording_len, uint16_t sample_rate);
void unpackMicRecording(uint16_t* unpacked, uint16_t unpacked_len, uint16_t* packed, uint16_t packed_len);
void printRecording(uint16_t* recording, uint16_t array_len);


#endif
#ifndef MIC_H
#define MIC_H

#include <stdio.h> //Needed for 'FILE' type
#include <avr/io.h>
#include <avr/interrupt.h>
#include "scheduler.h"

void mic_init();
void process_mic_data();

#define MIC_DATA_WINDOW_SIZE 8

#endif
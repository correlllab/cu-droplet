#ifndef random_h
#define random_h

#include <avr/io.h>

uint8_t r_position;
uint8_t r_round;

void random_init();

uint8_t rand_byte();


#endif
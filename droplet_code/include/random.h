#ifndef random_h
#define random_h
#include <avr/io.h>

void random_init();

uint8_t rand_byte();
uint16_t rand_short();
uint32_t rand_quad();

#endif
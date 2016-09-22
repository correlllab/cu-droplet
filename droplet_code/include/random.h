#pragma once

#include <avr/io.h>

uint8_t r_round;
uint8_t r_position;

void random_init();

uint8_t rand_byte();

uint16_t rand_short();
uint32_t rand_quad();
float rand_real();
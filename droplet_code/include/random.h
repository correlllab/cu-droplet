#pragma once

#include <avr/io.h>
#include <math.h>

void random_init();

uint8_t rand_byte();

uint16_t rand_short();
uint32_t rand_quad();
float rand_real();
float rand_norm(float mean, float stdDev);
#pragma once

#include <avr/io.h>
#include <math.h>

uint8_t r_round;
uint8_t r_position;

uint8_t randNormHasSaved;
float randNormSavedValue;

void random_init();

uint8_t rand_byte();

uint16_t rand_short();
uint32_t rand_quad();
float rand_real();
float rand_norm(float mean, float stdDev);
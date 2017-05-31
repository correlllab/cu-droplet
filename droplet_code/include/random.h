#pragma once

#include "droplet_base.h"
#include <math.h>

void random_init(void);

uint8_t rand_byte(void);

uint16_t rand_short(void);
uint32_t rand_quad(void);
float rand_real(void);
float rand_norm(float mean, float stdDev);
#pragma once

#include "droplet_base.h"

void randomInit(void);

uint8_t randByte(void);

uint16_t randShort(void);
uint32_t randQuad(void);
float randReal(void);
float randNorm(float mean, float stdDev);
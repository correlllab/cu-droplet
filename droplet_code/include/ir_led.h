/** \file *********************************************************************
 * Low level sensing functions using IR channels.
 * Note that there is no IR communication code in this file.
 *****************************************************************************/
#pragma once

#include "droplet_base.h"
#include "i2c.h"

#define ALL_EMITTERS_CARWAV_bm (PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)

#define IR_POWER_ADDR_A 0x2C
#define IR_POWER_ADDR_B 0x2E

uint16_t curr_ir_power;

void irLedInit(void);
void setAllirPowers(uint16_t power);
void irLedOn(uint8_t direction);
void irLedOff(uint8_t direction);
inline uint16_t getAllirPowers(void){ return curr_ir_power; }

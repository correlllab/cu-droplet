/** \file *********************************************************************
 * Low level sensing functions using IR channels.
 * Note that there is no IR communication code in this file.
 *****************************************************************************/
#pragma once

#include <avr/io.h>
#include "scheduler.h"
#include "i2c.h"

static const char TWI_WAITING_STR[] PROGMEM = "\tWaiting for TWI...\r\n";
static const char TWI_TIMEOUT_STR[] PROGMEM = "\tTWI timeout when setting IR Powers ";

extern USART_t* channel[];

#define ALL_EMITTERS_CARWAV_bm (PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)

#define IR_POWER_ADDR_A 0x2C
#define IR_POWER_ADDR_B 0x2E

uint16_t curr_ir_power;

void ir_led_init();
void set_all_ir_powers(uint16_t power);
void ir_led_on(uint8_t direction);
void ir_led_off(uint8_t direction);
inline uint16_t get_all_ir_powers(){ return curr_ir_power; }
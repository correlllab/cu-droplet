#pragma once
#include "droplet_base.h"
#include "ir_comm.h"
#include "ir_led.h"
#include "delay_x.h"

extern USART_t* channel[6];
extern uint8_t carrier_wave_pins[6];

void sendIRBurst(uint8_t dir, uint8_t addr, uint8_t cmd);
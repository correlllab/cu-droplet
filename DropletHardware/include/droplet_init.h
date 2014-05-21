#ifndef DROPLET_INIT_H
#define DROPLET_INIT_H

#include <avr/io.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "scheduler.h"
#include "pc_com.h"
#include "RGB_LED.h"
#include "RGB_sensor.h"
#include "power.h"
#include "random.h"
#include "ecc.h"
#include "IRcom.h"
#include "IR_Sensor.h"
#include "i2c.h"
#include "motor.h"
#include "Range_Algorithms.h"
#include "serial_handler.h"

uint16_t droplet_ID;
uint8_t got_rnb_cmd_flag;

void init_all_systems();
void calculate_id_number();
void enable_interrupts();
void startup_light_sequence();
void droplet_reboot();

inline uint16_t get_droplet_id(){ return droplet_ID; }

#endif

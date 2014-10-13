#ifndef DROPLET_INIT_H
#define DROPLET_INIT_H

#include <avr/io.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define DIR0		0x01
#define DIR1		0x02
#define DIR2		0x04
#define DIR3		0x08
#define DIR4		0x10
#define DIR5		0x20
#define ALL_DIRS	0x3F

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

void init_all_systems();
void calculate_id_number();
void enable_interrupts();
void startup_light_sequence();
void droplet_reboot();

inline uint16_t get_droplet_id(){ return droplet_ID; }

#endif

#ifndef DROPLET_INIT_H
#define DROPLET_INIT_H

#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "pc_com.h"
#include "motor.h"
#include "power.h"
#include "RGB_LED.h"
#include "RGB_sensor.h"
#include "i2c.h"
#include "IRcom.h"
#include "IR_Sensor.h"
#include "ecc.h"
#include "random.h"
#include "scheduler.h"
#include "sp_driver.h"
#include "Range_Algorithms.h"
#include "serial_handler.h"
//#include "boot.h"
//#include "commands.h"

uint16_t droplet_ID;
uint8_t got_rnb_cmd_flag;

void Config32MHzClock(void);

void init_all_systems();

uint16_t get_16bit_time();

void set_current_time(uint16_t count);

void delay_ms(uint16_t ms);

uint16_t get_droplet_id();//DEPRECATED

void calculate_id_number();

#endif

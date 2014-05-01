#ifndef DROPLET_INIT_H
#define DROPLET_INIT_H

#include <avr/io.h>
#include <util/delay.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "droplet_time.h"
#include "pc_com.h"
#include "scheduler.h"
#include "RGB_LED.h"
#include "power.h"
#include "RGB_sensor.h"
#include "i2c.h"
#include "IRcom.h"
#include "IR_Sensor.h"
#include "ecc.h"
#include "random.h"
#include "serial_handler.h"
#include "motor.h"
#include "Range_Algorithms.h"




uint16_t droplet_ID;
uint8_t got_rnb_cmd_flag;

void init_all_systems();
void calculate_id_number();
void droplet_reboot();
inline uint16_t get_droplet_id(){ return droplet_ID; }

#endif

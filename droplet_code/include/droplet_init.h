/** \file *********************************************************************
 * \brief Droplet initialization routines and global macros are defin3d here.
 * 
 * It is highly recommended to include ONLY this header file in any user level
 * droplet rather than including header files for each of the subsystems
 * independently.
 * 
 *
 *****************************************************************************/
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
#include "pc_comm.h"
#include "rgb_led.h"
#include "rgb_sensor.h"
#include "power.h"
#include "random.h"
#include "ecc.h"
#include "ir_comm.h"
#include "ir_sensor.h"
#include "i2c.h"
#include "motor.h"
#include "range_algs.h"
#include "serial_handler.h"

uint16_t droplet_ID;

typedef struct ir_msg_struct
{
	uint32_t arrival_time;	// Time of message receipt.	
	uint16_t sender_ID;		// ID of sending robot.	
	char* msg;				// Transmit / receive buffer	
	uint8_t dir_received;
	uint8_t length;		// Message length.
} ir_msg;

extern void init();
extern void loop();
extern void handle_msg(ir_msg* msg_struct);

/**
 * \brief Returns this Droplet's unique 16-bit identifier.
 */
inline uint16_t get_droplet_id(){ return droplet_ID; }

/**
 * \brief Initializes all the subsystems for this Droplet. This function MUST be called
 * by the user before using any other functions in the API.
 */ 
void init_all_systems();

/*
 * This function loops through all messages this robot has received since the last call
 * to check messages.
 * For each message, it populates an ir_msg struct and calls handle_msg with it.
 */
void check_messages();

/**
 * \brief Resets the Droplet's program counter and clears all low-level system buffers.
 */
void droplet_reboot();


void calculate_id_number();
void enable_interrupts();
void startup_light_sequence();


#endif

/** \file *********************************************************************
 * \brief Droplet initialization routines and global macros are defin3d here.
 * 
 * It is highly recommended to include ONLY this header file in any user level
 * droplet rather than including header files for each of the subsystems
 * independently.
 * 
 *
 *****************************************************************************/
#pragma once

//#define INIT_DEBUG_MODE

#ifdef INIT_DEBUG_MODE
#define INIT_DEBUG_PRINT(x) printf(x)
#else
#define INIT_DEBUG_PRINT(x)
#endif

#include <avr/io.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#define DIR0		((uint8_t)0x01)
#define DIR1		((uint8_t)0x02)
#define DIR2		((uint8_t)0x04)
#define DIR3		((uint8_t)0x08)
#define DIR4		((uint8_t)0x10)
#define DIR5		((uint8_t)0x20)
#define ALL_DIRS	((uint8_t)0x3F)

#define DIR_NE		DIR0
#define DIR_E		DIR1
#define DIR_SE		DIR2
#define DIR_SW		DIR3
#define DIR_W		DIR4
#define DIR_NW		DIR5

#include "scheduler.h"
#include "pc_comm.h"
#include "rgb_led.h"
#include "rgb_sensor.h"
#include "power.h"
#include "ecc.h"
#include "i2c.h"
#include "range_algs.h"
#include "rgb_sensor.h"
#include "ir_led.h"
#include "ir_sensor.h"
#include "ir_comm.h"
#include "speaker.h"
#include "mic.h"
#include "motor.h"
#include "random.h"
#include "firefly_sync.h"

#include "serial_handler.h"

#include <avr/pgmspace.h>

uint16_t droplet_ID;

typedef uint16_t bot_id_t;

typedef struct ir_msg_struct
{
	uint32_t arrival_time;	// Time of message receipt.	
	float range;
	float bearing;
	float heading;
	uint16_t sender_ID;		// ID of sending robot.	
	char* msg;				// The message.
	uint8_t dir_received;	// Which side was this message received on?
	uint8_t length;			// Message length.
	uint8_t wasTargeted;
} ir_msg;

extern void init();
extern void loop();
extern void handle_msg(ir_msg* msg_struct);
extern uint8_t user_handle_command(char* command_word, char* command_args);
extern void	user_leg_status_interrupt();

/**
 * \brief Returns this Droplet's unique 16-bit identifier. 0 will never be an identifier.
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

uint8_t get_droplet_ord(uint16_t id);
extern const uint16_t OrderedBotIDs[121];
static uint16_t inline get_id_from_ord(uint8_t ord){
	return pgm_read_word(&OrderedBotIDs[ord]);
}
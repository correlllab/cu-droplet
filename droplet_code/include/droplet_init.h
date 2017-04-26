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

#include "droplet_base.h"
#include "scheduler.h"
#include "pc_comm.h"
#include "rgb_led.h"
#include "rgb_sensor.h"
#include "power.h"
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
#include "matrix_utils.h"
#include "localization.h"

#define CMD_DROPLET_ID	0x8F6D

typedef struct ir_msg_struct
{
	uint32_t arrival_time;	// Time of message receipt.
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

void startup_light_sequence();
uint8_t get_droplet_ord(id_t id);

extern const id_t OrderedBotIDs[121];

inline id_t get_id_from_ord(uint8_t ord){
	return pgm_read_word(&OrderedBotIDs[ord]);
}
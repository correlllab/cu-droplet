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
#include "nvm.h"
#include "speaker.h"
#include "mic.h"
#include "motor.h"
#include "random.h"
#include "firefly_sync.h"
#include "serial_handler.h"
#include "matrix_utils.h"
#include "localization.h"

#define CMD_DROPLET_ID	0x8F6D

uint8_t reprogramming;

void send_hex(void);
void send_initial(void);

char dataHEX[64];
char initial_msg[20];
uint8_t number_of_hex;

typedef struct ir_msg_struct
{
	uint32_t arrivalTime;	// Time of message receipt.
	id_t senderID;		// ID of sending robot.
	char* msg;				// The message.
	uint8_t length;			// Message length.
} irMsg;
void handle_reprogramming(irMsg *msg_struct_hex);// __attribute__ ((section (".BOOT")));
void handle_serial_comm(irMsg *msg_struct);
//extern void		init() __attribute__ ((section (".USERCODE_TEXT")));
extern void		init(void) __attribute__ ((section (".USERCODE")));
extern void loop(void);
extern void handleMsg(irMsg* msg_struct);
extern uint8_t userHandleCommand(char* commandWord, char* commandArgs);

void startupLightSequence(void);
uint8_t getDropletOrd(id_t id);

extern const id_t OrderedBotIDs[121];

inline id_t getIdFromOrd(uint8_t ord){
	return pgm_read_word(&OrderedBotIDs[ord]);
}
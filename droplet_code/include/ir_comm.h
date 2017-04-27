/** \file *********************************************************************
 * \brief Droplet infrared communication subsystem functions are defined here.
 *
 *****************************************************************************/
#pragma once

#include <avr/io.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "scheduler.h"
#include "pc_comm.h"
#include "ir_led.h"
#include "ir_sensor.h"
#include "firefly_sync.h"

// FYI, the XMEGA128A3U has:
//		128 KB flash (program memory)
//		8 KB flash (bootloader memory)
//		2 KB EEPROM	(permanent variables)
//		8 KB SRAM (temporary variables)

#define MAX_USER_FACING_MESSAGES 6

#define KEY_POWER		((uint16_t)0x40BF)
#define KEY_CH_UP		((uint16_t)0x48B7)
#define KEY_CH_DOWN		((uint16_t)0xC837)
#define KEY_VOL_UP		((uint16_t)0xE01F)
#define KEY_VOL_DOWN	((uint16_t)0xD02F)
#define KEY_SOURCE		((uint16_t)0x807F)
#define KEY_ENTER		((uint16_t)0x1AE5)
#define KEY_UP			((uint16_t)0x06F9)
#define KEY_DOWN		((uint16_t)0x8679)
#define KEY_LEFT		((uint16_t)0xA659)
#define KEY_RIGHT		((uint16_t)0x46B9)

#define IR_BUFFER_SIZE			40u //bytes
#define IR_UPKEEP_FREQUENCY		16 //Hz
#define IR_MSG_TIMEOUT			16 //ms

#define IR_STATUS_BUSY_bm				0x01	// 0000 0001				
#define IR_STATUS_COMPLETE_bm			0x02	// 0000 0010
#define IR_STATUS_ERROR_bm				0x04	// 0000 0100
#define IR_STATUS_TARGETED_bm			0x10	// 0001 0000
#define IR_STATUS_TRANSMITTING_bm		0x20    // 0010 0000
#define IR_STATUS_TIMED_bm				0x40	// 0100 0000
#define IR_STATUS_COMMAND_bm			0x80	// 1000 0000
#define IR_STATUS_UNAVAILABLE_bm		0x03	// Complete or Busy

#define IR_STATUS_CRC_BITS_bm			0xC0	// SPECIAL or COMMAND

#define DATA_LEN_VAL_bm		0x3F
#define DATA_LEN_SPCL_bm	0x40
#define DATA_LEN_CMD_bm		0x80

#define DATA_LEN_STATUS_BITS_bm			0xC0

#define HEADER_POS_SENDER_ID_LOW 0
#define HEADER_POS_SENDER_ID_HIGH 1
#define HEADER_POS_CRC_LOW 2
#define HEADER_POS_CRC_HIGH 3
#define HEADER_POS_MSG_LENGTH 4
#define HEADER_POS_TARGET_ID_LOW 5
#define HEADER_POS_TARGET_ID_HIGH 6
#define HEADER_POS_SOURCE_DIR 7

#define HEADER_LEN 8U

#define MAX_WAIT_FOR_IR_TIME (5*(IR_BUFFER_SIZE+HEADER_LEN))

#ifdef AUDIO_DROPLET
	extern ADC_CH_t* ir_sense_channels[6];
#endif

extern USART_t* channel[6];


volatile struct
{	
	volatile uint32_t last_byte;			// TX time or RX time of last received byte	
	volatile uint16_t data_crc;
	volatile id_t sender_ID;
	volatile id_t target_ID;
	volatile uint16_t curr_pos;				// Current position in buffer
	volatile uint16_t calc_crc;
	volatile char buf[IR_BUFFER_SIZE];		// Transmit / receive buffer		
	volatile uint8_t  data_length;	
	volatile int8_t inc_dir;
	volatile uint8_t status;		// Transmit:
} ir_rxtx[6];

#define INC_DIR_KEY 0b11111000

volatile struct
{
	volatile uint32_t	arrival_time;
	volatile id_t		sender_ID;
	volatile char		msg[IR_BUFFER_SIZE];		
	volatile uint8_t	arrival_dir;
	volatile uint8_t	msg_length;
	volatile uint8_t	wasTargeted;
} msg_node[MAX_USER_FACING_MESSAGES];

volatile uint8_t hp_ir_block_bm;			//can only be set by other high priority ir things!
volatile uint8_t num_waiting_msgs;
volatile uint8_t user_facing_messages_ovf;

volatile uint32_t	cmd_arrival_time;
volatile id_t		cmd_sender_id;
volatile uint8_t	cmd_arrival_dir;
volatile uint8_t	cmd_sender_dir;

void ir_comm_init();

void handle_cmd_wrapper();

uint8_t ir_targeted_cmd(uint8_t dirs, char *data, uint8_t data_length, id_t target);
uint8_t ir_cmd(uint8_t dirs, char *data, uint8_t data_length);
uint8_t ir_targeted_send(uint8_t dirs, char *data, uint8_t data_length, id_t target);
uint8_t ir_send(uint8_t dirs, char *data, uint8_t data_length);
uint8_t hp_ir_cmd(uint8_t dirs, char *data, uint8_t data_length);
uint8_t hp_ir_targeted_cmd(uint8_t dirs, char *data, uint8_t data_length, id_t target);
void waitForTransmission(uint8_t dirs);

/*
 * dirs_mask specifies the directions a function caller is interested in.
 *
 * This function returns:
 * 3: If any of the specified directions are currently blocked by a high-priority IR task.
 * 2: If the droplet is currently transmitting in any of the specified directions.
 * 1: If the droplet is currently receiving a message in any of the specified directions.
 * 0: Otherwise (all directions clear).
 * 
 * Using (ir_is_busy(dirs_mask)>1) is equivalent to the old (!ir_is_available(dirs_mask))
 */
uint8_t ir_is_busy(uint8_t dirs_mask);
//uint8_t wait_for_ir(uint8_t dirs);
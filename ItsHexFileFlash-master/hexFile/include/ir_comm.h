/** \file *********************************************************************
 * \brief Droplet infrared communication subsystem functions are defined here.
 *
 *****************************************************************************/
#pragma once

#include <avr/io.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "droplet_init.h"
#include "scheduler.h"
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

#define IR_BUFFER_SIZE			40 //bytes
#define IR_UPKEEP_FREQUENCY		16 //Hz
#define IR_MSG_TIMEOUT			20 //ms

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

#define HEADER_LEN 8

#define MAX_WAIT_FOR_IR_TIME (5*(IR_BUFFER_SIZE+HEADER_LEN))

#ifdef AUDIO_DROPLET
	extern ADC_CH_t* ir_sense_channels[6];
#endif

volatile struct
{	
	volatile uint32_t last_byte;			// TX time or RX time of last received byte	
	#ifdef AUDIO_DROPLET
	volatile int16_t ir_meas[IR_BUFFER_SIZE+HEADER_LEN];		
	#endif
	volatile uint16_t data_crc;
	volatile uint16_t sender_ID;
	volatile uint16_t target_ID;
	volatile uint16_t curr_pos;				// Current position in buffer
	volatile uint16_t calc_crc;
	volatile char   buf[IR_BUFFER_SIZE];	// Transmit / receive buffer		
	volatile uint8_t  data_length;	
	volatile int8_t inc_dir;
	volatile uint8_t status;		// Transmit:
} ir_rxtx[6];

#define INC_DIR_KEY 0b11111000

volatile struct
{
	volatile uint32_t	arrival_time;
	volatile float		range;
	volatile float		bearing;
	volatile float		heading;
	volatile uint16_t	sender_ID;
	volatile char		msg[IR_BUFFER_SIZE];		
	volatile uint8_t	arrival_dir;
	volatile uint8_t	msg_length;
	volatile uint8_t	wasTargeted;
} msg_node[MAX_USER_FACING_MESSAGES];

volatile uint8_t hp_ir_block_bm;			//can only be set by other high priority ir things!
volatile uint8_t num_waiting_msgs;
volatile uint8_t user_facing_messages_ovf;

volatile uint32_t	cmd_arrival_time;
volatile uint16_t	cmd_sender_id;
volatile uint8_t	cmd_arrival_dir;
volatile uint8_t	cmd_sender_dir;

volatile uint8_t processing_cmd;
volatile uint8_t processing_ffsync;

void clear_ir_buffer(uint8_t dir);

void ir_comm_init();

void handle_cmd_wrapper();
void perform_ir_upkeep();
uint8_t ir_targeted_cmd(uint8_t dirs, char *data, uint8_t data_length, uint16_t target);
uint8_t ir_cmd(uint8_t dirs, char *data, uint8_t data_length);
uint8_t ir_targeted_send(uint8_t dirs, char *data, uint8_t data_length, uint16_t target);
uint8_t ir_send(uint8_t dirs, char *data, uint8_t data_length);
void waitForTransmission(uint8_t dirs);
uint8_t hp_ir_cmd(uint8_t dirs, char *data, uint8_t data_length);
uint8_t hp_ir_targeted_cmd(uint8_t dirs, char *data, uint8_t data_length, uint16_t target);

void handle_rx_length_byte(uint8_t in_byte, uint8_t dir);
uint8_t handle_tx_length_byte(uint8_t dir);
void ir_receive(uint8_t dir); //Called by Interrupt Handler Only
void received_ir_cmd(uint8_t dir);
void received_rnb_r(uint8_t delay, uint16_t senderID, uint32_t last_byte);
void received_ir_sync(uint8_t delay, uint16_t senderID);
void ir_transmit(uint8_t dir);
void ir_remote_send(uint8_t dir, uint16_t data);
void ir_transmit_complete(uint8_t dir);
void ir_reset_rx(uint8_t dir);
uint8_t ir_is_available(uint8_t dirs_mask);
//uint8_t wait_for_ir(uint8_t dirs);

static inline float comm_inverse_amplitude_model(float comm_amp)
{
	return 17.25+(6292.0/powf(comm_amp+12,2));
}
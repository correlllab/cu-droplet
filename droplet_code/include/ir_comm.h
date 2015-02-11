/** \file *********************************************************************
 * \brief Droplet infrared communication subsystem functions are defined here.
 *
 *****************************************************************************/

#ifndef IR_COMM_H
#define IR_COMM_H

#include <avr/io.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include "droplet_init.h"
#include "scheduler.h"

// FYI, the XMEGA128A3U has:
//		128 KB flash (program memory)
//		8 KB flash (bootloader memory)
//		2 KB EEPROM	(permanent variables)
//		8 KB SRAM (temporary variables)

#define MAX_USER_FACING_MESSAGES 6

#define IR_BUFFER_SIZE			16 //bytes
#define IR_UPKEEP_FREQUENCY		16 //Hz
#define IR_MSG_TIMEOUT			10 //ms

#define IR_STATUS_BUSY_bm				0x01	// 0000 0001				
#define IR_STATUS_COMPLETE_bm			0x02	// 0000 0010
#define IR_STATUS_ERROR_bm				0x04	// 0000 0100
#define IR_STATUS_COMMAND_bm			0x08	// 0000 1000
#define IR_STATUS_TARGETED_bm			0x10	// 0001 0000
#define IR_STATUS_UNAVAILABLE_bm		0x03	// Complete or Busy

#define DATA_LEN_VAL_bm		0x7F
#define DATA_LEN_CMD_bm		0x80

#define HEADER_POS_SENDER_ID_LOW 0
#define HEADER_POS_SENDER_ID_HIGH 1
#define HEADER_POS_MSG_LENGTH 2
#define HEADER_POS_CRC_LOW 3
#define HEADER_POS_CRC_HIGH 4
#define HEADER_POS_TARGET_ID_LOW 5
#define HEADER_POS_TARGET_ID_HIGH 6
#define HEADER_LEN 7

extern USART_t* channel[];

volatile struct
{	
	uint32_t last_byte;			// TX time or RX time of last received byte	
	char   buf[IR_BUFFER_SIZE];	// Transmit / receive buffer	
	uint16_t data_crc;
	uint16_t sender_ID;
	uint16_t target_ID;
	uint16_t curr_pos;				// Current position in buffer
	uint16_t calc_crc;
	uint8_t  data_length;
	volatile uint8_t status;		// Transmit:
} ir_rxtx[6];

volatile struct
{
	uint32_t	arrival_time;
	uint16_t	sender_ID;
	char		msg[IR_BUFFER_SIZE];		
	uint8_t		arrival_dir;
	uint8_t		msg_length;
} msg_node[MAX_USER_FACING_MESSAGES];

volatile uint8_t num_waiting_msgs;
volatile uint8_t user_facing_messages_ovf;

volatile uint32_t	cmd_arrival_time;
volatile uint16_t	cmd_sender_id;

void ir_comm_init();

void perform_ir_upkeep();
void ir_targeted_cmd(uint8_t dirs, char *data, uint16_t data_length, uint16_t target);
void ir_cmd(uint8_t dirs, char *data, uint16_t data_length);
void ir_targeted_send(uint8_t dirs, char *data, uint16_t data_length, uint16_t target);
void ir_send(uint8_t dirs, char *data, uint8_t data_length);
void ir_send_imp(uint8_t dirs, char* data, uint8_t data_length);
void ir_receive(uint8_t dir); //Called by Interrupt Handler Only
void ir_transmit(uint8_t dir);
void ir_transmit_complete(uint8_t dir);
void ir_reset_rx(uint8_t dir);
void wait_for_ir(uint8_t dirs);

void print_received_message(void* dir_star); //DEBUG -> Remove later

#endif

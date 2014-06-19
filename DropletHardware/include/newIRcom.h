#ifndef IR_COM_H
#define IR_COM_H

// FYI, the XMEGA128A3U has:
//		128 KB flash (program memory)
//		8 KB flash (bootloader memory)
//		2 KB EEPROM	(permanent variables)
//		8 KB SRAM (temporary variables)

#include <avr/io.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include "droplet_time.h"

#define IR_BUFFER_SIZE			64

#define IR_STATUS_BUSY_bm				0x01	// 0000 0001				
#define IR_STATUS_COMPLETE_bm			0x02	// 0000 0010
#define IR_STATUS_ERROR_bm				0x04	// 0000 0100
#define IR_STATUS_COMMAND_bm			0x08	// 0000 1000
#define IR_STATUS_TARGETED_bm			0x0F	// 0001 0000
#define IR_STATUS_ERROR_bm				0x08	// 0000 1000

#define HEADER_POS_MSG_LENGTH 0
#define HEADER_POS_CRC_LOW 1
#define HEADER_POS_CRC_HIGH 2
#define HEADER_POS_SENDER_ID_LOW 3
#define HEADER_POS_SENDER_ID_HIGH 4
#define HEADER_POS_TARGET_ID_LOW 5
#define HEADER_POS_TARGET_ID_HIGH 6
#define HEADER_LEN 7

extern USART_t* channel[] = {	
								&USARTC0,  //   -- Channel 0
								&USARTC1,  //   -- Channel 1
								&USARTD0,  //   -- Channel 2
								&USARTE0,  //   -- Channel 3
								&USARTE1,  //   -- Channel 4
								&USARTF0   //   -- Channel 5
							};// channel[] is declared here, but defined in IRcom.c

struct
{	
	uint8_t* buf;					// Transmit / receive buffer
	uint16_t data_crc;
	uint16_t sender_ID;
	uint16_t target_ID;
	uint16_t curr_pos;				// Current position in buffer
	uint16_t initial_use_time;		// TX time or RX time of first byte
	uint8_t  data_length;
	volatile uint8_t status;		// Transmit:
} ir_rxtx[6];

/* GENERAL IR RELATED ROUTINES */

void ir_com_init(uint16_t buffer_size);

void set_ir_power(uint8_t dir, uint16_t power);
// Sets intensity of IR transmitters
// power = 0 turns transmitter off
// power = 256 turns transmitter on maximum power
// Note: this function is blocking and will take approx 300us to complete

uint8_t check_ir_busy(uint8_t *busy_dir);
// returns 0 if not busy
// returns 1 if busy transmitting
// returns 2 if busy receiving
// *busy_dir is populated with the first direction that is found to be busy


/* IR RECEIVE ROUTINES */

void ir_reset_rx(uint8_t dir);

void print_RX_buffer(uint8_t dir);			// typically for debugging

void print_RX_buffer_status(uint8_t dir);	// typically for debugging

void erase_RX_buffer(uint8_t dir);			// typically for debugging


/* IR TRANSMIT ROUTINES */

uint8_t OK_to_send();
// USER CODE CALLS THIS BEFORE ir_send()
// this routine examines the IR RX and TX channels for busy status before transmitting
// the RX channels need to be clear before transmitting, since transmitting will shut these down temporarily
// returns 0 [BAD] if TX is busy, inspect this problem before proceeding with ir_send()
// returns 1 [GOOD] if all RX channels were ready with no work required, nearby IR likely quiet
// returns 2 [GOOD] if all RX channels are now clear, nearby IR may be noisy, (some (possibly all) channels had to be reset)
// returns 3 [GOOD] if all RX channels are now clear, nearby IR may be VERY noisy, (some (possibly all) channels had to be reset)
uint8_t ir_targeted_cmd(uint8_t dir, uint8_t *data, uint16_t data_length, uint16_t target); //Just sets the command bit and then calls ir_send.
uint8_t ir_targeted_broadcasted_cmd(uint8_t *data, uint16_t data_length, uint16_t target);
uint8_t ir_send_cmd(uint8_t dir, uint8_t *data, uint16_t data_length); //Just sets the command bit and then calls ir_send.
uint8_t ir_broadcast_cmd(uint8_t *data, uint16_t data_length);
uint8_t ir_targeted_send(uint8_t dir, uint8_t *data, uint16_t data_length, uint16_t target);
uint8_t ir_targeted_broadcast(uint8_t *data, uint16_t data_length, uint16_t target);
uint8_t ir_send(uint8_t dir, uint8_t *data, uint16_t data_length);	// 'kind of' BLOCKING (MED_LVL interrupt heavy routine)
uint8_t ir_broadcast(uint8_t *data, uint16_t data_length);
// possible return values:
// Error 0: no error
// Error 1: transmitter busy
// Error 2: receiver busy
// Error 3: zero-length transmission
// Error 4: message too big for transmit buffer

void ir_reset_tx(uint8_t dir);


/* GLOBAL RECEIVE BUFFER RELATED ROUTINES */

uint8_t check_for_new_messages();
// USER CODE CALLS THIS
// This routine populates the global rx buffer with new messages, if applicable

uint8_t check_buffers_for_packets();
// CALLED BY check_for_new_messages(), DO NOT CALL


#endif

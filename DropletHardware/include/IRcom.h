#ifndef IR_COM_H
#define IR_COM_H

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

#define IR_BUFFER_SIZE			63

#define IR_STATUS_BUSY_bm				0x01	// 0000 0001				
#define IR_STATUS_COMPLETE_bm			0x02	// 0000 0010
#define IR_STATUS_ERROR_bm				0x04	// 0000 0100
#define IR_STATUS_COMMAND_bm			0x08	// 0000 1000
#define IR_STATUS_TARGETED_bm			0x10	// 0001 0000


#define DATA_LEN_VAL_bm		0x3F
#define DATA_LEN_TGT_bm		0x40
#define DATA_LEN_CMD_bm		0x80

#define IR_ALL_DIRS 0x3F

#define HEADER_POS_MSG_LENGTH 0
#define HEADER_POS_CRC_LOW 1
#define HEADER_POS_CRC_HIGH 2
#define HEADER_POS_SENDER_ID_LOW 3
#define HEADER_POS_SENDER_ID_HIGH 4
#define HEADER_POS_TARGET_ID_LOW 5
#define HEADER_POS_TARGET_ID_HIGH 6
#define HEADER_LEN 7

struct
{	
	uint32_t initial_use_time;		// TX time or RX time of first byte	
	char* buf;					// Transmit / receive buffer	
	uint16_t data_crc;
	uint16_t sender_ID;
	uint16_t target_ID;
	uint16_t curr_pos;				// Current position in buffer
	uint8_t  data_length;
	volatile uint8_t status;		// Transmit:
} ir_rxtx[6];

/* GENERAL IR RELATED ROUTINES */

void ir_com_init();

/* 
 * Sets intensity of IR transmitters
 * power = 0 turns transmitter off
 * power = 256 turns transmitter on maximum power
 * Note: this function is blocking and will take approx 300us to complete
 */
void set_ir_power(uint8_t dir, uint16_t power);

uint8_t ir_targeted_cmd(uint8_t dirs, char *data, uint16_t data_length, uint16_t target);
uint8_t ir_cmd(uint8_t dirs, char *data, uint16_t data_length);
uint8_t ir_targeted_send(uint8_t dirs, char *data, uint16_t data_length, uint16_t target);
uint8_t ir_send(uint8_t dirs, char *data, uint8_t data_length);
void ir_receive(uint8_t dir); //Called by Interrupt Handler Only
void ir_transmit(uint8_t dir);
void ir_transmit_complete(uint8_t dir);
void ir_reset_rx(uint8_t dir);
void wait_for_ir(uint8_t dirs);

void print_received_message(void* dir_star); //DEBUG -> Remove later


///* returns 0 if not busy
 //* returns 1 if busy transmitting
 //* returns 2 if busy receiving
 //* busy_dir is populated with the first direction that is found to be busy
 //*/
//uint8_t check_ir_busy(uint8_t *busy_dir);
//uint8_t OK_to_send();
//// USER CODE CALLS THIS BEFORE ir_send()
//// this routine examines the IR RX and TX channels for busy status before transmitting
//// the RX channels need to be clear before transmitting, since transmitting will shut these down temporarily
//// returns 0 [BAD] if TX is busy, inspect this problem before proceeding with ir_send()
//// returns 1 [GOOD] if all RX channels were ready with no work required, nearby IR likely quiet
//// returns 2 [GOOD] if all RX channels are now clear, nearby IR may be noisy, (some (possibly all) channels had to be reset)
//// returns 3 [GOOD] if all RX channels are now clear, nearby IR may be VERY noisy, (some (possibly all) channels had to be reset)
//uint8_t ir_targeted_cmd(uint8_t dir, uint8_t *data, uint16_t data_length, uint16_t target); //Just sets the command bit and then calls ir_send.
//uint8_t ir_targeted_broadcasted_cmd(uint8_t *data, uint16_t data_length, uint16_t target);
//uint8_t ir_send_cmd(uint8_t dir, uint8_t *data, uint16_t data_length); //Just sets the command bit and then calls ir_send.
//uint8_t ir_broadcast_cmd(uint8_t *data, uint16_t data_length);
//uint8_t ir_targeted_send(uint8_t dir, uint8_t *data, uint16_t data_length, uint16_t target);
//uint8_t ir_targeted_broadcast(uint8_t *data, uint16_t data_length, uint16_t target);
//uint8_t ir_send(uint8_t dir, uint8_t *data, uint16_t data_length);	// 'kind of' BLOCKING (MED_LVL interrupt heavy routine)
//uint8_t ir_broadcast(uint8_t *data, uint16_t data_length);
//// possible return values:
//// Error 0: no error
//// Error 1: transmitter busy
//// Error 2: receiver busy
//// Error 3: zero-length transmission
//// Error 4: message too big for transmit buffer
//
//void ir_reset_tx(uint8_t dir);
//
//
///* GLOBAL RECEIVE BUFFER RELATED ROUTINES */
//
//uint8_t check_for_new_messages();
//// USER CODE CALLS THIS
//// This routine populates the global rx buffer with new messages, if applicable
//
//uint8_t check_buffers_for_packets();
//// CALLED BY check_for_new_messages(), DO NOT CALL
//

#endif

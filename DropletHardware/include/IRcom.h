#ifndef IR_COM_H
#define IR_COM_H

#include <avr/io.h>
#include <util/crc16.h>

#define IR_BUFFER_SIZE			72		// Proper size for IR reprogramming
#define IR_SMALL_BUFFER_SIZE	25		// Smaller size for debugging
#define IR_MAX_BUFFER_SIZE		

//For COMMAND and PROG: 00: normal comm, 01: serial command, 10: programming, 11: time-sensitive.
#define IR_PACKET_DATA_gc			0x00FF
#define IR_PACKET_COMMAND_bm		0x0100	// command byte, and we should treat this message as a serial command.
#define IR_PACKET_HEADER_bm			0x0200	// byte is first of message
#define IR_PACKET_START_OR_END_bm	0x0400	// byte is last byte of message
#define IR_PACKET_TARGET_bm			0x0800	// byte is targeted

#define IR_TX_STATUS_BUSY_bm				0x01	// 0000 0001				
#define IR_TX_STATUS_SEND_HEADER_bm			0x02
#define IR_TX_STATUS_SEND_START_OR_END_bm	0x04
#define IR_TX_STATUS_COMMAND_bm				0x08	// 0000 1000
#define IR_TX_STATUS_BROADCAST_bm			0x10
#define IR_TX_STATUS_TARGETED_bm			0x20

#define IR_RX_STATUS_BUSY_bm		0x01	// 0000 0001	note: this flag should be mutually exclusive with IR_RX_STATUS_PACKET_DONE (TODO, enforce this constraint)
#define IR_RX_STATUS_PACKET_DONE_bm	0x02	// 0000 0010	note: there is only exactly ONE place in the code where this flag should be set!
#define IR_RX_STATUS_BYTE_DONE_bm	0x04	// 0000 0100
#define IR_RX_STATUS_COMMAND_bm		0x08  // Receiving a command		
//#define IR_RX_STATUS_RX_BYTE_bm		0x0008	// 0000 1000 wasn't referenced.
#define IR_RX_STATUS_ECCERR_bm		0x10	// 0001 0000		
#define IR_RX_STATUS_LENERR_bm		0x20	// 0010 0000
#define IR_RX_STATUS_TIMEOUT_bm		0x40	// 0100 0000	note: see 1*
#define IR_RX_STATUS_ERR_gc			0x70	// 0111 0000	note: this is NEVER assigned, only used for reading flags!
#define IR_RX_STATUS_TARGETED_bm	0x80	// 1000 0000

// notes:
// 1*	ir_receive is only called IF there is new data coming in, it will not be able to check the timeout condition if there is no new data
//		and if there is new data that is significantly time delayed, then it may be part of a new message (in which case it will be tagged
//		with a header), if its not tagged with a header, then it MAY be a continuation of the current message (in which case, it may be
//		okay to accept this data).  We should then tag the message with this flag, and be suspicious of its contents.	(TODO)


extern USART_t* channel[];			// channel[] is declared here, but defined in IRcom.c


/* DATA STRUCTURES */

struct
{
	// MEMORY MANAGEMENT:
	// with IR_BUFFER_SIZE = 72, each direction grabs 
	// 2*(72(buf)+2(size)+2(curr_pos)+2(length)+4(encoded)+1(curr_encoded_pos)+2(ir_status)) bytes of memory
	// = 2 * 84 = 168 bytes per direction
	// total memory allocated to 6 IR buffers: 1008 bytes ~ 1 KB (or 8 KB total SRAM available)
	// FYI, the XMEGA128A3U has:
	//		128 KB flash (program memory)
	//		8 KB flash (bootloader memory)
	//		2 KB EEPROM	(permanent variables)
	//		8 KB SRAM (temporary variables)
	
	uint8_t *buf;					// Transmit / receive buffer
	uint16_t size;					// Buffer size

	uint16_t curr_pos;				// Current position in buffer
	uint16_t data_len;				// Length of message to send

	uint32_t encoded;				// Low-order 24 bits contain Golay encoded byte currently being sent / received

	uint8_t curr_encoded_pos;		// Current byte in the 3 byte Golay encoded word being sent / received
	
	// ir_status was previously a user-defined variable called STATUS, but renamed to avoid confusion with true STATUS hardware registers (e.g. channel[dir]->STATUS)
	volatile uint8_t ir_status;			// Transmit:	[ PROGRAMMING, DEAD TIME, ZERO-LENGTH TRANSMISSION, BUFFER OVERFLOW, SENDING LENGTH, BYTE COMPLETE, PACKET COMPLETE, BUSY, COMMAND, NULL, NULL, NULL,  NULL, NULL, NULL,  NULL, NULL ]
									// Receive:		[ PROGRAMMING, TIMEOUT ERROR, LENGTH ERROR, ECC ERROR, RECEIVING BYTE, BYTE COMPLETE, PACKET COMPLETE, BUSY, COMMAND, NULL, NULL, NULL,  NULL, NULL, NULL,  NULL, NULL ]

	uint8_t num_error_bytes;		// probably only will be used for debugging
	uint16_t last_access_time;		// for TX buffers this is ONLY updated when reading data, for RX buffers this is ONLY updated when writing data
	uint16_t initial_use_time;		// TX time or RX time of first byte
	uint16_t expect_complete_time;	// computed RX time of last byte

	//uint8_t data_crc;				// TODO: possibly implement this to aid in checking for duplicate messages
	uint8_t trash_position;			// keep track of the byte where the message cut out

} ir_tx[6], ir_rx[6];

struct  
{
	uint8_t *buf;
	uint16_t size;					

	uint8_t receivers_used;			// set flags of the receiver numbers which successfully received the message

	uint8_t data_len;				// to be copied from the header
	uint16_t sender_ID;				// to be copied from the header 

	uint16_t last_access_time;
	uint16_t initial_receive_time;

	uint8_t read;
	uint8_t printed_read_prompt;	// strictly for debugging 'read'

} global_rx_buffer;

#define HEADER_POS_MSG_LENGTH 0
#define HEADER_POS_CRC 1
#define HEADER_POS_SENDER_ID_LOW 2
#define HEADER_POS_SENDER_ID_HIGH 3
#define HEADER_POS_TARGET_ID_LOW 4
#define HEADER_POS_TARGET_ID_HIGH 5
#define HEADER_LEN 6

struct Message_header
{
	// HEY! the order of these variables in THIS code matters
	// it must follow the #defines just above
	uint8_t msg_data_length;
	uint8_t data_crc;
	uint16_t sender_ID;
	uint16_t target_ID;
	
} tx_msg_header[6], rx_msg_header[6];

uint8_t last_command_source_id;

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

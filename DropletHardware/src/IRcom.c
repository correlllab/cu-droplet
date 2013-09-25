
// NDF 3/18/13: Dustin request to add a change to cause messages with arrival time difference less than some 
//	time-out time to be ignored. I dont really know how this change would be implemented yet, will have to try things later.
//	the goal of implementing this is to prevent duplicate messages from being treated as separate?

#include "debug.h"
#include "IRcom.h"
#define F_CPU 32000000UL

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "i2c.h"
#include "ecc.h"
//#include "boot.h"
#include "RGB_LED.h"
#include "serial_handler.h"
#include "droplet_init.h"	// contains the global droplet_ID variable

#define BINARY "%d%d%d%d%d%d%d%d"	// used for debug printfs
#define BYTETOBINARY(byte)  \
(byte & 0x80 ? 1 : 0), \
(byte & 0x40 ? 1 : 0), \
(byte & 0x20 ? 1 : 0), \
(byte & 0x10 ? 1 : 0), \
(byte & 0x08 ? 1 : 0), \
(byte & 0x04 ? 1 : 0), \
(byte & 0x02 ? 1 : 0), \
(byte & 0x01 ? 1 : 0)

static uint8_t IR_RX_DEBUG_MODE = 0;
static uint8_t IR_TX_DEBUG_MODE = 0;
static uint8_t IR_RX_DEBUG_STATUS = 0;
static uint8_t IR_TX_DEBUG_STATUS = 0;
static uint8_t IR_RX_DEBUG_DATA = 0;
static uint8_t IR_TX_DEBUG_DATA = 0;	// 2 prints data in hex
static uint8_t IR_RX_DEBUG_ENCODE = 0;
static uint8_t IR_TX_DEBUG_ENCODE = 0;

uint8_t buffer_memory_allocated = 0;

#define USING_XMEGA_A3 0 //As opposed to the A3U

/* Hardware addresses for the USARTs on the IR channels */
// channel[] is defined here, but declared in IRcom.h
USART_t* channel[] = {	&USARTC0,  //   -- Channel 0
						&USARTC1,  //   -- Channel 1
						&USARTD0,  //   -- Channel 2
						&USARTE0,  //   -- Channel 3
						&USARTE1,  //   -- Channel 4
						&USARTF0   //   -- Channel 5
					};

uint8_t rx_allowed[6] = {1,1,1,1,1,1};	// for debugging

/* Hardware addresses for the port pins with the carrier wave */
uint8_t ir_carrier_bm[] = { PIN0_bm, PIN1_bm, PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm };

// Important for calculating timeout:

// TAKES ~ 4 ms to transmit a single (8-bit) byte (~13 ms when byte is golay encoded like we do)
// [measured with RTC on 11/20/2012, range = {3,4,5}, mode = 4]
// EXACT: at 2400 bits per second (BAUD), 8 bits takes 3.3333 ms
// comment: there is no start bit nor parity bits included in a byte transmission
// XMEGA hardware supports up to 2 parity bits & start, giving up to 11 bits per byte possible

void ir_com_init(uint16_t buffersize)
{
	/* Initialize carrier waves */
	
	if(USING_XMEGA_A3)
	{
		//Only channels 0 and 1 will work in this case.
		uint8_t carrier_pins = PIN0_bm | PIN1_bm;
		PORTF.DIRSET = carrier_pins;
				
//		TCF0.CTRLE = TC_BYTEM_NORMAL_gc;		// "split mode" puts this timer counter into "Type 2 mode"
		TCF0.CTRLA |= TC_CLKSEL_DIV4_gc;		// see CTRLA description in TC2 mode
		TCF0.CTRLB = TC0_CCAEN_bm | TC0_CCBEN_bm | TC_WGMODE_SINGLESLOPE_gc;
//		TCF0.CTRLB = carrier_pins;				// Set TC outputs on carrier wave pins (see CTRLA description in TC2 mode)
		
		TCF0.PER = 210;												// 32MHz / (4 * 210) = 38kHz
		TCF0.CCA = TCF0.CCB = 105;		// 50% duty cycLE
	}
	else
	{
		uint8_t carrier_pins = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
		PORTF.DIRSET = carrier_pins;
	
		TCF2.CTRLE = TC_BYTEM_SPLITMODE_gc;		// "split mode" puts this timer counter into "Type 2 mode"
		TCF2.CTRLA |= TC_CLKSEL_DIV4_gc;		// see CTRLA description in TC2 mode
		TCF2.CTRLB = carrier_pins;				// Set TC outputs on carrier wave pins (see CTRLA description in TC2 mode)
	
		TCF2.HPER = 210;												// 32MHz / (4 * 210) = 38kHz
		TCF2.LPER = 210;
		TCF2.HCMPA = TCF2.HCMPB = TCF2.HCMPC = TCF2.HCMPD = 105;		// 50% duty cycle
		TCF2.LCMPA = TCF2.LCMPB = 105;		
	}
		
	/* Initialize send & receive buffers */
	
	if(!buffer_memory_allocated)
	{
		if (buffersize < IR_BUFFER_SIZE + HEADER_LEN)
		// then buffer size is too small for IR reprogramming, needs to be bigger
		{
			printf("IR buffersize increased to minimum size\r\n");
			buffersize = IR_BUFFER_SIZE + HEADER_LEN; 
		}
		else if (buffersize > IR_BUFFER_SIZE + HEADER_LEN)
		{
			printf("IR buffersize is bigger than needed\r\n");
		}
	
		for (uint8_t i = 0; i < 6; i++)
		{
			// QUESTIONS: 
			//	1. Why are the RX and TX buffers right next to each other in memory? Couldn't malloc just be called twice?
			//		A1: when new programming messages arrive, they can spill over into the TX space
			//	2. Since droplets shouldn't RX while they TX, couldn't both directions use the same buffer? 
		
			ir_rx[i].buf = malloc(2*buffersize);		// EXAMPLE, buffersize = 6
			ir_rx[i].size = buffersize;					// ir_rx.buf = [0][1][2][3][4][5][6][7][8][9][10][11]
			ir_rx[i].curr_pos = 0;						// ir_rx.size = 6
			ir_rx[i].curr_encoded_pos = 0;
			ir_rx[i].encoded = 0;
			ir_rx[i].data_len = 0;
			ir_rx[i].ir_status = 0;
			ir_rx[i].trash_position = 0;
		
			ir_tx[i].buf = ir_rx[i].buf + buffersize;	// ir_tx.buf = [6][7][8][9][10][11]
			ir_tx[i].size = buffersize;					// ir_tx.size = 6
			ir_tx[i].curr_pos = 0;
			ir_tx[i].curr_encoded_pos = 0;
			ir_tx[i].encoded = 0;
			ir_tx[i].data_len = 0;
			ir_tx[i].ir_status = 0;
	
			tx_msg_header[i].sender_ID = get_id_number();
		}

		/* Initialize the common RX buffer */
	
		global_rx_buffer.buf = malloc(buffersize);
		global_rx_buffer.size = buffersize;
		global_rx_buffer.receivers_used	= 0;
		global_rx_buffer.data_len = 0;
		global_rx_buffer.sender_ID = 0;
		global_rx_buffer.read = 1;			// pretend there is an already read message in the buffer
											// so check_for_new_messages() returns "no new messages"
		buffer_memory_allocated = 1;
	}

	else
		printf("IR buffer memory preallocated\r\n");

	/* Initialize UARTs */
	
	// RX pins as inputs:
	PORTC.DIRCLR = PIN2_bm | PIN6_bm;		// DIR 0,1							
	PORTD.DIRCLR = PIN2_bm;					// DIR 2
	PORTE.DIRCLR = PIN2_bm | PIN6_bm;		// DIR 3,4
	PORTF.DIRCLR = PIN2_bm;					// DIR 5
	
	// TX pins as outputs:
	PORTC.DIRSET = PIN3_bm | PIN7_bm;		// DIR 0,1									
	PORTD.DIRSET = PIN3_bm;					// DIR 2
	PORTE.DIRSET = PIN3_bm | PIN7_bm;		// DIR 3,4
	PORTF.DIRSET = PIN3_bm;					// DIR 5
	
	// Invert the output pins:
	PORTC.PIN3CTRL = PORT_INVEN_bm;												
	PORTC.PIN7CTRL = PORT_INVEN_bm;
	PORTD.PIN3CTRL = PORT_INVEN_bm;
	PORTE.PIN3CTRL = PORT_INVEN_bm;
	PORTE.PIN7CTRL = PORT_INVEN_bm;
	PORTF.PIN3CTRL = PORT_INVEN_bm;

	for (uint8_t i = 0; i < 6; i++)
	{
		channel[i]->CTRLA = (uint8_t) USART_RXCINTLVL_MED_gc | USART_TXCINTLVL_MED_gc;		// Set USART as med-level interrupts
		channel[i]->CTRLC = (uint8_t) USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;		// 8 bits, no parity
		
		channel[i]->BAUDCTRLA = 0;					// 2400 baud
		channel[i]->BAUDCTRLB =  0b11101101; 		// BSCALE = ?, BSEL = ?
		
		if(rx_allowed[i])
			channel[i]->CTRLB |= USART_RXEN_bm;		// Enable communication
		channel[i]->CTRLB |= USART_TXEN_bm;
	}
}

void set_ir_power(uint8_t dir, uint16_t power)
// note: the use of the term 'power' is more of a misnomer in this instance (in that its not linear)
// since voltage across the LED + digipot combo is constant, we actually setting the power of the LED
// by setting the resistance of the digipot
// the equation for LED light output power is:
// LED_POWER = (Vtot - Vled)(Vled/Rpot)
// this means, the light-power output is proportional to 1/(256-'power') relationship with the variable 'power'
{
	uint8_t varpotaddr, wiperaddr;
	
	switch (dir)
	{
		case 0: varpotaddr = 0x58; wiperaddr = 0x00; break;
		case 1: varpotaddr = 0x58; wiperaddr = 0x10; break;
		case 2: varpotaddr = 0x58; wiperaddr = 0x60; break;
		case 3: varpotaddr = 0x5A; wiperaddr = 0x60; break;
		case 4: varpotaddr = 0x5A; wiperaddr = 0x00; break;
		case 5: varpotaddr = 0x5A; wiperaddr = 0x10; break;
		default: return;
	}
	
	if (power > 255)
	{
		wiperaddr++;		// LSB of wiper address byte is MSB of data
		power = 0;			// 0x100 is maximum powerF
	}		
	
	i2c_startbit();
	i2c_sendbyte(varpotaddr);
	i2c_sendbyte(wiperaddr);
	i2c_sendbyte(power);
	i2c_stopbit();
}

uint8_t ir_send_command(uint8_t dir, uint8_t *data, uint16_t data_length)
{
	uint8_t return_val, oldstatus;
	
	oldstatus = ir_tx[dir].ir_status;
	
	// at this point in the code, what is the expected value of ir_tx[dir].ir_status???
	// why is this an OR-EQUALS and not just EQUALS??
	
	ir_tx[dir].ir_status |= IR_TX_STATUS_COMMAND_bm;
	return_val = ir_send(dir,data,data_length);	// ir_send currently ONLY returns 0
	
	if (return_val != 0)
	{
		ir_tx[dir].ir_status = oldstatus;
	// NO ELSE CASE?
		printf("ir_send_command ERROR %i\r\n", return_val);
	}	
	
	return return_val;
}


// return values: 0 = OK, 1-4 = errors
uint8_t ir_send(uint8_t dir, uint8_t *data, uint16_t data_length)
{
	uint8_t byte;
	uint16_t crc = 0;
		
	/* CHECK FOR ANY REASONS WHY TRANSMITTING IS A BAD IDEA */
	if (ir_tx[dir].ir_status & IR_TX_STATUS_BUSY_bm)	return 1;		// Error 1: transmitter busy
	if (ir_rx[dir].ir_status & IR_RX_STATUS_BUSY_bm)	return 2;		// Error 2: receiver busy				(TODO, may want to check all the dirs or brightness levels)
	if (data_length == 0)								return 3;		// Error 2: zero-length transmission
	if (data_length+HEADER_LEN > ir_tx[dir].size)		return 4;		// Error 3: message too big for transmit buffer
	
	/* DISABLE RECEIVE MESSAGES WHILE TRANSMITTING */
	// This disables the receive in ALL directions.
	// TODO: possible problem here, could turn off RX while RX in another direction is busy. 
	// Remember to re-enable receive messages after transmitting,
	// do this in TX complete interrupt (TXCIF)
	for (uint8_t i = 0; i < 6; i++)
	{
		channel[i]->CTRLB &= ~USART_RXEN_bm;
	}
				
	/* SET TRANSMIT BUSY FLAG WHILE RECEIVE IN PROGRESS */
	// this flag should stay on until the packet is finished,
	// in which case, IR_RX_STATUS_BUSY_bm should turn off, and then IR_RX_STATUS_PACKET_DONE_bm should turn on
	//TODO: Double check john change of this = to |=
	
	if((ir_tx[dir].ir_status != 0)||(ir_tx[dir].ir_status != IR_TX_STATUS_COMMAND_bm))
	{	// this is the double check, NDF will remove as TODO
		printf("NDF says: this is a serious error!\r\n");
	}
	
	
	ir_tx[dir].ir_status |= IR_TX_STATUS_BUSY_bm;

	crc = (uint8_t)droplet_ID;	// include sender ID in crc calculation, if using 16-bit IDs, truncate to 8-bit for this calculation

	/* CALCULATE THE CRC OF THE OUTBOUND MESSAGE */
	for (uint8_t i = 0; i < data_length; i++)
	{
		byte = data[i];
		crc = _crc16_update(crc, byte);
	}

	/* Copy header into software TX buffer */
	tx_msg_header[dir].msg_data_length = data_length;
	//tx_msg_header[dir].sender_ID = get_id_number();		// THIS NEVER CHANGES, MOVED TO INIT FUNCTION (do once)
	tx_msg_header[dir].data_crc = (uint8_t)crc;
	
	memcpy(ir_tx[dir].buf, &(tx_msg_header[dir]), HEADER_LEN);

	/* Copy message into software TX buffer behind header*/
	ir_tx[dir].data_len = data_length;
	memcpy(ir_tx[dir].buf+HEADER_LEN, data, data_length);			
	ir_tx[dir].curr_pos = 0;
	ir_tx[dir].curr_encoded_pos = 0;
	
	TCF2.CTRLB |= ir_carrier_bm[dir];								// Turn on carrier wave on port dir
	
	ir_tx[dir].initial_use_time = get_16bit_time();

	/* Enable DRE interrupt to begin transmission */
	channel[dir]->CTRLA |= USART_DREINTLVL_MED_gc;			// USARTs are medium-level priority			
	
	// *** THE WHOLE TX WILL OCCUR HERE VIA INTERRUPTS! *** //
	// 
	//					( non blocking )

	return 0;
}

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
void ir_transmit(uint8_t dir)
{
	uint16_t curr_time = get_16bit_time();
	uint16_t t_diff = curr_time - ir_tx[dir].last_access_time;
	
	// TAKES ~ 4 ms to transmit a single (encoded) byte
	// at 2400 bits per second (BAUD), 8 bits takes 3.3333 ms

	ir_tx[dir].last_access_time = curr_time;
	
	/* CHECK TO SEE IF MESSAGE IS COMPLETE */
	if(ir_tx[dir].curr_pos >= ir_tx[dir].data_len+HEADER_LEN)
	{
		channel[dir]->CTRLA &= ~USART_DREINTLVL_gm;
		return;	// this message is over, do not send any more
	}

	if(ir_tx[dir].curr_encoded_pos == 0)	// if this is the beginning of a new byte
	{
		uint16_t data = 0;

		if(ir_tx[dir].curr_pos < HEADER_LEN)
		{
			ir_tx[dir].ir_status |= IR_TX_STATUS_SEND_HEADER_bm;
			data |= IR_PACKET_HEADER_bm;
		}
		else
			ir_tx[dir].ir_status &=~IR_TX_STATUS_SEND_HEADER_bm;

		if((ir_tx[dir].curr_pos == 0)||(ir_tx[dir].curr_pos == ir_tx[dir].data_len+HEADER_LEN-1))
		{
			ir_tx[dir].ir_status |= IR_TX_STATUS_SEND_START_OR_END_bm;
			data |= IR_PACKET_START_OR_END_bm;
		}
		else
			ir_tx[dir].ir_status &=~IR_TX_STATUS_SEND_START_OR_END_bm;

		data |= ir_tx[dir].buf[ir_tx[dir].curr_pos];
		
		if((ir_tx[dir].ir_status & IR_TX_STATUS_COMMAND_bm) == IR_TX_STATUS_COMMAND_bm)
		{
			data |= IR_PACKET_COMMAND_bm;
			//printf("In ir_transmit, at top of byte, we saw the command flag.\r\n");
		}

		ir_tx[dir].encoded = golay_encode(data);
	}

	//note: >> RIGHT SHIFT gives the bigger bytes (e.g. 0011010100001111 >> 8 = 00110101)
	uint8_t tx_data = (ir_tx[dir].encoded >> 8*ir_tx[dir].curr_encoded_pos);

//	if(IR_TX_DEBUG_MODE >= 2)

	if(IR_TX_DEBUG_DATA == 1)		printf("DS:%02X\r\n", tx_data);		// DS for Data Send
	else if(IR_TX_DEBUG_DATA == 2)	printf("DS:"BINARY" "BINARY"\r\n", BYTETOBINARY(tx_data>>8),BYTETOBINARY(tx_data));

	if ((ir_tx[dir].ir_status & IR_TX_STATUS_BROADCAST_bm) && dir == 0)
	{
		for (uint8_t i = 0; i < 6; i++)
			channel[i]->DATA = tx_data;		// Send the data out all directions at once		
	}
	else
	{
		channel[dir]->DATA = tx_data;		// (* BOMBS AWAY *) this data is heading out
	}
	//printf("%x ",tx_data); //Used for debugging - prints raw bytes as we send them	

	if(IR_TX_DEBUG_MODE == 1)		printf("TX: %02X ", tx_data);

	ir_tx[dir].curr_encoded_pos++;

	if(ir_tx[dir].curr_encoded_pos == 3)
	{
		ir_tx[dir].curr_encoded_pos = 0;
		ir_tx[dir].curr_pos++;

		if(IR_TX_DEBUG_MODE == 1)		printf("\r\n");
	}
}

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
void ir_receive(uint8_t dir)
{
	// this routine receives a SINGLE uint8_t from the UART DATA register
	// important to know: if bytes are being encoded, then there will be more than 1 encoded byte associated with a
	// single data byte.  Therefore this routine will be looped through a few times to just receive a single (significant) byte 
	
	/* GET THE DATA, AND "Frame Error Status" IF APPLICABLE */
	uint8_t error_status = channel[dir]->STATUS & USART_FERR_bm;		// what does this do? is it used?
	uint8_t in_data = channel[dir]->DATA;				// (* KA-POW *) some data just came in
	//printf("%x ",in_data); //Used for debugging - prints raw bytes as we get them.
	if(ir_rx[dir].ir_status & IR_RX_STATUS_ERR_gc)
	{
		// there was an irrecoverable error in a previous byte,
		// we are done accepting new bytes on this channel
		//printf("Due to errors, disabling this channel!");
		return;
	}

	uint16_t curr_time = get_16bit_time();
	uint16_t t_diff = curr_time - ir_rx[dir].last_access_time;	// last_access_time is saved at the end of this routine

	uint8_t suspicious_package = 0;

	/* CHECK IF WE ARE EXPECTING A BRAND NEW MESSAGE */
	if((ir_rx[dir].curr_pos == 0)&&(ir_rx[dir].curr_encoded_pos == 0))
	{
		// this is the first byte of a new message
		// become busy while taking in data:
		ir_rx[dir].ir_status = IR_RX_STATUS_BUSY_bm;
		
		// we dont care when the last activity on this channel was, just accept the new time
		ir_rx[dir].initial_use_time = curr_time;
	}

	else if(ir_rx[dir].ir_status & IR_RX_STATUS_PACKET_DONE_bm)
	{	// a new message is coming in before the old message was moved to the central buffer
		// you will need to call 'check_for_new_messages()' more frequently to prevent this from happening
		// unfortunately, user code is responsible for calling this

		printf("dropping message %i", dir);
		printf("there is already one unread in the buffer\r\n");
		printf("buffer status: %02X\r\n", ir_rx[dir].ir_status);
		printf("message length: %02X\r\n", ir_rx[dir].data_len);

		return;
	}

	else // we are already in progress of receiving a message
	{
		/* CHECK THAT THE TIMING IS CORRECT FOR GOOD COMMUNICATION RATE */
		
		if(t_diff > 9) // 10 milliseconds is the chosen cutoff for time difference between bytes
		{
			// the timing check failed
			// this likely indicates that the previous message that was incoming never finished
			// the best strategy here is to assume it failed, and start a new message
			//
			// CORRECTION: actually, if the other bot was interrupted during transmit, and then continues
			// by picking up where it left off with a significant time difference, it would still
			// be acceptable to continue to receive the message, and perform the CRC at the end to make
			// sure that everything checks out.	(TODO, this isn't yet implemented)

			ir_rx[dir].ir_status |= IR_RX_STATUS_TIMEOUT_bm;

			return;
		}
	}
	
	/* SET THE STATUS FLAG TO BUSY & RX_BYTE */
	
	// we are both busy, and we are actually receiving a byte, right now
	ir_rx[dir].ir_status |= IR_RX_STATUS_BUSY_bm;

	/* CHECK IF AT BEGINNING OF A NEW ENCODED BYTE */
	// prepare to begin to receive a triple of bytes
	if(ir_rx[dir].curr_encoded_pos == 0)
	{
		ir_rx[dir].encoded = 0;
	}

	/* ADD THE NEW DATA TO THE CURRENTLY ACCUMULATING ENCODED BYTE */

	//note: << LEFT SHIFT writes the bigger bytes (e.g. 0011010100001111 = 0000000000001111 + 00110101 << 8)
	ir_rx[dir].encoded |=((uint32_t)in_data) << (8 * (ir_rx[dir].curr_encoded_pos));

	ir_rx[dir].curr_encoded_pos++;
	
	if(ir_rx[dir].curr_encoded_pos == 3)
	// if curr_encoded_pos now equals 3, then we have just received the 2th (of 0,1,2) encoded byte
	// we can now decode the byte 
	{
		uint16_t decoded;
		
		/* CHECK THE ENCODED BYTES FOR ERRORS */
		if (golay_find_errors(ir_rx[dir].encoded))
		{	// 'ERROR FOUND' PATHWAY
			uint8_t num_errors = 0;
			
			decoded = golay_decode(ir_rx[dir].encoded, &num_errors);
			
			// an error was fixed, it may have been fixed incorrectly?, or not?
			// mark that an error was fixed, and later it will be checked to see if the fixed data makes sense
			suspicious_package = 1;		
		}

		else
		{	// NO 'ERROR FOUND' PATHWAY
			decoded = golay_decode_fast(ir_rx[dir].encoded);
		}

		/* CHECK THE BYTE FLAGS ON DECODED */		
		if(0x0F00 & decoded)		// TODO: do something more than just printf the byte flags (TODO, do this below)
		{							// check that they make sense!  If they don't make sense for the place
									// we are in the message, then highly likely the flag was set by error correction
			/*
			if(decoded & IR_PACKET_HEADER_bm)			printf(" H");
			if(decoded & IR_PACKET_START_OR_END_bm)		printf(" S/E");
			if(decoded & IR_PACKET_PROG_bm)				printf(" P");
			if(decoded & 0x0100)						printf(" ???");		// this one is VERY BAD
			*/
		}
//TODO: What is the next curly brace starting?
		/* DO STUFF WITH DECODED */
		{
			/* ACTUALLY WRITE DOWN THE DATA IN THE RECEIVE SOFTWARE BUFFER, DO THIS NOW */
			ir_rx[dir].buf[ir_rx[dir].curr_pos] = (uint8_t)decoded;	// thanks
			//printf("%c ",ir_rx[dir].buf[ir_rx[dir].curr_pos]);
			if(ir_rx[dir].curr_pos < HEADER_LEN)
			{// you are inside the header
				
				/* CHECK THAT THE EXPECTED PACKET FLAGS ARE ON (or OFF) */
				if(ir_rx[dir].curr_pos == 0)	// if you just got the 0th byte, it had better have the correct packet flags set
				{
					if((decoded & IR_PACKET_START_OR_END_bm) != IR_PACKET_START_OR_END_bm)
					{
						/* this message is already trashed, and we don't know the sender */
						if(suspicious_package)
						{
							ir_rx[dir].ir_status |= IR_RX_STATUS_ECCERR_bm;

							return;
						}
					
						else
						{
							// FIRST BYTE RECEIVED IS NOT OF START/END TYPE!! (bad)
							printf("#ERR1#");		// leave this print statement in the final product
							// TODO: set some higher level error flag so this can be caught 'in the wild'
							// its not really a length error, but we dont have a better classification, this error is probably rare
							ir_rx[dir].ir_status |= IR_RX_STATUS_LENERR_bm;

							return;
						}
					}
				}
				else
				{
					if((decoded & IR_PACKET_START_OR_END_bm) == IR_PACKET_START_OR_END_bm)
					{
						/* this message is already trashed, and we may not know the sender */
						if(suspicious_package)
						{
							ir_rx[dir].ir_status |= IR_RX_STATUS_ECCERR_bm;

							return;
						}
						else
						{
							// A HEADER BYTE (not the first one) RECEIVED IS WRONGLY OF START/END TYPE!! (bad)
							printf("#ERR2#");		// leave this print statement in the final product
							// TODO: set some higher level error flag so this can be caught 'in the wild'
							// its not really a length error, but we dont have a better classification, this error is probably rare
							ir_rx[dir].ir_status |= IR_RX_STATUS_LENERR_bm;

							return;
						}
					}
				}	

				if((decoded & IR_PACKET_HEADER_bm) != IR_PACKET_HEADER_bm)
				{
					/* this message is already trashed, and we don't know the sender */
					if(suspicious_package)
					{
						ir_rx[dir].ir_status |= IR_RX_STATUS_ECCERR_bm;

						return;
					}

					else
					{
						// HEADER BYTE EXPECTED, NOT RECEIVED! (bad)
						printf("#ERR3#");		// leave this print statement in the final product
						// TODO: set some higher level error flag so this can be caught 'in the wild'
						// its not really a length error, but we dont have a better classification, this error is probably rare
						ir_rx[dir].ir_status |= IR_RX_STATUS_LENERR_bm;

						return;
					}

				}

				if(decoded & IR_PACKET_PROG_bm)
				{
					// PROG HEADER BYTE RECEIVED!

					/* this is OKAY, but unexpected in development, programming bytes come from the tower? */
					// if this is a programming byte, then we would expect all of them to be programming bytes
					if(ir_rx[dir].curr_pos == 0)
						// upon first notification, we should record that this is the case, so we can check against in future bytes
						ir_rx[dir].ir_status |= IR_RX_STATUS_PROGRAMMING_bm;
					else
					{
						if(!(ir_rx[dir].ir_status & IR_RX_STATUS_PROGRAMMING_bm))
						// PREVIOUS BYTES ARE NOT PROGRAMMING BYTES (bad)
						// this would be an error

						if(suspicious_package)
						{
							ir_rx[dir].ir_status |= IR_RX_STATUS_ECCERR_bm;

							return;
						}
					}
				}
				
				if(decoded & IR_PACKET_COMMAND_bm)
				{
					//Command header byte received.
					if(ir_rx[dir].curr_pos == 0)
					{
						//printf("S\r\n");
						//Record that we're recieving a command byte.
						ir_rx[dir].ir_status |= IR_RX_STATUS_COMMAND_bm;
					}
					else
					{
						if(!(ir_rx[dir].ir_status & IR_RX_STATUS_COMMAND_bm))
						{
							printf("Command flag set, but wasn't set in first byte of this message.\r\n");
						}
						
						if(suspicious_package)
						{
							ir_rx[dir].ir_status |= IR_RX_STATUS_ECCERR_bm;
							return;
						}
					}
				}
				
				// ////////////////////////////////////////////////////////////////////	//
				//			FILL OUT THE CRITICAL HEADER INFO AS IT COMES IN			//
				//							do this now									//
				// ////////////////////////////////////////////////////////////////////	//
				
				switch(ir_rx[dir].curr_pos)
				{
					case HEADER_POS_MSG_LENGTH:		// if currently importing the message length
						// now there are two places where the message length is stored
						// TODO: no need to store message length in two places?
						
						// check to see that the length is reasonable	
						if(((uint8_t)decoded+HEADER_LEN > ir_rx[dir].size)||((uint8_t)decoded == 0))
						{
							// either the reported (received) length is bigger than the buffer permits or is length zero
							// this is very bad, indicates that the expected length will be wrong
							// !!! LENGTH CHECK IS INVALID !!!

							ir_rx[dir].ir_status |= IR_RX_STATUS_LENERR_bm;

							return;
						}

						rx_msg_header[dir].msg_data_length = (uint8_t)decoded;
						ir_rx[dir].data_len = (uint8_t)decoded;

						// estimate the complete time
						ir_rx[dir].expect_complete_time = ir_rx[dir].initial_use_time + 18*ir_rx[dir].data_len;	// 18 = 6*3, a slight over-estimate
						// TODO: this does not account for clock wrap around
						break;
					
					case HEADER_POS_CRC:		// if currently importing the crc "checksum"
						rx_msg_header[dir].data_crc = (uint8_t)decoded;
						break;			
					
					case HEADER_POS_SENDER_ID:		// if currently importing the sender ID
						rx_msg_header[dir].sender_ID = (uint8_t)decoded;
						break;
					
					default:
						printf("\r\nYOU CANT GET HERE (processing receive header)\r\n");
						break;				
				}	
			}
			else
			{// you are beyond the header and receiving the actual message
				/* CHECK THAT THE EXPECTED BYTE/PACKET FLAGS ARE ON (or OFF) */
				if((decoded & IR_PACKET_START_OR_END_bm) == IR_PACKET_START_OR_END_bm)
				{
					// A BYTE RECEIVED IS OF START/END TYPE (bad/good)	// this may not be an error
				}

				if((decoded & IR_PACKET_HEADER_bm) == IR_PACKET_HEADER_bm)
				{
					// A BYTE RECEIVED IS WRONGLY OF HEADER TYPE!! (bad) 

					if(suspicious_package)
					{
						ir_rx[dir].ir_status |= IR_RX_STATUS_ECCERR_bm;

						return;
					}

				}
				if((decoded & IR_PACKET_PROG_bm)==IR_PACKET_PROG_bm)
				{
					// PROGRAMMING BYTE RECEIVED!

					/* this is OKAY, but unexpected in development, programming bytes come from the tower? */
					// if this is a programming byte, then we would expect all of them to be programming bytes
					if(!(ir_rx[dir].ir_status & IR_RX_STATUS_PROGRAMMING_bm))
						// programming status byte is only set on the first byte, all others that follow should also be programming bytes
						// this would be an error
						// PREV BYTES ARE NOT PROGRAMMING BYTES (bad)

						if(suspicious_package)
						{
							ir_rx[dir].ir_status |= IR_RX_STATUS_ECCERR_bm;

							return;
						}
				}
				if((decoded & IR_PACKET_COMMAND_bm)==IR_PACKET_COMMAND_bm)
				{
					if(!(ir_rx[dir].ir_status & IR_RX_STATUS_COMMAND_bm))
					{
						printf("Command flag set, but wasn't set in first byte of this message.\r\n");
					}
										
					if(suspicious_package)
					{
						ir_rx[dir].ir_status |= IR_RX_STATUS_ECCERR_bm;
						return;
					}
				}
			}


			if(ir_rx[dir].curr_pos-HEADER_LEN == rx_msg_header[dir].msg_data_length-1)
			{
				// if you got here in code, all the message bytes are in to have a successful message
				
				if((decoded & IR_PACKET_START_OR_END_bm) != IR_PACKET_START_OR_END_bm)
				{
					// LAST EXPECTED BYTE IS NOT OF START/END TYPE!! (bad)
					ir_rx[dir].ir_status |= IR_RX_STATUS_LENERR_bm;

					return;
				}
				else 
				{	// LAST EXPECTED BYTE IS MARKED AS START/END TYPE... (good)
					
					if((decoded & IR_PACKET_HEADER_bm) == IR_PACKET_HEADER_bm)
					{
						// ...LAST EXPECTED BYTE IS A HEADER (very bad)
						// TODO: this message is now suspect, handle it accordingly
						if(suspicious_package)
						{
							ir_rx[dir].ir_status |= IR_RX_STATUS_ECCERR_bm;

							return;
						}
					}
					else
					{
						// ...AND ITS NOT A HEADER (good)
					
						/* check that the lengths agree */
						if(ir_rx[dir].data_len == rx_msg_header[dir].msg_data_length)
						{
							// lengths agree (good)
						}
						else
						{
							// lengths dont agree (very bad)
							// TODO: this message is now suspect, handle it accordingly
							printf("YOU (probably) CANT GET HERE $$$$  ");
						}
						
						/* ASSUME THAT EVERYTHING IS CORRECT WITH THIS MESSAGE, PUBLISH TO GENERAL BUFFER */
						//(we want the general buffer to check the crc before passing through to user level)
						// dont check crc in ir_receive, this is a time consuming process
					
						if(ir_rx[dir].ir_status & IR_RX_STATUS_COMMAND_bm)
						{
							//If this is a command, we don't want to publish to buffer. We want to handle the command.
							char command[ir_rx[dir].data_len];
							last_command_source_id = rx_msg_header[dir].sender_ID;
							memcpy(command,ir_rx[dir].buf+HEADER_LEN,ir_rx[dir].data_len);
							handle_serial_command(command,ir_rx[dir].data_len);
							ir_reset_rx(dir);
							return;
						}
						else
						{
							/* YOUR MESSAGE IS NOW DONE */
							// the message should now sit in the buffer until check message ready
							// is able to copy it into the general buffer
							ir_rx[dir].ir_status |= IR_RX_STATUS_PACKET_DONE_bm;	// this is the ONLY! place in the code where this assignment is allowed
							ir_rx[dir].ir_status &= ~IR_RX_STATUS_BUSY_bm;			// holding a finished packet does not make a buffer 'busy'	(HOPE this doesn't cause problems!!!! (TODO verify))
						}

					}
				}
			} 
		}
		
		ir_rx[dir].curr_encoded_pos = 0;
		ir_rx[dir].curr_pos++;
	}

	/* CHECK IF MESSAGE ACTUAL MESSAGE LENGTH EXCEEDS EXPECTED MESSAGE LENGTH */ 
	if(ir_rx[dir].curr_pos >= ir_rx[dir].data_len+HEADER_LEN)
	{
		// MESSAGE IS TOO LONG (bad)
	}

	ir_rx[dir].last_access_time = curr_time;

	// end ir_receive()
}

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
void ir_transmit_complete(uint8_t dir)
{
	// this code is being executed because a TXCIF interrupt was executed, see pg. 305 AU manual:
	//	TXCIF: Transmit Complete Interrupt Flag
	//	This flag is set when the entire frame in the transmit shift register has been shifted out and there
	//	are no new data in the transmit buffer (DATA). TXCIF is automatically cleared when the transmit
	//	complete interrupt vector is executed. The flag can also be cleared by writing a one to its bit location.
	
	// Calling this code signals the end of the transmit process

	if(IR_RX_DEBUG_MODE >=2 )	printf("LIGHTS OFF %i\r\n", dir);
	
	TCF2.CTRLB &= ~ir_carrier_bm[dir];
	ir_reset_tx(dir);

	// THIS IS A HACK	(TODO, make sure we dont need this)
	ir_reset_rx(dir);
	// THIS IS A HACK

	/* REENABLE RECEIVE ON ALL PORTS */
	
	for (uint8_t i = 0; i < 6; i++)
	{
		if(rx_allowed[i])			// debugging allows to disable receive more permanently
			channel[i]->CTRLB |= USART_RXEN_bm;		// this enables receive on the USART
	}
}

void ir_reset_rx(uint8_t dir)
{
	if(rx_allowed[dir] == 0)		// debugging allows to disable receive more permanently
		return;

	ir_rx[dir].ir_status = 0;

	ir_rx[dir].data_len = 0;
	ir_rx[dir].curr_pos = 0;
	ir_rx[dir].curr_encoded_pos = 0;
	ir_rx[dir].encoded = 0;
	//ir_rx[dir].expect_complete_time = 0;
	ir_rx[dir].initial_use_time = 0;
	ir_rx[dir].trash_position =0;

	rx_msg_header[dir].data_crc = 0;		// redundant method to prevent old messages from reappearing as new messages
	rx_msg_header[dir].msg_data_length = 0; // redundant method to prevent old messages from reappearing as new messages
	rx_msg_header[dir].sender_ID = 0;

	channel[dir]->STATUS |= USART_RXCIF_bm;		// writing a 1 to this bit manually clears the RXCIF flag
}

void ir_reset_tx(uint8_t dir)
{	
	ir_tx[dir].ir_status = 0;
	
	ir_tx[dir].data_len = 0;
	ir_tx[dir].curr_pos = 0;
	ir_tx[dir].curr_encoded_pos = 0;
	ir_tx[dir].encoded = 0;
	channel[dir]->STATUS |= USART_TXCIF_bm;		// writing a 1 to this bit manually clears the TXCIF flag
}

// USER CALLS THIS
// returns 2 if you need to mark previous message as read before you can get a new one
// returns 1 if there is a new unread message
// returns 0 if there is no new message
// This routine populates the global rx buffer with new messages, if applicable 
uint8_t check_for_new_messages()
{
	// General Design of this routine:
	// 1.	If an unread message exists, then tell the user to read the unread message first.
	//		This would happed if this routine was called twice in succession without reading the message from the first time.
	// 2.	If new message exists, and the current message has already been read, then copy the oldest(newest?) message to the global buffer, (check all directions for duplicate messages)
	// 3.	If no new message exists, leave the current message in the global buffer, tell the user that there is no new messages

	uint8_t common_message_available_dirs = 0;

	/* CASE 1 */
	if(!global_rx_buffer.read)	// if the current message has not yet been read
	{
		printf(" - unread message available -\r\n");
		return 2;
	}

	/* CASE 2 */
	else if((common_message_available_dirs = /* assignment equals */ check_buffers_for_packets()))
	// if you get here, then global_rx_buffer.read == 1
	{
/*
//		printf("\r\nnew messages exist here:");
		for(uint8_t i = 0; i < 6; i++)
		{
			if((1<<i)&common_message_available_dirs)
			{
				// then there is a message in buffer i, may want to do somthing with this info
			}
		}
		printf("\r\n");
*/		
		uint8_t buffer_used_for_sample = 255;

		if(common_message_available_dirs & 1<<0)
			buffer_used_for_sample = 0;
		else if(common_message_available_dirs & 1<<1)
			buffer_used_for_sample = 1;
		else if(common_message_available_dirs & 1<<2)
			buffer_used_for_sample = 2;
		else if(common_message_available_dirs & 1<<3)
			buffer_used_for_sample = 3;
		else if(common_message_available_dirs & 1<<4)
			buffer_used_for_sample = 4;
		else if(common_message_available_dirs & 1<<5)
			buffer_used_for_sample = 5;
		else // cant get here
			printf("CANT GET HERE %%%%\r\n");
		
		/* COPY MESSAGE INTO GLOBAL ACCESS BUFFER */		
		memcpy(	global_rx_buffer.buf,							/* destination pointer */
				ir_rx[buffer_used_for_sample].buf+HEADER_LEN,	/* source pointer */
				ir_rx[buffer_used_for_sample].data_len);		/* num bytes to copy */

		global_rx_buffer.receivers_used = common_message_available_dirs;						// set flags of the receiver numbers which successfully received the message
		global_rx_buffer.data_len = ir_rx[buffer_used_for_sample].data_len;						// to be copied from the header
		global_rx_buffer.sender_ID = rx_msg_header[buffer_used_for_sample].sender_ID;			// to be copied from the header
		global_rx_buffer.last_access_time = ir_rx[buffer_used_for_sample].last_access_time;		// grab the last access time as the received time
		global_rx_buffer.initial_receive_time = ir_rx[buffer_used_for_sample].initial_use_time;	// grab the message arrival time as the received time
		global_rx_buffer.read = 0;																// mark new message as not read yet
		
		/* Clear the message buffers of those sharing the common read message */
		
		for(uint8_t dir = 0; dir < 6; dir++)
		{
			if(common_message_available_dirs & 1<<dir)	
			{
				//printf("resetting DONE: %02X; %i\r\n", ir_rx[dir].ir_status, dir);
				
				// recall:
				//	IR_RX_STATUS_BUSY_bm			0x01
				//	IR_RX_STATUS_PACKET_DONE_bm		0x02
				//	IR_RX_STATUS_BYTE_DONE_bm		0x04	not used
				//	IR_RX_STATUS_RX_BYTE_bm			0x08
				//  IR_RX_STATUS_ECCERR_bm			0x10
				//  IR_RX_STATUS_LENERR_bm			0x20
				//	IR_RX_STATUS_TIMEOUT_bm			0x40
				//	IR_RX_STATUS_PROGRAMMING_bm		0x80

				// what you may see:
				// DONE: 09 = 0000 1001, signals incomplete
				//	09 = (IR_RX_STATUS_BUSY_bm, IR_RX_STATUS_RX_BYTE_bm)

				// DONE: 0A = 0000 1010, currently signals complete												[GOOD]
				//	0A = (IR_RX_STATUS_PACKET_DONE_bm, IR_RX_STATUS_RX_BYTE_bm) "not busy"

				// DONE: 0B = 0000 1011, previously signaled complete, busy flag left on is no longer correct
				//	0B = (IR_RX_STATUS_BUSY_bm, IR_RX_STATUS_PACKET_DONE_bm, IR_RX_STATUS_RX_BYTE_bm)

				// DONE: 19 = 0001 1001, error correction probably set a programming flag
				//	19 = (IR_RX_STATUS_ECCERR_bm, IR_RX_STATUS_RX_BYTE_bm, IR_RX_STATUS_BUSY_bm)

				// DONE: 29 = 0010 1001, found when the length in the header is inappropriate, message should be discarded completely
				//	29 = (IR_RX_STATUS_LENERR_bm, IR_RX_STATUS_BUSY_bm, IR_RX_STATUS_RX_BYTE_bm)

				// DONE: 49 = 0100 1001, found when a byte was dropped causing a timeout error
				//	49 = (IR_RX_STATUS_TIMEOUT_bm, IR_RX_STATUS_BUSY_bm, IR_RX_STATUS_RX_BYTE_bm)

				// DONE: 4A = 0100 1010, found when a new packet tried to 'append' to an unread packet, code should prevent this now
				//	4A = (IR_RX_STATUS_TIMEOUT_bm, IR_RX_STATUS_PACKET_DONE_bm, IR_RX_STATUS_RX_BYTE_bm) "not busy"
			
				// DONE: 99 = 1001 1001, error correction probably set a programming flag
				//	99 = (IR_RX_STATUS_PROGRAMMING_bm, IR_RX_STATUS_ECCERR_bm, IR_RX_STATUS_RX_BYTE_bm, IR_RX_STATUS_BUSY_bm)

				// DONE: A9 = 1010 1001, error correction probably set a programming flag and also corrected length wrong
				//	A9 = (IR_RX_STATUS_PROGRAMMING_bm, IR_RX_STATUS_LENERR_bm, IR_RX_STATUS_RX_BYTE_bm, IR_RX_STATUS_BUSY_bm)

				ir_reset_rx(dir);	
			}
		}

		for(uint8_t dir = 0; dir < 6; dir++)
		{
			//printf("resetting float check: %02X; %i\r\n", ir_rx[dir].ir_status, dir);
			if(ir_rx[dir].ir_status == 0x09)	// 09 = 0000 1001, signals incomplete = (IR_RX_STATUS_BUSY_bm, IR_RX_STATUS_RX_BYTE_bm)
				if(get_16bit_time() > ir_rx[dir].expect_complete_time)
				{
					//printf("timeout, resetting\r\n");
					//printf("DIR %i {T: %u}\r\n",dir, get_16bit_time());
					//printf("float check timeout, initial use time: %u\r\n",ir_rx[dir].initial_use_time);
					//printf("float check timeout, last access time: %u\r\n",ir_rx[dir].last_access_time);
					//printf("float check timeout, expected completion time: %u\r\n",ir_rx[dir].expect_complete_time);
					ir_reset_rx(dir);
					//printf("resetting float check: %02X; %i\r\n", ir_rx[dir].ir_status, dir);
				}
		}
	
		return 1;	// HEY! global_rx_buffer.read is set in case 2
	}

	/* CASE 3 */
	else
	{
		// - no new messages - //
		return 0;
	}
}

// DO NOT CALL THIS DIRECTLY
// CALLED BY check_for_new_messages()
// returns 0 if no direction has a packet ready
// returns 0x00xxxxxx with bits set for buffers sharing the next message to take
uint8_t check_buffers_for_packets()
{
	// here < (less than) is used to look for oldest messages first, and best_time is started high
	
	uint16_t best_time = 0XFFFF;		//TODO: if TIME is changed to uint32_t, then this needs to be updated 
	uint8_t dir;
	uint8_t dirs_with_approximate_best_time = 0;
	uint8_t message_found = 0;
	uint8_t data;
	uint16_t computed_crc;
	uint8_t good_crc;

	for(dir = 0; dir < 6; dir++) // 1st for loop
	{
		if((ir_rx[dir].ir_status & IR_RX_STATUS_PACKET_DONE_bm)&&(ir_rx[dir].data_len > 0))
		{
			/* calculate CRC of the message */
			
			computed_crc = ir_rx[dir].buf[HEADER_POS_SENDER_ID];		// include sender ID in crc (init crc with this)
			for (uint8_t i = HEADER_LEN; i < ir_rx[dir].data_len+HEADER_LEN; i++)
			{
				data = ir_rx[dir].buf[i];
				computed_crc = _crc16_update(computed_crc, data);
			}
			
			if(rx_msg_header[dir].data_crc == (uint8_t)computed_crc)
			{
				good_crc = (uint8_t)computed_crc;
			
				/* find the direction with the lowest access time (oldest message)*/
				//if(ir_rx[dir].last_access_time < best_time)
				if(ir_rx[dir].initial_use_time < best_time)
				{
					best_time = ir_rx[dir].initial_use_time;
					message_found = 1;
				}
		
				// TODO: implement some way such that messages just bigger than T >= 0
				// evaluate to being newer than messages tagged as T <= MAX_TIME
				// right now, RvB droplets reboot at MAX_TIME so this will never be an issue until that is resolved
			}

			else
			{
				// CRC fail!
				// this message, despite all previous attempts at error checking, has an error
				// it needs to be discarded

				ir_reset_rx(dir);
			}
		}

	} // end 1st for

	if(!message_found)
	{
		// possibly also use this time to check for messages that have positive errors

		for(dir = 0; dir < 6; dir++)
		{
			if(ir_rx[dir].ir_status & IR_RX_STATUS_ERR_gc)
			{
				if(get_16bit_time() > ir_rx[dir].expect_complete_time)
				// don't reset too early, because its possible the message is still incoming, and we don't want to catch just the tail-end of it!
				{
					/* this is a good to use debug message
					printf("resetting ERROR (no msg): %02X; %i {T: %u}\r\n", ir_rx[dir].ir_status, dir, get_16bit_time());
					*/

					ir_reset_rx(dir);
				}
			}
		}
		
		// - got nothing - //
		return 0;
	}

	for(dir = 0; dir < 6; dir++) // 2nd for loop
	{
		// if you are here, then a message has been found
		// use this time to also check for messages that have positive errors
		if(ir_rx[dir].ir_status & IR_RX_STATUS_ERR_gc)
		{			
			// this will reset "length errors" without regard to expected completion time
			// if the error is a length error, most likely the expected completion time is wrong anyway
			// we think resetting this error here will be okay because at least one of the directions got a good message
			// so likely the transmission is over already

			/* this is a good to use debug message
			printf("resetting ERROR: %02X; %i\r\n", ir_rx[dir].ir_status, dir);
			*/
			ir_reset_rx(dir);
		}

		if(ir_rx[dir].ir_status & IR_RX_STATUS_PACKET_DONE_bm)
		{	
			/* find the directions with approximately the same access time (tdiff of 5 is permitted) */

			if(ir_rx[dir].initial_use_time < best_time+5)	
			{
				// redundant check of the crc (messages that 'look complete' with incorrect crc's should be thrown out above)
				if(rx_msg_header[dir].data_crc == good_crc)

					dirs_with_approximate_best_time |= 1<<(dir);
			}			
		}	
	} // end 2nd for
	
	return dirs_with_approximate_best_time;
}

// this checks all directions for busy
// returns 0 if not busy
// returns 1 if busy transmitting
// returns 2 if busy receiving
// if returning non-zero, busy_dir will be set to the direction that was found to be busy
uint8_t check_ir_busy(uint8_t *busy_dir)
{
	for(uint8_t dir = 0; dir < 6; dir++)
	{
		if(ir_tx[dir].ir_status & IR_TX_STATUS_BUSY_bm)
		{
			*busy_dir = dir;
			return 1;	// transmitting
		}

		else if(ir_rx[dir].ir_status & IR_RX_STATUS_BUSY_bm)
		{
			*busy_dir = dir;
			return 2;	// receiving
		}
	}
	
	return 0;
}

// this checks a single direction for busy
// returns 0 if not busy
// returns 1 if busy transmitting
// returns 2 if busy receiving
uint8_t check_direction_ir_busy(uint8_t dir)
{
	if(ir_tx[dir].ir_status & IR_TX_STATUS_BUSY_bm)
		return 1;	// transmitting

	else if(ir_rx[dir].ir_status & IR_RX_STATUS_BUSY_bm)
		return 2;	// receiving

	return 0;
}

// ISRs for IR channel 0
ISR( USARTC0_RXC_vect ) { ir_receive(0); }
ISR( USARTC0_TXC_vect ) { ir_transmit_complete(0); }
ISR( USARTC0_DRE_vect ) { ir_transmit(0); }

// ISRs for IR channel 1
ISR( USARTC1_RXC_vect ) { ir_receive(1); }
ISR( USARTC1_TXC_vect ) { ir_transmit_complete(1); }
ISR( USARTC1_DRE_vect ) { ir_transmit(1); }

// ISRs for IR channel 2
ISR( USARTD0_RXC_vect ) { ir_receive(2); }
ISR( USARTD0_TXC_vect ) { ir_transmit_complete(2); }
ISR( USARTD0_DRE_vect ) { ir_transmit(2); }

// ISRs for IR channel 3
ISR( USARTE0_RXC_vect ) { ir_receive(3); }
ISR( USARTE0_TXC_vect ) { ir_transmit_complete(3); }
ISR( USARTE0_DRE_vect ) { ir_transmit(3); }

// ISRs for IR channel 4
ISR( USARTE1_RXC_vect ) { ir_receive(4); }
ISR( USARTE1_TXC_vect ) { ir_transmit_complete(4); }
ISR( USARTE1_DRE_vect ) { ir_transmit(4); }

// ISRs for IR channel 5
ISR( USARTF0_RXC_vect ) { ir_receive(5); }
ISR( USARTF0_TXC_vect ) { ir_transmit_complete(5); }
ISR( USARTF0_DRE_vect ) { ir_transmit(5); }


uint8_t ir_broadcast_command(uint8_t *data, uint16_t data_length)
{
	uint8_t oldstatus[6];
	for(uint8_t dir=0 ; dir < 6 ; dir++) 
	{
		oldstatus[dir] = ir_tx[dir].ir_status;
		ir_tx[dir].ir_status |= IR_TX_STATUS_COMMAND_bm;
	}
	uint8_t retval = ir_broadcast(data,data_length);
	if (retval != 0) 
	{
		for(uint8_t dir=0 ; dir < 6 ; dir++) ir_tx[dir].ir_status = oldstatus[dir];
	}
	return retval;
}

// TODO: Refactor ir_send() and ir_broadcast() with better modularity
uint8_t ir_broadcast(uint8_t *data, uint16_t data_length)
{
	uint8_t dir;

	/* CHECK FOR ANY REASONS WHY TRANSMITTING IS A BAD IDEA */
	if (data_length == 0)								return 3;		// Error 2: zero-length transmission
	if (data_length+HEADER_LEN > ir_tx[0].size)			return 4;		// Error 3: message too big for transmit buffer
	for (dir = 0; dir < 6; dir++)
	{
		if (ir_tx[dir].ir_status & IR_TX_STATUS_BUSY_bm)	return 1;		// Error 1: transmitter busy
		if (ir_rx[dir].ir_status & IR_RX_STATUS_BUSY_bm)	return 2;		// Error 2: receiver busy				(TODO, may want to check all the dirs or brightness levels)
	}		

	for (dir = 0; dir < 6; dir++)
	{
		/* DISABLE RECEIVE MESSAGES WHILE TRANSMITTIG */
		channel[dir]->CTRLB &= ~USART_RXEN_bm;

		/* SET TX STATUS TO BUSY AND BROADCAST IN ALL DIRECTIONS */
		ir_tx[dir].ir_status |= IR_TX_STATUS_BUSY_bm | IR_TX_STATUS_BROADCAST_bm;
		
		/* TURN ON CARRIER WAVES */
		TCF2.CTRLB |= ir_carrier_bm[dir];
	}
	
	// Broadcast data using TX 0's buffers
	dir = 0;

	uint8_t byte;
	uint16_t crc = 0;

	crc = (uint8_t)droplet_ID;	// include sender ID in crc calculation, has to be 8-bit for now

	/* CALCULATE THE CRC OF THE OUTBOUND MESSAGE */
	for (uint8_t i = 0; i < data_length; i++)
	{
		byte = data[i];
		//printf("byte: %i %02X\r\n", i, byte);
		crc = _crc16_update(crc, byte);
	}

	tx_msg_header[dir].msg_data_length = data_length;
	tx_msg_header[dir].data_crc = (uint8_t)crc;
	
	memcpy(ir_tx[dir].buf, &(tx_msg_header[dir]), HEADER_LEN);

	if(IR_RX_DEBUG_MODE >= 2)
	for(uint8_t header_cursor = 0; header_cursor < HEADER_LEN; header_cursor++)
	printf("HEADER: %02X, [%i]\r\n", ((uint8_t*)(&tx_msg_header[dir]))[header_cursor], header_cursor);

	ir_tx[dir].data_len = data_length;
	memcpy(ir_tx[dir].buf+HEADER_LEN, data, data_length);
	ir_tx[dir].curr_pos = 0;
	ir_tx[dir].curr_encoded_pos = 0;
	
	ir_tx[dir].initial_use_time = get_16bit_time();

	/* Enable DRE interrupt to begin transmission */
	
	channel[dir]->CTRLA |= USART_DREINTLVL_MED_gc;			// USARTs are medium-level priority			

	
	// *** THE WHOLE TX WILL OCCUR HERE VIA INTERRUPTS! *** //
	// 
	//					( non blocking )

	return 0;
}


void print_RX_buffer(uint8_t dir)
{
	printf("\r\ndir %i buffer: ", dir);
	for(uint8_t i = 0; i < ir_rx[dir].size; i++)
		printf("%02X ", ir_rx[dir].buf[i]);
	printf("\r\r\n");
}

void print_RX_buffer_status(uint8_t dir)
{
	printf("\r\ndir %i buffer status: %02X ", dir, ir_rx[dir].ir_status);
	if(ir_rx[dir].ir_status & IR_RX_STATUS_ERR_gc)
		printf("ERROR\r\n");
	else
		printf("\r\n");
}

void erase_RX_buffer(uint8_t dir)
{
	for(uint8_t i = 0; i < ir_rx[dir].size; i++)
		ir_rx[dir].buf[i] = 0;

	printf("ERASED %i\r\n",dir);
}

// USER CODE CALLS THIS BEFORE ir_send()
// this routine examines the IR RX and TX channels for busy status before transmitting
// the RX channels need to be clear before transmitting, since transmitting will shut these down temporarily
// returns 0 [BAD] if TX is busy, inspect this problem before proceeding with ir_send() 
// returns 1 [GOOD] if all RX channels were ready with no work required, nearby IR likely quiet
// returns 2 [GOOD] if all RX channels are now clear, nearby IR may be noisy, (some (possibly all) channels had to be reset)
// returns 3 [GOOD] if all RX channels are now clear, nearby IR may be VERY noisy, (some (possibly all) channels had to be reset)
uint8_t OK_to_send()
{
	uint8_t busy_dir = 255;

	if(check_ir_busy(&busy_dir))
	{
		uint8_t busy_count = 0;
	
		while(check_ir_busy(&busy_dir))
		{
			busy_count++;
		
			if(1 == check_ir_busy(&busy_dir))
			{
				// busy is a result of TX
				printf("IR busy: %i TX\r\n", busy_dir);
				return 0;
			}
			
			else if(2 == check_ir_busy(&busy_dir))
			{
				/* this is a good to use debug message
				printf("IR busy: %i RX\r\n", busy_dir);
				printf("busy with curr_pos: %i, encoded_pos %i\r\n",ir_rx[busy_dir].curr_pos, ir_rx[busy_dir].curr_encoded_pos);
				printf("last used: %u, ",get_16bit_time()-ir_rx[busy_dir].last_access_time);
				*/
				
				if(get_16bit_time() > ir_rx[busy_dir].expect_complete_time)
				{
					// this may happen if the end of an inbound packet was dropped
					
					/* this is a good to use debug message
					printf("IR reset busy, SHOULD BE DONE (good)\r\n");
					*/
					ir_reset_rx(busy_dir);
				}
			
				else if(get_16bit_time()-ir_rx[busy_dir].last_access_time > 20)		// TWENTY (milliseconds) <-  chosen arbitrarily, this is a hard-coded varaible
				{
					// this may happen if bytes are being received intermittently (bytes are being dropped from a weak signal)
					/* this is a good to use debug message
					printf("IR reset busy, UNUSED (good)\r\n");
					print_RX_buffer(busy_dir);
					*/
					ir_reset_rx(busy_dir);
				}

				_delay_ms(10);					// TEN (milliseconds) <-  chosen arbitrarily, this is a hard-coded varaible
				if(busy_count > 20)				// TWENTY (count) <-  chosen arbitrarily, this is a hard-coded varaible
				{
					/* this is a good to use debug message
					printf("IR reset busy by COUNT\r\n");
					*/
					ir_reset_rx(busy_dir);
				}
			}
			
		} // END while

		if(busy_count <= 20)					// TWENTY (count) <-  chosen arbitrarily, this is a hard-coded varaible
		{
			/* this is a good to use debug message
			printf("IR busy fixed itself (probably OK)\r\n");
			*/
			return 2;
		}

		else
		{
			printf("IR fixed busy !!! (probably bad)\r\n");
			return 3;
		}
	}

	else // not busy
		return 1;
}
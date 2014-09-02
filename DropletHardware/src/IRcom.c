#include "IRcom.h"

/* Hardware addresses for the port pins with the carrier wave */
uint8_t ir_carrier_bm[] = { PIN0_bm, PIN1_bm, PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm };

//USART_t* channel[] = {
	//&USARTC0,  //   -- Channel 0
	//&USARTC1,  //   -- Channel 1
	//&USARTD0,  //   -- Channel 2
	//&USARTE0,  //   -- Channel 3
	//&USARTE1,  //   -- Channel 4
	//&USARTF0   //   -- Channel 5
//};

// TAKES ~ 4 ms to transmit a single (8-bit) byte (~13 ms when byte is golay encoded like we do)
// [measured with RTC on 11/20/2012, range = {3,4,5}, mode = 4]
// EXACT: at 2400 bits per second (BAUD), 8 bits takes 3.3333 ms
// comment: there is no start bit nor parity bits included in a byte transmission
// XMEGA hardware supports up to 2 parity bits & start, giving up to 11 bits per byte possible
void ir_com_init()
{
	/* Initialize carrier waves */
	uint8_t carrier_pins = PIN0_bm | PIN1_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
	PORTF.DIRSET = carrier_pins;
	
	TCF2.CTRLE = TC_BYTEM_SPLITMODE_gc;		// "split mode" puts this timer counter into "Type 2 mode"
	TCF2.CTRLA |= TC_CLKSEL_DIV4_gc;		// see CTRLA description in TC2 mode
	TCF2.CTRLB = carrier_pins;				// Set TC outputs on carrier wave pins (see CTRLA description in TC2 mode)
	
	TCF2.HPER = 210; TCF2.LPER = 210; // 32MHz / (4 * 210) = 38kHz
	TCF2.HCMPA = 105; TCF2.HCMPB = 105; TCF2.HCMPC = 105; // 50% Duty Cycle
	TCF2.HCMPD = 105; TCF2.LCMPA = 105; TCF2.LCMPB = 105; // 50% Duty Cycle
	
	/* Initialize UARTs */
	// RX pins as inputs:
	PORTC.DIRCLR = PIN2_bm | PIN6_bm;		// DIR 0,1							
	PORTD.DIRCLR = PIN2_bm;					// DIR 2
	PORTE.DIRCLR = PIN2_bm | PIN6_bm;		// DIR 3,4
	PORTF.DIRCLR = PIN2_bm;					// DIR 5
	
	// TX pins as outputs:
	PORTC.DIRSET = PIN3_bm | PIN7_bm;		// DIR 0,1									
	//PORTD.DIRSET = PIN3_bm;					// DIR 2
	//PORTD.DIRCLR = PIN3_bm;					//undoes the previous line; sets the pin as output. (Just for the bugged samples)
	PORTE.DIRSET = PIN3_bm | PIN7_bm;		// DIR 3,4
	//PORTF.DIRSET = PIN3_bm;					// DIR 5
	//PORTF.DIRCLR = PIN3_bm;					//undoes the previous line; sets the pin as output. (Just for the bugged samples)
	
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
		
		//channel[i]->BAUDCTRLA = 0b00000000; channel[i]->BAUDCTRLB = 0b11101101; // 2400 baud
		channel[i]->BAUDCTRLA = 0b01110000; channel[i]->BAUDCTRLB = 0b00000010; //3200 baud.
		
		channel[i]->CTRLB |= USART_RXEN_bm;		// Enable communication
		channel[i]->CTRLB |= USART_TXEN_bm;
	}
	set_all_ir_powers(256);
	for(uint8_t dir=0; dir<6; dir++) clear_ir_buffer(dir); //this initializes the buffer's values to 0.
	last_ir_msg = NULL;
	schedule_task(1000/IR_UPKEEP_FREQUENCY, perform_ir_upkeep, NULL);
}

int8_t recover_msg(uint8_t dirs)
{
	//for now, we assume the header information all got through okay. If it didn't, we won't be able to recover the message.
	//even more temporarily, this assumes that we have exactly two channels with bad_crcs.
	int8_t dirA = -1;
	int8_t dirB = -1;
	for(uint8_t dir=0; dir<6; dir++)
	{
		if(!(dirs&(1<<dir))) continue; //if a channel wasn't marked as having a bad crc, skip it.
		if(dirA<0) dirA = (1<<dir);
		else if(dirB<8) dirB = (1<<dir);
		else return; //too many channels with bad crc
	}	
	if(dirA<0 || dirB<0) return -1; //not enough channels with bad crc.
	
	uint16_t crcA = 0;
	uint16_t crcB = 0;
	
	for(uint8_t i=0;i<IR_BUFFER_SIZE;i++)
	{
		crcA = _crc16_update(crcA, ir_rxtx[dirA].buf[i]);
		crcB = _crc16_update(crcB, ir_rxtx[dirB].buf[i]);
		
	}
}

int8_t recover_msg_recur(uint8_t dirs, uint16_t crc, uint8_t pos)
{
	uint8_t seen[6];
	for(uint8_t dir=0;dir<6;dir++)
	{
		if(!(dirs&(1<<dir))) continue;
		if(pos>=ir_rxtx[dir].data_length)
		{
			if(ir_rxtx[dir].data_crc = crc) return 1;//true;//good match found!
			else return 0;//false;
		}
		for(uint8_t prev_dir=(dir-1);prev_dir<dir; prev_dir++) if(seen[prev_dir]=ir_rxtx[dir].buf[pos]) continue;
		if(recover_msg_recur(dirs, _crc16_update(crc, ir_rxtx[dir].buf[pos]), pos++)) printf("%c", ir_rxtx[dir].buf[pos]);
		else seen[dir]=ir_rxtx[dir].buf[pos];
	}
}

void perform_ir_upkeep()
{
	//For now, can only get one message per call to this function.
	int8_t msg_chan = -1; //This value is negative if we don't have a message, and otherwise holds the value of the chanel which received the message.
	uint8_t bad_crc_dirs = 0;
	for(int8_t dir=0; dir<6; dir++) //This first loop looks for a channel on which we got a good message.
	{
		if(!(ir_rxtx[dir].status&IR_STATUS_COMPLETE_bm)) continue; //ignore a channel if it isn't done yet.
		if(ir_rxtx[dir].target_ID) if(ir_rxtx[dir].target_ID != get_droplet_id()) continue;//Is this message targeted, to this droplet?	

		uint16_t crc = 0;
		for(uint8_t i=0; i<ir_rxtx[dir].data_length; i++) crc = _crc16_update(crc, ir_rxtx[dir].buf[i]); //Calculate CRC of inbound message.
		if(ir_rxtx[dir].data_crc == crc) //crc check passed, hurray!
		{
			msg_chan = dir;
			break;
		}
		else //crc check failed.
		{	
			//for(uint8_t i=0;i<ir_rxtx[dir].data_length; i++) printf("%c",ir_rxtx[dir].buf[i]);
			//printf("\r\n");
			bad_crc_dirs |= (0x1<<dir);
		}
	}
	if(msg_chan<0 && bad_crc_dirs>0) msg_chan = recover_msg(bad_crc_dirs);
	for(int8_t dir=0;dir<6;dir++) //free up the ir buffers for all the channels we don't need.
	{
		if(ir_rxtx[dir].status&IR_STATUS_COMPLETE_bm) if(dir!=msg_chan) clear_ir_buffer(dir);
	}
	if(msg_chan>0) //If we got a good message.
	{
		if(ir_rxtx[msg_chan].status & IR_STATUS_COMMAND_bm) //If the message is a command.
		{
			char msg[ir_rxtx[msg_chan].data_length+1];
			memcpy(msg, ir_rxtx[msg_chan].buf, ir_rxtx[msg_chan].data_length);
			msg[ir_rxtx[msg_chan].data_length]='\0';
			uint8_t cmd_length = ir_rxtx[msg_chan].data_length;
			clear_ir_buffer(msg_chan);
			handle_serial_command(msg, cmd_length);
		}
		else //Normal message, so add to msg queue
		{
			msg_node* new_node = (msg_node*)malloc(sizeof(msg_node));
			char* tmp = (char*)malloc(ir_rxtx[msg_chan].data_length+1);
			new_node->msg = tmp;
			memcpy(new_node->msg, ir_rxtx[msg_chan].buf, ir_rxtx[msg_chan].data_length);
			new_node->msg[ir_rxtx[msg_chan].data_length]='\0';
			new_node->arrival_time = ir_rxtx[msg_chan].last_byte;
			new_node->sender_ID = ir_rxtx[msg_chan].sender_ID;
			new_node->msg_length = ir_rxtx[msg_chan].data_length;
			new_node->prev = last_ir_msg;
			last_ir_msg = new_node;
			clear_ir_buffer(msg_chan);
		}
	}
	schedule_task(1000/IR_UPKEEP_FREQUENCY, perform_ir_upkeep, NULL);
	//printf("\r\n");
}

inline void clear_ir_buffer(uint8_t dir)
{
	ir_rxtx[dir].status = 0;
	ir_rxtx[dir].data_length = 0;
	ir_rxtx[dir].curr_pos = 0;
	ir_rxtx[dir].target_ID = 0;
	ir_rxtx[dir].sender_ID = 0;
	channel[dir]->CTRLB |= USART_RXEN_bm; //this enables receive on the USART
}

void ir_targeted_cmd(uint8_t dirs, char *data, uint16_t data_length, uint16_t target)
{
	for(uint8_t dir=0; dir<6; dir++) if(dirs&(1<<dir)) ir_rxtx[dir].target_ID=target;
	for(uint8_t dir=0; dir<6; dir++) if(dirs&(1<<dir)) ir_rxtx[dir].status = IR_STATUS_COMMAND_bm;
	ir_send(dirs, data, data_length);
}

void ir_cmd(uint8_t dirs, char *data, uint16_t data_length)
{	
	for(uint8_t dir=0; dir<6; dir++) if(dirs&(1<<dir)) ir_rxtx[dir].status = IR_STATUS_COMMAND_bm;
	ir_send(dirs, data, data_length);
}

void ir_targeted_send(uint8_t dirs, char *data, uint16_t data_length, uint16_t target)
{
	for(uint8_t dir=0; dir<6; dir++) if(dirs&(1<<dir)) ir_rxtx[dir].target_ID=target;
	ir_send(dirs, data, data_length);
}

void ir_send(uint8_t local_dirs, char *data, uint8_t data_length)
{
	uint8_t dirs = local_dirs;
		
	for(uint8_t dir=0; dir<6; dir++) 
	{
		if(ir_rxtx[dir].status & IR_STATUS_BUSY_bm) dirs&=(~(1<<dir)); //remove a direction from sending list if it is busy.
		else
		{
			channel[dir]->CTRLB &= ~USART_RXEN_bm; //Disable receive messages on this channel while transmitting.
			ir_rxtx[dir].status |= IR_STATUS_BUSY_bm;
		}
	}
	
	uint16_t crc = 0;
	for(uint8_t i=0; i<data_length; i++) crc = _crc16_update(crc, data[i]); //Calculate CRC of outbound message.
	
	for(uint8_t dir=0; dir<6; dir++)
	{
		if(dirs&(1<<dir))
		{
			ir_rxtx[dir].data_length = data_length;
			ir_rxtx[dir].data_crc = crc;
			ir_rxtx[dir].curr_pos = 0;
			ir_rxtx[dir].sender_ID = get_droplet_id();		
			memcpy(ir_rxtx[dir].buf, data, data_length); 
			TCF2.CTRLB |= ir_carrier_bm[dir];		// Turn on carrier wave on port dir
		}
	}
	
	for(uint8_t dir=0; dir<6; dir++)
	{
		 if(dirs&(1<<dir)) ir_rxtx[dir].last_byte = 0;
	}
	for(uint8_t dir=0; dir<6; dir++)
	{
		if(dirs&(1<<dir)) channel[dir]->CTRLA |= USART_DREINTLVL_MED_gc;
	}

	/* The whole transmission will now occur in interrupts. */
}

// To be called from interrupt handler only. Do not call.
void ir_receive(uint8_t dir)
{
	uint8_t in_byte = channel[dir]->DATA;				// Some data just came in
	printf("%02X ",in_byte); //Used for debugging - prints raw bytes as we get them.	

	uint32_t now = get_32bit_time();
	if(now-ir_rxtx[dir].last_byte > IR_MSG_TIMEOUT) ir_rxtx[dir].curr_pos = 0;
	ir_rxtx[dir].last_byte = get_32bit_time();
	
	switch(ir_rxtx[dir].curr_pos)
	{
		case HEADER_POS_MSG_LENGTH:		if(in_byte&DATA_LEN_CMD_bm) ir_rxtx[dir].status|=IR_STATUS_COMMAND_bm; ir_rxtx[dir].data_length	= in_byte&DATA_LEN_VAL_bm; break;
		case HEADER_POS_CRC_LOW:		ir_rxtx[dir].data_crc		= (uint16_t)in_byte; break;
		case HEADER_POS_CRC_HIGH:		ir_rxtx[dir].data_crc	   |= (((uint16_t)in_byte)<<8); break;
		case HEADER_POS_SENDER_ID_LOW:	ir_rxtx[dir].sender_ID		= (uint16_t)in_byte; break;
		case HEADER_POS_SENDER_ID_HIGH:	ir_rxtx[dir].sender_ID	   |= (((uint16_t)in_byte)<<8); break;
		case HEADER_POS_TARGET_ID_LOW:  ir_rxtx[dir].target_ID		= (uint16_t)in_byte; break;
		case HEADER_POS_TARGET_ID_HIGH: ir_rxtx[dir].target_ID	   |= (((uint16_t)in_byte)<<8); break;
		default: ir_rxtx[dir].buf[ir_rxtx[dir].curr_pos-HEADER_LEN] = in_byte;
	}
	ir_rxtx[dir].curr_pos++;
	if(ir_rxtx[dir].curr_pos>=((ir_rxtx[dir].data_length)+HEADER_LEN))
	{	
		ir_rxtx[dir].status |= IR_STATUS_COMPLETE_bm;
		channel[dir]->CTRLB &= ~USART_RXEN_bm; //Disable receiving messages on this channel until the message has been processed.
		//schedule_task(5, print_completed_msg, (void*)dir);
}

}

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
volatile uint8_t next_byte;
void ir_transmit(uint8_t dir)
{	

	switch(ir_rxtx[dir].curr_pos)
	{
		case HEADER_POS_MSG_LENGTH:		next_byte  = ir_rxtx[dir].data_length; if(ir_rxtx[dir].status & IR_STATUS_COMMAND_bm) next_byte|= DATA_LEN_CMD_bm; break;
		case HEADER_POS_CRC_LOW:		next_byte  = (uint8_t)(ir_rxtx[dir].data_crc&0xFF); break;
		case HEADER_POS_CRC_HIGH:		next_byte  = (uint8_t)((ir_rxtx[dir].data_crc>>8)&0xFF); break;
		case HEADER_POS_SENDER_ID_LOW:  next_byte  = (uint8_t)(ir_rxtx[dir].sender_ID&0xFF); break;
		case HEADER_POS_SENDER_ID_HIGH: next_byte  = (uint8_t)((ir_rxtx[dir].sender_ID>>8)&0xFF); break;
		case HEADER_POS_TARGET_ID_LOW:  next_byte  = (uint8_t)(ir_rxtx[dir].target_ID&0xFF); break;
		case HEADER_POS_TARGET_ID_HIGH: next_byte  = (uint8_t)((ir_rxtx[dir].target_ID>>8)&0xFF); break;
		default: next_byte = ir_rxtx[dir].buf[ir_rxtx[dir].curr_pos - HEADER_LEN];
	}
	//printf("%02X ", next_byte);	
	channel[dir]->DATA = next_byte;
	ir_rxtx[dir].curr_pos++;
	/* CHECK TO SEE IF MESSAGE IS COMPLETE */
	if(ir_rxtx[dir].curr_pos >= (ir_rxtx[dir].data_length+HEADER_LEN))
	{
		//printf("\r\n");
		channel[dir]->CTRLA &= ~USART_DREINTLVL_gm; //Turn off interrupt things.
	}

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
	//	To do: this suggests we don't really need to do this a byte at a time? Need to check.
	//  Calling this code signals the end of the transmit process
	//printf("\r\n");
	TCF2.CTRLB &= ~ir_carrier_bm[dir]; //Turn off the carrier wave.

	ir_rxtx[dir].status = 0;
	ir_rxtx[dir].data_length = 0;
	ir_rxtx[dir].curr_pos = 0;
	ir_rxtx[dir].target_ID = 0;
	ir_rxtx[dir].sender_ID = 0;
	channel[dir]->STATUS |= USART_TXCIF_bm;		// writing a 1 to this bit manually clears the TXCIF flag
	channel[dir]->CTRLB |= USART_RXEN_bm;	// this enables receive on the USART
}

void ir_reset_rx(uint8_t dir)
{
	ir_transmit_complete(dir); //main reason I can see not to this is that when we're receiving we don't need to turn off the carrier wave. Doing shouldn't hurt, however?
}


void wait_for_ir(uint8_t dirs)
{
	uint8_t busy;
	do
	{
		busy=0;
		for(uint8_t dir=0; dir<6; dir++)
		{
			if(dirs&(1<<dir))
			{
				if(ir_rxtx[dir].status & IR_STATUS_UNAVAILABLE_bm)
				{
					busy=1;
					break;
				}
			}
		}
	} while (busy);
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


//// USER CALLS THIS
//// returns 2 if you need to mark previous message as read before you can get a new one
//// returns 1 if there is a new unread message
//// returns 0 if there is no new message
//// This routine populates the global rx buffer with new messages, if applicable 
//uint8_t check_for_new_messages()
//{
	//// General Design of this routine:
	//// 1.	If an unread message exists, then tell the user to read the unread message first.
	////		This would happed if this routine was called twice in succession without reading the message from the first time.
	//// 2.	If new message exists, and the current message has already been read, then copy the oldest(newest?) message to the global buffer, (check all directions for duplicate messages)
	//// 3.	If no new message exists, leave the current message in the global buffer, tell the user that there is no new messages
//
	//uint8_t common_message_available_dirs = 0;
//
	///* CASE 1 */
	//if(!global_rx_buffer.read)	// if the current message has not yet been read
	//{
		//printf(" - unread message available -\r\n");
		//return;
	//}
//
	///* CASE 2 */
	//else if((common_message_available_dirs = /* assignment equals */ check_buffers_for_packets()))
	//// if you get here, then global_rx_buffer.read == 1
	//{
///*
////		printf("\r\nnew messages exist here:");
		//for(uint8_t i = 0; i < 6; i++)
		//{
			//if((1<<i)&common_message_available_dirs)
			//{
				//// then there is a message in buffer i, may want to do somthing with this info
			//}
		//}
		//printf("\r\n");
//*/		
		//uint8_t buffer_used_for_sample = 0xFF;
//
		//if(common_message_available_dirs & 0x1)
			//buffer_used_for_sample = 0;
		//else if(common_message_available_dirs & 0x2)
			//buffer_used_for_sample = 1;
		//else if(common_message_available_dirs & 0x4)
			//buffer_used_for_sample = 2;
		//else if(common_message_available_dirs & 0x8)
			//buffer_used_for_sample = 3;
		//else if(common_message_available_dirs & 0x10)
			//buffer_used_for_sample = 4;
		//else if(common_message_available_dirs & 0x20)
			//buffer_used_for_sample = 5;
		//else // cant get here
			//printf("CANT GET HERE %%%%\r\n");
		//
		///* COPY MESSAGE INTO GLOBAL ACCESS BUFFER */		
		//memcpy(	global_rx_buffer.buf,							/* destination pointer */
		//ir_rx[buffer_used_for_sample].buf+HEADER_LEN,	/* source pointer */
		//ir_rx[buffer_used_for_sample].data_len);		/* num bytes to copy */
		//global_rx_buffer.receivers_used = common_message_available_dirs;						// set flags of the receiver numbers which successfully received the message
		//global_rx_buffer.data_len = ir_rx[buffer_used_for_sample].data_len;						// to be copied from the header
		//global_rx_buffer.sender_ID = rx_msg_header[buffer_used_for_sample].sender_ID;			// to be copied from the header
		//global_rx_buffer.last_access_time = ir_rx[buffer_used_for_sample].last_access_time;		// grab the last access time as the received time
		//global_rx_buffer.initial_receive_time = ir_rx[buffer_used_for_sample].initial_use_time;	// grab the message arrival time as the received time
		//global_rx_buffer.read = 0;																// mark new message as not read yet
		//
		///* Clear the message buffers of those sharing the common read message */
		//
		//for(uint8_t dir = 0; dir < 6; dir++)
		//{
			//if(common_message_available_dirs & 1<<dir)	
			//{
				////printf("resetting DONE: %02X; %i\r\n", ir_rx[dir].ir_status, dir);
				//
				//// recall:
				////	IR_RX_STATUS_BUSY_bm			0x01
				////	IR_RX_STATUS_PACKET_DONE_bm		0x02
				////	IR_RX_STATUS_BYTE_DONE_bm		0x04	not used
				////	IR_RX_STATUS_RX_BYTE_bm			0x08
				////  IR_RX_STATUS_ECCERR_bm			0x10
				////  IR_RX_STATUS_LENERR_bm			0x20
				////	IR_RX_STATUS_TIMEOUT_bm			0x40
				////	IR_RX_STATUS_PROGRAMMING_bm		0x80
//
				//// what you may see:
				//// DONE: 09 = 0000 1001, signals incomplete
				////	09 = (IR_RX_STATUS_BUSY_bm, IR_RX_STATUS_RX_BYTE_bm)
//
				//// DONE: 0A = 0000 1010, currently signals complete												[GOOD]
				////	0A = (IR_RX_STATUS_PACKET_DONE_bm, IR_RX_STATUS_RX_BYTE_bm) "not busy"
//
				//// DONE: 0B = 0000 1011, previously signaled complete, busy flag left on is no longer correct
				////	0B = (IR_RX_STATUS_BUSY_bm, IR_RX_STATUS_PACKET_DONE_bm, IR_RX_STATUS_RX_BYTE_bm)
//
				//// DONE: 19 = 0001 1001, error correction probably set a programming flag
				////	19 = (IR_RX_STATUS_ECCERR_bm, IR_RX_STATUS_RX_BYTE_bm, IR_RX_STATUS_BUSY_bm)
//
				//// DONE: 29 = 0010 1001, found when the length in the header is inappropriate, message should be discarded completely
				////	29 = (IR_RX_STATUS_LENERR_bm, IR_RX_STATUS_BUSY_bm, IR_RX_STATUS_RX_BYTE_bm)
//
				//// DONE: 49 = 0100 1001, found when a byte was dropped causing a timeout error
				////	49 = (IR_RX_STATUS_TIMEOUT_bm, IR_RX_STATUS_BUSY_bm, IR_RX_STATUS_RX_BYTE_bm)
//
				//// DONE: 4A = 0100 1010, found when a new packet tried to 'append' to an unread packet, code should prevent this now
				////	4A = (IR_RX_STATUS_TIMEOUT_bm, IR_RX_STATUS_PACKET_DONE_bm, IR_RX_STATUS_RX_BYTE_bm) "not busy"
			//
				//// DONE: 99 = 1001 1001, error correction probably set a programming flag
				////	99 = (IR_RX_STATUS_PROGRAMMING_bm, IR_RX_STATUS_ECCERR_bm, IR_RX_STATUS_RX_BYTE_bm, IR_RX_STATUS_BUSY_bm)
//
				//// DONE: A9 = 1010 1001, error correction probably set a programming flag and also corrected length wrong
				////	A9 = (IR_RX_STATUS_PROGRAMMING_bm, IR_RX_STATUS_LENERR_bm, IR_RX_STATUS_RX_BYTE_bm, IR_RX_STATUS_BUSY_bm)
//
				//ir_reset_rx(dir);	
			//}
		//}
//
		//for(uint8_t dir = 0; dir < 6; dir++)
		//{
			////printf("resetting float check: %02X; %i\r\n", ir_rx[dir].ir_status, dir);
			//if(ir_rx[dir].ir_status == 0x09)	// 09 = 0000 1001, signals incomplete = (IR_RX_STATUS_BUSY_bm, IR_RX_STATUS_RX_BYTE_bm)
				//if(get_16bit_time() > ir_rx[dir].expect_complete_time)
				//{
					////printf("timeout, resetting\r\n");
					////printf("DIR %i {T: %u}\r\n",dir, get_16bit_time());
					////printf("float check timeout, initial use time: %u\r\n",ir_rx[dir].initial_use_time);
					////printf("float check timeout, last access time: %u\r\n",ir_rx[dir].last_access_time);
					////printf("float check timeout, expected completion time: %u\r\n",ir_rx[dir].expect_complete_time);
					//ir_reset_rx(dir);
					////printf("resetting float check: %02X; %i\r\n", ir_rx[dir].ir_status, dir);
				//}
		//}
	//
		//return 1;	// HEY! global_rx_buffer.read is set in case 2
	//}
//
	///* CASE 3 */
	//else
	//{
		//// - no new messages - //
		//return 0;
	//}
//}
//
//// DO NOT CALL THIS DIRECTLY
//// CALLED BY check_for_new_messages()
//// returns 0 if no direction has a packet ready
//// returns 0x00xxxxxx with bits set for buffers sharing the next message to take
//uint8_t check_buffers_for_packets()
//{
	//// here < (less than) is used to look for oldest messages first, and best_time is started high
	//
	//uint16_t best_time = 0XFFFF;		//TODO: if TIME is changed to uint32_t, then this needs to be updated 
	//uint8_t dir;
	//uint8_t dirs_with_approximate_best_time = 0;
	//uint8_t message_found = 0;
	//uint8_t data;
	//uint16_t computed_crc;
	//uint8_t good_crc;
//
	//for(dir = 0; dir < 6; dir++) // 1st for loop
	//{
		//if((ir_rx[dir].ir_status & IR_RX_STATUS_PACKET_DONE_bm)&&(ir_rx[dir].data_len > 0))
		//{
			///* calculate CRC of the message */
			////printf("Calculating CRC from Recieved.\r\n");
			////printf("%04X\r\n",rx_msg_header[dir].sender_ID);
			//computed_crc = (uint8_t)rx_msg_header[dir].sender_ID;	// include low byte of sender ID in crc (init crc with this)
			////printf("%02X\r\n ",computed_crc);
			//for (uint8_t i = HEADER_LEN; i < ir_rx[dir].data_len+HEADER_LEN; i++)
			//{
				//data = ir_rx[dir].buf[i];
				////printf("%02X ",data);
				//computed_crc = _crc16_update(computed_crc, data);
			//}
			//
			//if(rx_msg_header[dir].data_crc == (uint8_t)computed_crc)
			//{
				//good_crc = (uint8_t)computed_crc;
			//
				///* find the direction with the lowest access time (oldest message)*/
				////if(ir_rx[dir].last_access_time < best_time)
				//if(ir_rx[dir].initial_use_time < best_time)
				//{
					//best_time = ir_rx[dir].initial_use_time;
					//message_found = 1;
				//}
		//
				//// TODO: implement some way such that messages just bigger than T >= 0
				//// evaluate to being newer than messages tagged as T <= MAX_TIME
				//// right now, RvB droplets reboot at MAX_TIME so this will never be an issue until that is resolved
			//}
//
			//else
			//{
				//printf("CRC fail.\r\n");
				//// CRC fail!
				//// this message, despite all previous attempts at error checking, has an error
				//// it needs to be discarded
//
				//ir_reset_rx(dir);
			//}
		//}
//
	//} // end 1st for
//
	//if(!message_found)
	//{
		//// possibly also use this time to check for messages that have positive errors
//
		//for(dir = 0; dir < 6; dir++)
		//{
			//if(ir_rx[dir].ir_status & IR_RX_STATUS_ERR_gc)
			//{
				//if(get_16bit_time() > ir_rx[dir].expect_complete_time)
				//// don't reset too early, because its possible the message is still incoming, and we don't want to catch just the tail-end of it!
				//{
					////this is a good to use debug message
					////printf("resetting ERROR (no msg): %02X; %i {T: %u}\r\n", ir_rx[dir].ir_status, dir, get_16bit_time());
					///**/
//
					//ir_reset_rx(dir);
				//}
			//}
		//}
		//
		//// - got nothing - //
		//return 0;
	//}
//
	//for(dir = 0; dir < 6; dir++) // 2nd for loop
	//{
		//// if you are here, then a message has been found
		//// use this time to also check for messages that have positive errors
		//if(ir_rx[dir].ir_status & IR_RX_STATUS_ERR_gc)
		//{			
			//// this will reset "length errors" without regard to expected completion time
			//// if the error is a length error, most likely the expected completion time is wrong anyway
			//// we think resetting this error here will be okay because at least one of the directions got a good message
			//// so likely the transmission is over already
//
			///* this is a good to use debug message*/
			////printf("resetting ERROR: %02X; %i\r\n", ir_rx[dir].ir_status, dir);
			///**/
			//ir_reset_rx(dir);
		//}
//
		//if(ir_rx[dir].ir_status & IR_RX_STATUS_PACKET_DONE_bm)
		//{	
			///* find the directions with approximately the same access time (tdiff of 5 is permitted) */
//
			//if(ir_rx[dir].initial_use_time < best_time+5)	
			//{
				//// redundant check of the crc (messages that 'look complete' with incorrect crc's should be thrown out above)
				//if(rx_msg_header[dir].data_crc == good_crc)
				//{
					//dirs_with_approximate_best_time |= 1<<(dir);								
				//}
//
			//}			
		//}	
	//} // end 2nd for
	//
	//return dirs_with_approximate_best_time;
//}+
//
////
//// this checks all directions for busy
//// returns 0 if not busy
//// returns 1 if busy transmitting
//// returns 2 if busy receiving
//// if returning non-zero, busy_dir will be set to the direction that was found to be busy
//uint8_t check_ir_busy(uint8_t *busy_dir)
//{
	//for(uint8_t dir = 0; dir < 6; dir++) i
	//{
		//if(ir_tx[dir].ir_status & IR_TX_STATUS_BUSY_bm)
		//{
			//*busy_dir = dir;
			//return 1;	// transmitting
		//}
//
		//else if(ir_rx[dir].ir_status & IR_RX_STATUS_BUSY_bm)
		//{
			//*busy_dir = dir;
			//return 2;	// receiving
		//}
	//}
	//
	//return 0;
//}
//
//// this checks a single direction for busy
//// returns 0 if not busy
//// returns 1 if busy transmitting
//// returns 2 if busy receiving
//uint8_t check_direction_ir_busy(uint8_t dir)
//{
	//if(ir_tx[dir].ir_status & IR_TX_STATUS_BUSY_bm)
		//return 1;	// transmitting
//
	//else if(ir_rx[dir].ir_status & IR_RX_STATUS_BUSY_bm)
		//return 2;	// receiving
//
	//return 0;
//}

//void print_RX_buffer(uint8_t dir)
//{
	//printf("\r\ndir %i buffer: ", dir);
	//for(uint8_t i = 0; i < ir_rx[dir].size; i++)
		//printf("%02X ", ir_rx[dir].buf[i]);
	//printf("\r\r\n");
//}
//
//void print_RX_buffer_status(uint8_t dir)
//{
	//printf("\r\ndir %i buffer status: %02X ", dir, ir_rx[dir].ir_status);
	//if(ir_rx[dir].ir_status & IR_RX_STATUS_ERR_gc)
		//printf("ERROR\r\n");
	//else
		//printf("\r\n");
//}
//
//void erase_RX_buffer(uint8_t dir)
//{
	//for(uint8_t i = 0; i < ir_rx[dir].size; i++)
		//ir_rx[dir].buf[i] = 0;
//
	//printf("ERASED %i\r\n",dir);
//}

//// USER CODE CALLS THIS BEFORE ir_send()
//// this routine examines the IR RX and TX channels for busy status before transmitting
//// the RX channels need to be clear before transmitting, since transmitting will shut these down temporarily
//// returns 0 [BAD] if TX is busy, inspect this problem before proceeding with ir_send() 
//// returns 1 [GOOD] if all RX channels were ready with no work required, nearby IR likely quiet
//// returns 2 [GOOD] if all RX channels are now clear, nearby IR may be noisy, (some (possibly all) channels had to be reset)
//// returns 3 [GOOD] if all RX channels are now clear, nearby IR may be VERY noisy, (some (possibly all) channels had to be reset)
//uint8_t OK_to_send()
//{
	//uint8_t busy_dir = 255;
//
	//if(check_ir_busy(&busy_dir))
	//{
		//uint8_t busy_count = 0;
	//
		//while(check_ir_busy(&busy_dir))
		//{
			//busy_count++;
		//
			//if(1 == check_ir_busy(&busy_dir))
			//{
				//// busy is a result of TX
				//printf("IR busy: %i TX\r\n", busy_dir);
				//return 0;
			//}
			//
			//else if(2 == check_ir_busy(&busy_dir))
			//{
				///* this is a good to use debug message
				//printf("IR busy: %i RX\r\n", busy_dir);
				//printf("busy with curr_pos: %i, encoded_pos %i\r\n",ir_rx[busy_dir].curr_pos, ir_rx[busy_dir].curr_encoded_pos);
				//printf("last used: %u, ",get_16bit_time()-ir_rx[busy_dir].last_access_time);
				//*/
				//
				//if(get_16bit_time() > ir_rx[busy_dir].expect_complete_time)
				//{
					//// this may happen if the end of an inbound packet was dropped
					//
					///* this is a good to use debug message
					//printf("IR reset busy, SHOULD BE DONE (good)\r\n");
					//*/
					//ir_reset_rx(busy_dir);
				//}
			//
				//else if(get_16bit_time()-ir_rx[busy_dir].last_access_time > 20)		// TWENTY (milliseconds) <-  chosen arbitrarily, this is a hard-coded varaible
				//{
					//// this may happen if bytes are being received intermittently (bytes are being dropped from a weak signal)
					///* this is a good to use debug message
					//printf("IR reset busy, UNUSED (good)\r\n");
					//print_RX_buffer(busy_dir);
					//*/
					//ir_reset_rx(busy_dir);
				//}
//
				//_delay_ms(10);					// TEN (milliseconds) <-  chosen arbitrarily, this is a hard-coded varaible
				//if(busy_count > 20)				// TWENTY (count) <-  chosen arbitrarily, this is a hard-coded varaible
				//{
					///* this is a good to use debug message
					//printf("IR reset busy by COUNT\r\n");
					//*/
					//ir_reset_rx(busy_dir);
				//}
			//}
			//
		//} // END while
//
		//if(busy_count <= 20)					// TWENTY (count) <-  chosen arbitrarily, this is a hard-coded varaible
		//{
			///* this is a good to use debug message
			//printf("IR busy fixed itself (probably OK)\r\n");
			//*/
			//return 2;
		//}
//
		//else
		//{
			//printf("IR fixed busy !!! (probably bad)\r\n");
			//return 3;
		//}
	//}
//
	//else // not busy
		//return 1;
//}
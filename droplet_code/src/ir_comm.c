#include "ir_comm.h"

volatile uint16_t	cmd_length;
volatile char		cmd_buffer[BUFFER_SIZE];

inline void clear_ir_buffer(uint8_t dir)
{
	ir_rxtx[dir].status = 0;
	ir_rxtx[dir].data_length = 0;
	ir_rxtx[dir].curr_pos = 0;
	ir_rxtx[dir].target_ID = 0;
	ir_rxtx[dir].sender_ID = 0;
	
	channel[dir]->CTRLB |= USART_RXEN_bm; //this enables receive on the USART
}

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
void ir_comm_init()
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
	PORTD.DIRSET = PIN3_bm;					// DIR 2
	//PORTD.DIRCLR = PIN3_bm;					//undoes the previous line; sets the pin as output. (Just for the bugged samples)
	PORTE.DIRSET = PIN3_bm | PIN7_bm;		// DIR 3,4
	PORTF.DIRSET = PIN3_bm;					// DIR 5
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
	cmd_arrival_time=0;
	num_waiting_msgs=0;
	user_facing_messages_ovf=0;
	schedule_task(1000/IR_UPKEEP_FREQUENCY, perform_ir_upkeep, NULL);
	
}

void handle_cmd_wrapper()
{
	char local_msg_copy[cmd_length+1];	
	uint16_t local_msg_len;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)  // Disable interrupts just in case we get another command in the middle of copying this one.
	{
		memcpy(local_msg_copy, cmd_buffer, cmd_length+1);
		local_msg_len = cmd_length;
	}
	handle_serial_command(local_msg_copy, local_msg_len);
}

void perform_ir_upkeep()
{	
	uint16_t seen_crcs[6] = {0,0,0,0,0,0};
	uint8_t crc_seen;
	for(int8_t dir= 0; dir<6; dir++) //This first loop looks for a channel on which we got a good message.
	{
		if(ir_rxtx[dir].status&IR_STATUS_COMPLETE_bm)
		{
			crc_seen = 0;
			for(int8_t check_dir=(dir-1) ;  check_dir>=0 ; check_dir--)
				if(seen_crcs[check_dir]==ir_rxtx[dir].data_crc) crc_seen = 1;
			seen_crcs[dir] = ir_rxtx[dir].data_crc;

			if(crc_seen) clear_ir_buffer(dir);
			else if(ir_rxtx[dir].status&IR_STATUS_COMMAND_bm)
			{
				memcpy(cmd_buffer, (char*)ir_rxtx[dir].buf, ir_rxtx[dir].data_length);
				cmd_buffer[ir_rxtx[dir].data_length]='\0';
				cmd_length = ir_rxtx[dir].data_length;
				cmd_arrival_time = ir_rxtx[dir].last_byte;	//This is a 'global' value, referenced by other *.c files.
				cmd_sender_id = ir_rxtx[dir].sender_ID;		//This is a 'global' value, referenced by other *.c files.
				for(uint8_t other_dirs=dir ; other_dirs<6 ; other_dirs++) clear_ir_buffer(other_dirs); //clear the rest of the buffers first
				schedule_task(5, handle_cmd_wrapper, NULL);
				break; //commands can take awhile, so we gave up on the rest of these messages.
			}
			else //Normal message; add to message queue.
			{
				if(num_waiting_msgs>=MAX_USER_FACING_MESSAGES)
				{
					user_facing_messages_ovf = 1;
					num_waiting_msgs=0;
				}
				memcpy(msg_node[num_waiting_msgs].msg, (char*)ir_rxtx[dir].buf, ir_rxtx[dir].data_length);
				msg_node[num_waiting_msgs].msg[ir_rxtx[dir].data_length]='\0';
				msg_node[num_waiting_msgs].arrival_time = ir_rxtx[dir].last_byte;
				msg_node[num_waiting_msgs].arrival_dir = dir;
				msg_node[num_waiting_msgs].sender_ID = ir_rxtx[dir].sender_ID;
				msg_node[num_waiting_msgs].msg_length = ir_rxtx[dir].data_length;
				num_waiting_msgs++;
				clear_ir_buffer(dir);
			}
		}
	}
	
	schedule_task(1000/IR_UPKEEP_FREQUENCY, perform_ir_upkeep, NULL);
}

void send_msg(uint8_t dirs, char *data, uint8_t data_length)
{
	
	uint16_t crc = get_droplet_id();
	for(uint8_t dir=0; dir<6; dir++)
	{
		if(dirs&(1<<dir))
		{
			crc = _crc16_update(crc, (ir_rxtx[dir].status & IR_STATUS_COMMAND_bm));
			break;
		}	
	}
	for(uint8_t i=0; i<data_length; i++) crc = _crc16_update(crc, data[i]); //Calculate CRC of outbound message.
	
	for(uint8_t dir=0; dir<6; dir++)
	{
		if(dirs&(1<<dir))
		{
			ir_rxtx[dir].data_length = data_length;
			ir_rxtx[dir].data_crc = crc;
			ir_rxtx[dir].curr_pos = 0;
			ir_rxtx[dir].sender_ID = get_droplet_id();
			memcpy((char*)ir_rxtx[dir].buf, data, data_length);
			TCF2.CTRLB |= ir_carrier_bm[dir];		// Turn on carrier wave on port dir
		}
	}

	for(uint8_t dir=0; dir<6; dir++)
	{
		if(dirs&(1<<dir)){
			ir_rxtx[dir].last_byte = 0;
			channel[dir]->CTRLA |= USART_DREINTLVL_MED_gc;
		}
	}

	/* The whole transmission will now occur in interrupts. */
}

inline void all_ir_sends(uint8_t dirs, char* data, uint8_t data_length, uint16_t target, uint8_t cmd_flag)
{
	while(dirs)
	{
		for(uint8_t dir=0; dir<6; dir++) //first pass. send what you can.
		{
			if(dirs & (1<<dir))
			{
				if(!((ir_rxtx[dir].status & IR_STATUS_BUSY_bm) || (get_time() - ir_rxtx[dir].last_byte < IR_MSG_TIMEOUT)))
				{
					channel[dir]->CTRLB &= ~USART_RXEN_bm; //Disable receive messages on this channel while transmitting.
					ir_rxtx[dir].status = IR_STATUS_BUSY_bm;
					if(cmd_flag) ir_rxtx[dir].status |= IR_STATUS_COMMAND_bm;
					ir_rxtx[dir].target_ID=target;
					send_msg(1<<dir, data, data_length);		
					dirs&=(~(1<<dir));
				}
			}
		}
		if(dirs) delay_ms(rand_byte()%10);
	}
}

void ir_targeted_cmd(uint8_t dirs, char *data, uint16_t data_length, uint16_t target)
{
	all_ir_sends(dirs, data, data_length, target, 1);
}

void ir_cmd(uint8_t dirs, char *data, uint16_t data_length)
{	
	all_ir_sends(dirs, data, data_length, 0, 1);
}

void ir_targeted_send(uint8_t dirs, char *data, uint16_t data_length, uint16_t target)
{
	all_ir_sends(dirs, data, data_length, target, 0);
}

void ir_send(uint8_t dirs, char *data, uint8_t data_length)
{
	all_ir_sends(dirs, data, data_length, 0, 0);
}

// To be called from interrupt handler only. Do not call.
void ir_receive(uint8_t dir)
{
	uint8_t in_byte = channel[dir]->DATA;				// Some data just came in
	//if(dir>2) printf("%hhu:%02x ", dir, in_byte); //Used for debugging - prints raw bytes as we get them.	

	uint32_t now = get_time();
	if(now-ir_rxtx[dir].last_byte > IR_MSG_TIMEOUT) ir_rxtx[dir].curr_pos = 0;
	ir_rxtx[dir].last_byte = now;
	switch(ir_rxtx[dir].curr_pos)
	{
		case HEADER_POS_MSG_LENGTH:		
			if(in_byte&DATA_LEN_CMD_bm) ir_rxtx[dir].status|=IR_STATUS_COMMAND_bm; 
										ir_rxtx[dir].data_length	= in_byte&DATA_LEN_VAL_bm;	
																								break;
		case HEADER_POS_CRC_LOW:		ir_rxtx[dir].data_crc		= (uint16_t)in_byte;		break;
		case HEADER_POS_CRC_HIGH:		ir_rxtx[dir].data_crc	   |= (((uint16_t)in_byte)<<8); break;
		case HEADER_POS_SENDER_ID_LOW:	ir_rxtx[dir].sender_ID		= (uint16_t)in_byte;		break;
		case HEADER_POS_SENDER_ID_HIGH:	
										ir_rxtx[dir].sender_ID	   |= (((uint16_t)in_byte)<<8);
										ir_rxtx[dir].calc_crc		= _crc16_update(ir_rxtx[dir].sender_ID, ir_rxtx[dir].status & IR_STATUS_COMMAND_bm);
																								break;
		case HEADER_POS_TARGET_ID_LOW:  ir_rxtx[dir].target_ID		= (uint16_t)in_byte;		break;
		case HEADER_POS_TARGET_ID_HIGH: ir_rxtx[dir].target_ID	   |= (((uint16_t)in_byte)<<8); break;
		default: 
			ir_rxtx[dir].buf[ir_rxtx[dir].curr_pos-HEADER_LEN] = in_byte;
			ir_rxtx[dir].calc_crc = _crc16_update(ir_rxtx[dir].calc_crc, in_byte);
	}
	ir_rxtx[dir].curr_pos++;
	if(ir_rxtx[dir].curr_pos>=(ir_rxtx[dir].data_length+HEADER_LEN))
	{	
		if(ir_rxtx[dir].calc_crc!=ir_rxtx[dir].data_crc)							ir_rxtx[dir].curr_pos = 0;	//crc check failed.
		else if(ir_rxtx[dir].target_ID && ir_rxtx[dir].target_ID!=get_droplet_id()) ir_rxtx[dir].curr_pos = 0;	//msg targeted, but not to me.
		else if(ir_rxtx[dir].sender_ID == get_droplet_id())							ir_rxtx[dir].curr_pos = 0;  //ignore a message if it is from me. Silly reflections.
		else
		{
			ir_rxtx[dir].status |= IR_STATUS_COMPLETE_bm;
			ir_rxtx[dir].status |= IR_STATUS_BUSY_bm; //mark as busy so we don't overwrite it.
			channel[dir]->CTRLB &= ~USART_RXEN_bm; //Disable receiving messages on this channel until the message has been processed.
		}
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
		clear_ir_buffer(dir);
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
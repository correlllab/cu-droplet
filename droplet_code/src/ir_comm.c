#include "ir_comm.h"
#include "rgb_led.h"

//#define IR_IS_SPECIAL 1

volatile uint16_t	cmd_length;
volatile char		cmd_buffer[BUFFER_SIZE];

void clear_ir_buffer(uint8_t dir)
{
	ir_rxtx[dir].data_crc		= 0;
	ir_rxtx[dir].sender_ID		= 0;
	ir_rxtx[dir].target_ID		= 0;	
	ir_rxtx[dir].curr_pos		= 0;
	ir_rxtx[dir].calc_crc		= 0;
	ir_rxtx[dir].data_length	= 0;	
	
	ir_rxtx[dir].status			= 0;	
	
	channel[dir]->CTRLB |= USART_RXEN_bm; //this enables receive on the USART
}

/* Hardware addresses for the port pins with the carrier wave */
uint8_t ir_carrier_bm[] = { PIN0_bm, PIN1_bm, PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm };

void ir_comm_init()
{
	/* Initialize UARTs */
	// RX pins as inputs:
	PORTC.DIRCLR = PIN2_bm | PIN6_bm;		// DIR 0,1							
	PORTD.DIRCLR = PIN2_bm;					// DIR 2
	PORTE.DIRCLR = PIN2_bm | PIN6_bm;		// DIR 3,4
	PORTF.DIRCLR = PIN2_bm;					// DIR 5

	for (uint8_t i = 0; i < 6; i++)
	{
		channel[i]->CTRLA = (uint8_t) USART_RXCINTLVL_MED_gc | USART_TXCINTLVL_MED_gc;		// Set USART as med-level interrupts
		channel[i]->CTRLC = (uint8_t) USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;		// 8 bits, no parity
		
		channel[i]->BAUDCTRLA = 0b01110000; channel[i]->BAUDCTRLB = 0b00000010;	//3200 baud - we have an upper bound of 3800 baud because of the 38KHz carrier wave. The IR Receiver's data sheets say that frequency of the data should be a tenth or less than that of the carrier.
		//channel[i]->BAUDCTRLA = 0b11000000; channel[i]->BAUDCTRLB = 0b00000100; //9600 baud - this would be the upper bound if we switched to TSDP34156-ND.
		
		channel[i]->CTRLB |= USART_RXEN_bm;		// Enable communication
		channel[i]->CTRLB |= USART_TXEN_bm;
	}
	curr_ir_power=0;	
	set_all_ir_powers(256);
	for(uint8_t dir=0; dir<6; dir++) clear_ir_buffer(dir); //this initializes the buffer's values to 0.
	cmd_arrival_time=0;
	num_waiting_msgs=0;
	user_facing_messages_ovf=0;

	schedule_periodic_task(1000/IR_UPKEEP_FREQUENCY, perform_ir_upkeep, NULL);
}

void handle_cmd_wrapper()
{
	char local_msg_copy[cmd_length+1];	
	uint16_t local_msg_len;
	//printf("\tIn handle_cmd_wrapper.\r\n");
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)  // Disable interrupts just in case we get another command in the middle of copying this one.
	{
		memcpy(local_msg_copy, (const void*)cmd_buffer, cmd_length+1);
		local_msg_len = cmd_length;
	}
	handle_serial_command(local_msg_copy, local_msg_len);
}

void perform_ir_upkeep()
{	
	uint16_t seen_crcs[6] = {0,0,0,0,0,0};
	uint8_t crc_seen;
	int8_t check_dir;
	int8_t dir;
	for(dir= 0; dir<6; dir++) //This first loop looks for a channel on which we got a good message.
	{
		if(ir_rxtx[dir].status&IR_STATUS_COMPLETE_bm)
		{
			crc_seen = 0;
			for(check_dir=(dir-1) ;  check_dir>=0 ; check_dir--)
				if(seen_crcs[check_dir]==ir_rxtx[dir].data_crc) crc_seen = 1;
			seen_crcs[dir] = ir_rxtx[dir].data_crc;

			if(crc_seen) clear_ir_buffer(dir);
			else //Normal message; add to message queue.
			{				
				if(num_waiting_msgs>=MAX_USER_FACING_MESSAGES)
				{
					user_facing_messages_ovf = 1;
					num_waiting_msgs=0;
				}
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
					if(ir_rxtx[dir].data_length==0)
					{
						printf_P(PSTR("ERROR: Message length 0 in perform_ir_upkeep.\r\n"));
					}
					memcpy((void *)msg_node[num_waiting_msgs].msg, (char*)ir_rxtx[dir].buf, ir_rxtx[dir].data_length);
					msg_node[num_waiting_msgs].msg[ir_rxtx[dir].data_length]='\0';
					msg_node[num_waiting_msgs].arrival_time = ir_rxtx[dir].last_byte;
					msg_node[num_waiting_msgs].arrival_dir = dir;
					msg_node[num_waiting_msgs].sender_ID = ir_rxtx[dir].sender_ID;
					msg_node[num_waiting_msgs].msg_length = ir_rxtx[dir].data_length;
				}
				num_waiting_msgs++;
				clear_ir_buffer(dir);
			}
		}
	}
}

void send_msg(uint8_t dirs, char *data, uint8_t data_length)
{
	if(data_length>IR_BUFFER_SIZE) printf_P(PSTR("ERROR: Message exceeds IR_BUFFER_SIZE.\r\n"));
	
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
			ir_rxtx[dir].status |= IR_STATUS_TRANSMITTING_bm;
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

#ifdef IR_IS_SPECIAL
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
					wait_for_ir(1<<dir);
					dirs&=(~(1<<dir));
				}
			}
		}
		if(dirs) delay_ms(rand_byte()%10);
	}
}
#endif

///*
 //* MORE COMPLICATED ALL_IR_SENDS, allows two channels on opposite sides to send at the same time.
 //*/
//inline void all_ir_sends(uint8_t dirs_to_go, char* data, uint8_t data_length, uint16_t target, uint8_t cmd_flag)
//{
	//uint32_t start=get_time();	
	//uint8_t busy_channel=0;
	//while(dirs_to_go)
	//{
		//uint8_t sending_dirs = 0;
		//uint8_t sendable_dirs = dirs_to_go;
		//
		//busy_channel=0;
		//for(uint8_t dir=0; dir<6; dir++) //first pass. send what you can.
		//{		
			//if(sendable_dirs & (1<<dir))
			//{
				//if(!((ir_rxtx[dir].status & IR_STATUS_BUSY_bm) || (get_time() - ir_rxtx[dir].last_byte < IR_MSG_TIMEOUT)))
				//{
					//channel[dir]->CTRLB &= ~USART_RXEN_bm; //Disable receive messages on this channel while transmitting.
					//ir_rxtx[dir].status = IR_STATUS_BUSY_bm;
					//if(cmd_flag) ir_rxtx[dir].status |= IR_STATUS_COMMAND_bm;
					//ir_rxtx[dir].target_ID=target;
					//sending_dirs  |=1<<dir;
					//uint8_t left_m	=1<<((dir-1)%6);
					//uint8_t right_m	=1<<((dir+1)%6);
					//sendable_dirs &=~left_m;
					//sendable_dirs &=~right_m;
					////sendable_dirs &=(~(1<<((dir+3)%6)); //opposite direction.
				//}
				//else busy_channel=1;
			//}	
		//}
		//if(sending_dirs)
		//{
			//send_msg(sending_dirs, data, data_length);
			////printf("\t%hX:\t%lu\r\n",sending_dirs, get_time()-start);			
			//wait_for_ir(sending_dirs);
			//dirs_to_go&=(~sending_dirs);
		//}
		//else if(busy_channel) delay_ms(rand_byte()%10);
	//}
//}

#ifndef IR_IS_SPECIAL
//SIMPLEST POSSIBLE ALL_IR_SENDS.
inline void all_ir_sends(uint8_t dirs_to_go, char* data, uint8_t data_length, uint16_t target, uint8_t cmd_flag)
{
	
	if(!wait_for_ir(dirs_to_go)) return;
	for(uint8_t dir
	=0;dir<6;dir++)
	{
		if(dirs_to_go&(1<<dir))
		{
			channel[dir]->CTRLB &= ~USART_RXEN_bm;
			ir_rxtx[dir].status = IR_STATUS_BUSY_bm;	
			if(cmd_flag) ir_rxtx[dir].status |= IR_STATUS_COMMAND_bm;
			ir_rxtx[dir].target_ID=target;
		}
	}
	send_msg(dirs_to_go, data, data_length);
}
#endif


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
	//if(dir==2)printf("%02hhx ", in_byte); //Used for debugging - prints raw bytes as we get them.	

	uint32_t now = get_time();
	if(now-ir_rxtx[dir].last_byte > IR_MSG_TIMEOUT)	clear_ir_buffer(dir);
	ir_rxtx[dir].last_byte = now;
	switch(ir_rxtx[dir].curr_pos)
	{
		case HEADER_POS_SENDER_ID_LOW:	ir_rxtx[dir].sender_ID		= (uint16_t)in_byte;		break;
		case HEADER_POS_SENDER_ID_HIGH:
										ir_rxtx[dir].sender_ID	   |= (((uint16_t)in_byte)<<8);
										ir_rxtx[dir].calc_crc		= ir_rxtx[dir].sender_ID;
																								break;		
		case HEADER_POS_MSG_LENGTH:		
			if(in_byte&DATA_LEN_CMD_bm) ir_rxtx[dir].status		   |= IR_STATUS_COMMAND_bm;
										ir_rxtx[dir].calc_crc		= _crc16_update(ir_rxtx[dir].calc_crc, ir_rxtx[dir].status & IR_STATUS_COMMAND_bm); 
										ir_rxtx[dir].data_length	= in_byte&DATA_LEN_VAL_bm;	
																								break;
		case HEADER_POS_CRC_LOW:		ir_rxtx[dir].data_crc		= (uint16_t)in_byte;		break;
		case HEADER_POS_CRC_HIGH:		ir_rxtx[dir].data_crc	   |= (((uint16_t)in_byte)<<8); break;

		case HEADER_POS_TARGET_ID_LOW:  ir_rxtx[dir].target_ID		= (uint16_t)in_byte;		break;
		case HEADER_POS_TARGET_ID_HIGH: ir_rxtx[dir].target_ID	   |= (((uint16_t)in_byte)<<8); break;
		default: 
			ir_rxtx[dir].buf[ir_rxtx[dir].curr_pos-HEADER_LEN] = in_byte;
			ir_rxtx[dir].calc_crc = _crc16_update(ir_rxtx[dir].calc_crc, in_byte);
	}
	ir_rxtx[dir].curr_pos++;
	if(ir_rxtx[dir].curr_pos>=(ir_rxtx[dir].data_length+HEADER_LEN))
	{			
		if((ir_rxtx[dir].calc_crc!=ir_rxtx[dir].data_crc)||ir_rxtx[dir].calc_crc==0)clear_ir_buffer(dir); //crc check failed.
		else if(ir_rxtx[dir].target_ID && ir_rxtx[dir].target_ID!=get_droplet_id()) clear_ir_buffer(dir); //msg targeted, but not to me.
		else if(ir_rxtx[dir].sender_ID == get_droplet_id())							clear_ir_buffer(dir); //ignore a message if it is from me. Silly reflections.
		else
		{
			if(ir_rxtx[dir].status & IR_STATUS_COMMAND_bm)
			{
				if(ir_rxtx[dir].data_length==0)
				{
					#ifdef SYNCHRONIZED
						update_firefly_counter();
					#endif
					for(uint8_t other_dir=0;other_dir<6;other_dir++) clear_ir_buffer(other_dir);
				}
				
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
					memcpy((void *)cmd_buffer, (char*)ir_rxtx[dir].buf, ir_rxtx[dir].data_length);
					cmd_buffer[ir_rxtx[dir].data_length]='\0';
					cmd_length = ir_rxtx[dir].data_length;
					cmd_arrival_time = ir_rxtx[dir].last_byte;	//This is a 'global' value, referenced by other *.c files.
					cmd_sender_id = ir_rxtx[dir].sender_ID;		//This is a 'global' value, referenced by other *.c file.s
				}
				for(uint8_t other_dir=0;other_dir<6;other_dir++) clear_ir_buffer(other_dir);
				//{
					//if(ir_rxtx[other_dir].sender_ID==ir_rxtx[dir].sender_ID) clear_ir_buffer(other_dir);
				//}
				schedule_task(10, handle_cmd_wrapper, NULL);
				//printf("Got cmd from %X.\r\n", cmd_sender_id);
			}
			else
			{
				if(ir_rxtx[dir].data_length==0)
					clear_ir_buffer(dir);
				ir_rxtx[dir].status |= IR_STATUS_COMPLETE_bm;
				ir_rxtx[dir].status |= IR_STATUS_BUSY_bm; //mark as busy so we don't overwrite it.
				channel[dir]->CTRLB &= ~USART_RXEN_bm; //Disable receiving messages on this channel until the message has been processed.
			}

			//printf("\r\n");
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
		case HEADER_POS_CRC_LOW:		next_byte  = (uint8_t)(ir_rxtx[dir].data_crc&0xFF);			break;
		case HEADER_POS_CRC_HIGH:		next_byte  = (uint8_t)((ir_rxtx[dir].data_crc>>8)&0xFF);	break;
		case HEADER_POS_SENDER_ID_LOW:  next_byte  = (uint8_t)(ir_rxtx[dir].sender_ID&0xFF);		break;
		case HEADER_POS_SENDER_ID_HIGH: next_byte  = (uint8_t)((ir_rxtx[dir].sender_ID>>8)&0xFF);	break;
		case HEADER_POS_TARGET_ID_LOW:  next_byte  = (uint8_t)(ir_rxtx[dir].target_ID&0xFF);		break;
		case HEADER_POS_TARGET_ID_HIGH: next_byte  = (uint8_t)((ir_rxtx[dir].target_ID>>8)&0xFF);	break;
		default: next_byte = ir_rxtx[dir].buf[ir_rxtx[dir].curr_pos - HEADER_LEN];
	}
	channel[dir]->DATA = next_byte;
	ir_rxtx[dir].curr_pos++;
	/* CHECK TO SEE IF MESSAGE IS COMPLETE */
	if(ir_rxtx[dir].curr_pos >= (ir_rxtx[dir].data_length+HEADER_LEN))
	{
		clear_ir_buffer(dir);
		channel[dir]->CTRLA &= ~USART_DREINTLVL_gm; //Turn off interrupt things.
		//printf("\r\n");
	}

}

void ir_remote_send(uint8_t dir, uint16_t data)
{	
	//printf("In ir_remote_send.\r\n");
	//wait_for_ir(1<<dir);
	//printf("ir_remote_send, post wait_for_ir.\r\n");
	//ir_rxtx[dir].status = IR_STATUS_BUSY_bm;	
	channel[dir]->CTRLB &= ~USART_RXEN_bm;
	channel[dir]->CTRLB &= ~USART_TXEN_bm;
	//printf("Sending:\t");
	TCF2.CTRLB |= ir_carrier_bm[dir];
	PORT_t* port = 0;
	if((dir==0)|(dir==1))		port=&PORTC;
	else if(dir==2)			port=&PORTD;
	else if((dir==3)|(dir==4))	port=&PORTE;
	else if(dir==5)			port=&PORTF;
	uint8_t pin_mask=0;
	if((dir==0)|(dir==2)|(dir==3)|(dir==5)) pin_mask=PIN3_bm;
	else if((dir==1)|(dir==4))			pin_mask=PIN7_bm;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		port->DIRSET = pin_mask;	
		//start bit
		port->OUTCLR = pin_mask;				delay_us(5000);
		port->OUTSET = pin_mask;				delay_us(5000);
		//send E0E0:
		for(uint8_t i=0;i<16;i++)
		{
			port->OUTCLR = pin_mask;	delay_us(560);		
			if((0xE0E0<<i)&0x8000){	port->OUTSET = pin_mask;	delay_us(1600);}
			else{					port->OUTSET = pin_mask;	delay_us(560);	}
		}
		//send data:
		for(uint8_t i=0;i<16;i++)
		{
			port->OUTCLR = pin_mask;	delay_us(560);		
			if((data<<i)&0x8000){		port->OUTSET = pin_mask;	delay_us(1600);}
			else{						port->OUTSET = pin_mask;	delay_us(560);	}
		}	
		//stop bit
		port->OUTCLR = pin_mask;		delay_us(560);
		port->OUTSET = pin_mask;
	}
	channel[dir]->CTRLB |= USART_TXEN_bm;
	ir_transmit_complete(dir);
	//printf("End of ir_remote_send.\r\n");	
}

//void ir_remote_send(uint8_t dir, uint16_t data)
//{
	//printf("In ir_remote_send.\r\n");
	//wait_for_ir(1<<dir);
	//printf("ir_remote_send, post wait_for_ir.\r\n");
	//ir_rxtx[dir].status = IR_STATUS_BUSY_bm;
	//channel[dir]->CTRLB &= ~USART_RXEN_bm;
	//channel[dir]->CTRLB &= ~USART_TXEN_bm;
	////printf("Sending:\t");
	//TCF2.CTRLB |= ir_carrier_bm[dir];
	//
	//ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	//{
		//PORTE.DIRSET = PIN7_bm;
		////start bit
		//PORTE.OUTCLR = PIN7_bm;				delay_us(5000);
		//PORTE.OUTSET = PIN7_bm;				delay_us(5000);
		////send E0E0:
		//for(uint8_t i=0;i<16;i++)
		//{
			//PORTE.OUTCLR = PIN7_bm;	delay_us(560);
			//if((0xE0E0<<i)&0x8000){	PORTE.OUTSET = PIN7_bm;	delay_us(1600);}
			//else{					PORTE.OUTSET = PIN7_bm;	delay_us(560);	}
		//}
		////send data:
		//for(uint8_t i=0;i<16;i++)
		//{
			//PORTE.OUTCLR = PIN7_bm;	delay_us(560);
			//if((data<<i)&0x8000){		PORTE.OUTSET = PIN7_bm;	delay_us(1600);}
			//else{						PORTE.OUTSET = PIN7_bm;	delay_us(560);	}
		//}
		////stop bit
		//PORTE.OUTCLR = PIN7_bm;		delay_us(560);
		//PORTE.OUTSET = PIN7_bm;
	//}
	//channel[dir]->CTRLB |= USART_TXEN_bm;
	//ir_transmit_complete(dir);
	//printf("End of ir_remote_send.\r\n");
//}

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

uint8_t wait_for_ir(uint8_t dirs)
{
	uint8_t r = get_red_led();
	uint8_t g = get_green_led();
	uint8_t b = get_blue_led();
	set_rgb(255, 0, 255);
	uint32_t time_wait_start = get_time();
	uint8_t busy;
	do
	{
		busy=0;
		for(uint8_t dir=0; dir<6; dir++)
		{
			if(dirs&(1<<dir))
			{
				if(ir_rxtx[dir].status & IR_STATUS_TRANSMITTING_bm)
				{
					busy=1;
				}
				//else
				//{
					//clear_ir_buffer(dir);
				//}
			}
		}
		delay_us(100);	
		//if(busy&&task_list_check())
		//{
			//printf("!!!!\r\n!!!!\r\nFrom wait_for_ir (%ld): should perform task_list_cleanup.\r\n!!!!\r\n!!!!\r\n", get_time()-(task_list->scheduled_time));
			////task_list_cleanup(); //if the scheduled time for the current task is past and we're busy, perform task list cleanup	
		//}
	} while (busy&&((get_time()-time_wait_start)<MAX_WAIT_FOR_IR_TIME));
	set_rgb(r, g, b);
	if((get_time()-time_wait_start)>=MAX_WAIT_FOR_IR_TIME) return 0;
	return 1;
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
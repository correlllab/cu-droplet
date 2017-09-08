#include "ir_comm.h"
#include "rgb_led.h"

static volatile uint8_t processing_cmd;
static volatile uint8_t processing_ffsync_flag;

static void clear_ir_buffer(uint8_t dir);
static void perform_ir_upkeep(void);
static uint8_t all_ir_sends(uint8_t dirs, char* data, uint8_t data_length, id_t target, uint8_t cmd_flag);
static uint8_t all_hp_ir_cmds(uint8_t dirs, char* data, uint8_t data_length, id_t target);
static void ir_receive(uint8_t dir); //Called by Interrupt Handler Only
static void received_ir_cmd(uint8_t dir);
static void received_rnb_r(uint8_t delay, id_t senderID, uint32_t last_byte);
static void received_ir_sync(uint8_t delay, id_t senderID);
static void ir_transmit(uint8_t dir);
//static void ir_remote_send(uint8_t dir, uint16_t data);
static void ir_transmit_complete(uint8_t dir);

static volatile uint16_t	cmd_length;
static volatile char		cmd_buffer[SRL_BUFFER_SIZE];
/* Hardware addresses for the port pins with the carrier wave */
static uint8_t ir_carrier_bm[] = { PIN0_bm, PIN1_bm, PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm };

//#define HARDCORE_DEBUG_DIR 1

static void clear_ir_buffer(uint8_t dir){
	#ifdef AUDIO_DROPLET
		ir_sense_channels[dir]->INTCTRL = ADC_CH_INTLVL_OFF_gc;
	#endif
	#ifdef HARDCORE_DEBUG_DIR
		if(dir==HARDCORE_DEBUG_DIR) printf("\r\n");
	#endif
	ir_rxtx[dir].data_crc		= 0;
	ir_rxtx[dir].sender_ID		= 0;
	
	ir_rxtx[dir].target_ID		= 0;	
	ir_rxtx[dir].curr_pos		= 0;
	ir_rxtx[dir].calc_crc		= 0;
	ir_rxtx[dir].data_length	= 0;	
	ir_rxtx[dir].inc_dir 		= 0;
	
	ir_rxtx[dir].status			= 0;	
	
	channel[dir]->CTRLB |= USART_RXEN_bm; //this enables receive on the USART

}

void ir_comm_init(){
	/* Initialize UARTs */
	// RX pins as inputs:
	PORTC.DIRCLR = PIN2_bm | PIN6_bm;		// DIR 0,1							
	PORTD.DIRCLR = PIN2_bm;					// DIR 2
	PORTE.DIRCLR = PIN2_bm | PIN6_bm;		// DIR 3,4
	PORTF.DIRCLR = PIN2_bm;					// DIR 5

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		hp_ir_block_bm=0x3F;
	}

	for (uint8_t i = 0; i < 6; i++){
		channel[i]->CTRLA = (uint8_t) USART_RXCINTLVL_MED_gc | USART_TXCINTLVL_MED_gc;		// Set USART as med-level interrupts
		channel[i]->CTRLC = (uint8_t) USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;		// 8 bits, no parity
		
		channel[i]->BAUDCTRLA = 0b01110000; channel[i]->BAUDCTRLB = 0b00000010;	//3200 baud - we have an upper bound of 3800 baud because of the 38KHz carrier wave. The IR Receiver's data sheets say that frequency of the data should be a tenth or less than that of the carrier.
		//channel[i]->BAUDCTRLA = 0b11000000; channel[i]->BAUDCTRLB = 0b00000100; //9600 baud - this would be the upper bound if we switched to TSDP34156-ND.
		
		channel[i]->CTRLB |= USART_RXEN_bm;		// Enable communication
		channel[i]->CTRLB |= USART_TXEN_bm;
	}
	#ifdef AUDIO_DROPLET
		PORTC.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;
		PORTC.PIN6CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;
		PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;
		PORTE.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;
		PORTE.PIN6CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;
		PORTF.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;
		EVSYS.CH1MUX = EVSYS_CHMUX_PORTC_PIN2_gc;
		EVSYS.CH2MUX = EVSYS_CHMUX_PORTC_PIN6_gc;
		EVSYS.CH3MUX = EVSYS_CHMUX_PORTD_PIN2_gc;
		EVSYS.CH5MUX = EVSYS_CHMUX_PORTE_PIN2_gc;
		EVSYS.CH6MUX = EVSYS_CHMUX_PORTE_PIN6_gc;
		EVSYS.CH7MUX = EVSYS_CHMUX_PORTF_PIN2_gc;
	#endif	

	for(uint8_t dir=0; dir<6; dir++) clear_ir_buffer(dir); //this initializes the buffer's values to 0.
	cmd_arrival_time=0;
	num_waiting_msgs=0;
	user_facing_messages_ovf=0;
	processing_cmd = 0;
	processing_ffsync_flag = 0;

	schedule_periodic_task(1000/IR_UPKEEP_FREQUENCY, perform_ir_upkeep, NULL);
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		hp_ir_block_bm = 0;
	}
}

void handle_cmd_wrapper(){
	char local_msg_copy[cmd_length+1];	
	uint16_t local_msg_len;
	//printf("\tIn handle_cmd_wrapper.\r\n");
	//Nothing should touch the cmd_buffer and stuff as long as processing_cmd is still 1.
	memcpy(local_msg_copy, (const void*)cmd_buffer, cmd_length+1);
	local_msg_len = cmd_length;
	handle_serial_command(local_msg_copy, local_msg_len);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		processing_cmd = 0;
	}
}

static void perform_ir_upkeep(void){
	uint16_t seen_crcs[6] = {0,0,0,0,0,0};
	uint8_t crc_seen;
	int8_t check_dir;
	int8_t dir;
	for(dir= 0; dir<6; dir++){ //This first loop looks for a channel on which we got a good message.
		if(ir_is_busy(1<<dir)>=8){ //This dir is busy with some low level high priority task; skip it.
			continue;
		}
		if(ir_rxtx[dir].status&IR_STATUS_COMPLETE_bm){
			crc_seen = 0;
			for(check_dir=(dir-1) ;  check_dir>=0 ; check_dir--)
				if(seen_crcs[check_dir]==ir_rxtx[dir].data_crc) crc_seen = 1;
			seen_crcs[dir] = ir_rxtx[dir].data_crc;

			if(crc_seen) clear_ir_buffer(dir);
			else{ //Normal message; add to message queue.		
				if(num_waiting_msgs>=MAX_USER_FACING_MESSAGES){
					user_facing_messages_ovf = 1;
					num_waiting_msgs=0;
				}
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
					if(ir_rxtx[dir].data_length==0){
						printf_P(PSTR("ERROR: Message length 0 in perform_ir_upkeep.\r\n"));
					}
					memcpy((void *)msg_node[num_waiting_msgs].msg, (char*)ir_rxtx[dir].buf, ir_rxtx[dir].data_length);
					msg_node[num_waiting_msgs].msg[ir_rxtx[dir].data_length]='\0';
					msg_node[num_waiting_msgs].arrival_time = ir_rxtx[dir].last_byte;
					msg_node[num_waiting_msgs].arrival_dir = dir;
					msg_node[num_waiting_msgs].sender_ID = ir_rxtx[dir].sender_ID;
					msg_node[num_waiting_msgs].msg_length = ir_rxtx[dir].data_length;
					msg_node[num_waiting_msgs].wasTargeted = !!(ir_rxtx[dir].status&IR_STATUS_TARGETED_bm);
					if(msg_node[num_waiting_msgs].msg_length > IR_BUFFER_SIZE){
						printf_P(PSTR("ERROR! Message too long?\r\n"));
					}
					num_waiting_msgs++;
				}

				clear_ir_buffer(dir);
			}
		}
	}
}

void send_msg(uint8_t dirs, char *data, uint8_t data_length, uint8_t hp_flag){
	if(data_length>IR_BUFFER_SIZE) printf_P(PSTR("ERROR: Message exceeds IR_BUFFER_SIZE.\r\n"));
	
	uint16_t crc = get_droplet_id();
	for(uint8_t dir=0; dir<6; dir++){
		if(dirs&(1<<dir)){			
			crc = _crc16_update(crc, (ir_rxtx[dir].status & IR_STATUS_CRC_BITS_bm));
			crc = _crc16_update(crc, ir_rxtx[dir].target_ID);
			break;
		}	
	}

	for(uint8_t i=0; i<data_length; i++) crc = _crc16_update(crc, data[i]); //Calculate CRC of outbound message.
	
	for(uint8_t dir=0; dir<6; dir++){
		if(dirs&(1<<dir)){
			ir_rxtx[dir].status |= IR_STATUS_TRANSMITTING_bm;
			ir_rxtx[dir].data_length = data_length;
			ir_rxtx[dir].data_crc = crc;
			ir_rxtx[dir].curr_pos = 0;
			ir_rxtx[dir].sender_ID = get_droplet_id();
			memcpy((char*)ir_rxtx[dir].buf, data, data_length);
			TCF2.CTRLB |= ir_carrier_bm[dir];		// Turn on carrier wave on port dir
		}
	}

	for(uint8_t dir=0; dir<6; dir++){
		if(dirs&(1<<dir)){
			ir_rxtx[dir].last_byte = 0;
			if(hp_flag){
				channel[dir]->CTRLA |= USART_DREINTLVL_HI_gc;
			}else{
				channel[dir]->CTRLA |= USART_DREINTLVL_MED_gc;			
			}
		}
	}

	/* The whole transmission will now occur in interrupts. */
}

/*
 * This function returns '0' if no message was sent because the channels were busy, and '1' if it was successful 
 * in claiming channels and starting the message send process. Note that this function returning '1' doesn't
 * guarantee a successful transmission, as it's still possible for something to go wrong with the send.
 */
static uint8_t all_ir_sends(uint8_t dirs, char* data, uint8_t data_length, id_t target, uint8_t cmd_flag){
	uint8_t busy_status = ir_is_busy(dirs);
	if(busy_status>1){
        printf_P(PSTR("Aborting IR send [%02hx] while trying: "), busy_status);
		uint8_t text = 1;
		for(uint8_t i=0;i<data_length;i++){
			if( (data[i] < 32) || (data[i] > 126) ){ //printable ASCII range.
				text = 0;
				break;
			}
		}
		for(uint8_t i=0;i<data_length;i++){
			if(!text){
				printf("%02hX ", data[i]);
			}else{
				printf("%c", data[i]);
			}
		}
		printf("\r\n");
        return 0;
    }        
	for(uint8_t dir=0;dir<6;dir++){
		if(dirs&(1<<dir)){		
			channel[dir]->CTRLB &= ~USART_RXEN_bm;
			ir_rxtx[dir].status = IR_STATUS_BUSY_bm;
			if(cmd_flag) ir_rxtx[dir].status |= IR_STATUS_COMMAND_bm;
			ir_rxtx[dir].target_ID=target;
		}
	}
	send_msg(dirs, data, data_length, 0);
    return 1;
}

uint8_t ir_targeted_cmd(uint8_t dirs, char *data, uint8_t data_length, id_t target){
	return all_ir_sends(dirs, data, data_length, target, 1);
}

uint8_t ir_cmd(uint8_t dirs, char *data, uint8_t data_length){	
	return all_ir_sends(dirs, data, data_length, 0, 1);
}

uint8_t ir_targeted_send(uint8_t dirs, char *data, uint8_t data_length, id_t target){
	return all_ir_sends(dirs, data, data_length, target, 0);
}

uint8_t ir_send(uint8_t dirs, char *data, uint8_t data_length){
	return all_ir_sends(dirs, data, data_length, 0, 0);
}

static uint8_t all_hp_ir_cmds(uint8_t dirs, char* data, uint8_t data_length, id_t target){
    //perform_ir_upkeep();
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(ir_is_busy(dirs)>=4){
			//printf_P(PSTR("HP send blocked by other HP. Should only see this rarely.\r\n"));
			return 0;
		}
		uint8_t timed;
		if(data_length>=64){
			data_length-=64;
			timed=1;
		}
		for(uint8_t dir=0;dir<6;dir++){
			if(dirs&(1<<dir)){
				channel[dir]->CTRLB &= ~USART_RXEN_bm;
				ir_rxtx[dir].status = IR_STATUS_BUSY_bm | IR_STATUS_COMMAND_bm;
				ir_rxtx[dir].status |= (timed ? IR_STATUS_TIMED_bm : 0);
				ir_rxtx[dir].target_ID=target;
			}
		}
	}
    send_msg(dirs, data, data_length, 1);
	return 1;
}

uint8_t hp_ir_cmd(uint8_t dirs, char *data, uint8_t data_length){
	return all_hp_ir_cmds(dirs, data, data_length, 0);
}

uint8_t hp_ir_targeted_cmd(uint8_t dirs, char *data, uint8_t data_length, id_t target){
	return all_hp_ir_cmds(dirs, data, data_length, target);
}

void waitForTransmission(uint8_t dirs){
	   uint8_t busy;
	   do{
		   busy=0;
		   for(uint8_t dir=0; dir<6; dir++){
			   if(dirs&(1<<dir)){
				   if(ir_rxtx[dir].status & IR_STATUS_TRANSMITTING_bm){
					   busy=1;
				   }
			   }
		   }
		   delay_us(100);
	   } while (busy);
}



// To be called from interrupt handler only. Do not call.
static void ir_receive(uint8_t dir){
	uint8_t in_byte = channel[dir]->DATA;				// Some data just came in
	#ifdef AUDIO_DROPLET
		//ir_sense_channels[dir]->INTCTRL = ADC_CH_INTLVL_HI_gc;
	#endif	
	
	uint32_t now = get_time();
	if(now-ir_rxtx[dir].last_byte > IR_MSG_TIMEOUT)	clear_ir_buffer(dir);	
	ir_rxtx[dir].last_byte = now;
	#ifdef HARDCORE_DEBUG_DIR
		if(dir==HARDCORE_DEBUG_DIR) printf("%02hx ", in_byte); //Used for debugging - prints raw bytes as we get them.
	#endif	
	switch(ir_rxtx[dir].curr_pos){
		case HEADER_POS_SENDER_ID_LOW:	ir_rxtx[dir].sender_ID		= (uint16_t)in_byte;		break;
		case HEADER_POS_SENDER_ID_HIGH:	ir_rxtx[dir].sender_ID	   |= (((uint16_t)in_byte)<<8);	break;
		case HEADER_POS_CRC_LOW:		ir_rxtx[dir].data_crc		= (uint16_t)in_byte;		break;
		case HEADER_POS_CRC_HIGH:		ir_rxtx[dir].data_crc	   |= (((uint16_t)in_byte)<<8); break;																								
		case HEADER_POS_MSG_LENGTH:
										ir_rxtx[dir].status		   |= (in_byte&DATA_LEN_STATUS_BITS_bm);
										ir_rxtx[dir].calc_crc		= _crc16_update(ir_rxtx[dir].sender_ID, ir_rxtx[dir].status & IR_STATUS_CRC_BITS_bm);
										ir_rxtx[dir].data_length	= in_byte&DATA_LEN_VAL_bm;
										if(ir_rxtx[dir].data_length>IR_BUFFER_SIZE) ir_rxtx[dir].data_length=1; //basically, this will cause the message to get aborted.
																								break;
		case HEADER_POS_TARGET_ID_LOW:  ir_rxtx[dir].target_ID		= (uint16_t)in_byte;		break;
		case HEADER_POS_TARGET_ID_HIGH:
										ir_rxtx[dir].target_ID	   |= (((uint16_t)in_byte)<<8);
										ir_rxtx[dir].calc_crc		= _crc16_update(ir_rxtx[dir].calc_crc, ir_rxtx[dir].target_ID);
										break;
		case HEADER_POS_SOURCE_DIR:		ir_rxtx[dir].inc_dir		= in_byte;				break;										
		default:
			ir_rxtx[dir].buf[ir_rxtx[dir].curr_pos-HEADER_LEN] = in_byte;
			ir_rxtx[dir].calc_crc = _crc16_update(ir_rxtx[dir].calc_crc, in_byte);
	}
	ir_rxtx[dir].curr_pos++;
	if(ir_rxtx[dir].curr_pos>=(ir_rxtx[dir].data_length+HEADER_LEN)){
		ir_rxtx[dir].status |= ir_rxtx[dir].target_ID ? IR_STATUS_TARGETED_bm : 0;
		//pre checks.
		const uint8_t crcMismatch = ir_rxtx[dir].calc_crc!=ir_rxtx[dir].data_crc;
		const uint8_t nullCrc	  = ir_rxtx[dir].calc_crc==0;
		const uint8_t selfSender  = ir_rxtx[dir].sender_ID == get_droplet_id();
		const uint8_t notTimed	  = !(ir_rxtx[dir].status & IR_STATUS_TIMED_bm);
		const uint8_t wrongTarget = (notTimed && ir_rxtx[dir].target_ID && ir_rxtx[dir].target_ID!=get_droplet_id());
		const uint8_t incDirErr	= 0;//(notTimed && (ir_rxtx[dir].inc_dir&INC_DIR_KEY)!=INC_DIR_KEY);
		if(!((crcMismatch||nullCrc)||(selfSender||wrongTarget)||incDirErr)){
			if(notTimed){
				ir_rxtx[dir].inc_dir = ir_rxtx[dir].inc_dir&(~INC_DIR_KEY); //remove key bits.							
			}
			if(ir_rxtx[dir].status & IR_STATUS_COMMAND_bm){
				if(notTimed){
					received_ir_cmd(dir);
				}else{
					switch(ir_rxtx[dir].data_length){
						case 0: received_ir_sync(ir_rxtx[dir].inc_dir, ir_rxtx[dir].sender_ID); break;
						case 1: received_rnb_r(ir_rxtx[dir].inc_dir, ir_rxtx[dir].sender_ID, ir_rxtx[dir].last_byte); break;
					}			
				}			
			}else{
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
					ir_rxtx[dir].status |= IR_STATUS_COMPLETE_bm;
					ir_rxtx[dir].status |= IR_STATUS_BUSY_bm; //mark as busy so we don't overwrite it.
					channel[dir]->CTRLB &= ~USART_RXEN_bm; //Disable receiving messages on this channel until the message has been processed.
				}
			}
		}else{
			clear_ir_buffer(dir);
		}
	}
}

static void received_ir_cmd(uint8_t dir){
	uint8_t processThisCommand = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(!processing_cmd){
			processThisCommand = 1;
			memcpy((void*)cmd_buffer, (char*)ir_rxtx[dir].buf, ir_rxtx[dir].data_length);
			cmd_buffer[ir_rxtx[dir].data_length]='\0';
			cmd_length = ir_rxtx[dir].data_length;
			cmd_arrival_time = ir_rxtx[dir].last_byte;	//This is a 'global' value, referenced by other *.c files.
			cmd_sender_id = ir_rxtx[dir].sender_ID;		//This is a 'global' value, referenced by other *.c files.
			cmd_arrival_dir = dir;
			cmd_sender_dir  = ir_rxtx[dir].inc_dir;
			processing_cmd = 1;
		}
	}
	if(processThisCommand){
		schedule_task(5, handle_cmd_wrapper, NULL);
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			for(uint8_t other_dir=0;other_dir<6;other_dir++){
				clear_ir_buffer(other_dir);
			}
		}
	}
}

static void received_ir_sync(uint8_t delay, id_t senderID){
	uint8_t processThisFFSync = 0;
	uint16_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(!processing_ffsync_flag){
			count = TCE0.CNT;
			if(delay!=0xFF){
				processThisFFSync = 1;
				processing_ffsync_flag = 1;
			}
		}
	}
	if(processThisFFSync){
		//printf("senderID: %04X\tdelay: %hu\r\n", ir_rxtx[dir].sender_ID, delay);
		update_firefly_counter(count, delay);
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			for(uint8_t dir=0;dir<6;dir++){
				if(ir_rxtx[dir].sender_ID==senderID){
					clear_ir_buffer(dir);
				}
			}
			processing_ffsync_flag = 0;
		}
	}
	//printf("F\r\n");
}

static void received_rnb_r(uint8_t delay, id_t senderID, uint32_t last_byte){
	uint8_t processThisRNB = 0;
	uint32_t rnbCmdSentTime = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(!processing_rnb_flag && (ir_is_busy(ALL_DIRS)<8)){
			if(delay!=0xFF){
				rnbCmdID = senderID;
				//printf("%04X: %hu\r\n", rnbCmdID, delay+5);			
				if(delay<5) delay = 20-delay;
				rnbCmdSentTime = last_byte-(delay+5);
				processThisRNB = 1;
				processing_rnb_flag = 1;
				hp_ir_block_bm = 0x3F;

			}
		}
	}
	if(processThisRNB){
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			for(uint8_t dir=0;dir<6;dir++){
				if(ir_rxtx[dir].sender_ID==senderID){
					clear_ir_buffer(dir);
				}
			}
		}
		ir_range_meas(rnbCmdSentTime);	
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			hp_ir_block_bm = 0;
		}
		schedule_task(10, use_rnb_data, NULL);
	}
	//printf("R\r\n");
}

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
static volatile uint8_t next_byte;
static void ir_transmit(uint8_t dir){
	switch(ir_rxtx[dir].curr_pos){
		case HEADER_POS_SENDER_ID_LOW:  next_byte  = (uint8_t)(ir_rxtx[dir].sender_ID&0xFF);		break;
		case HEADER_POS_SENDER_ID_HIGH: next_byte  = (uint8_t)((ir_rxtx[dir].sender_ID>>8)&0xFF);	break;	
		case HEADER_POS_CRC_LOW:		next_byte  = (uint8_t)(ir_rxtx[dir].data_crc&0xFF);			break;
		case HEADER_POS_CRC_HIGH:		next_byte  = (uint8_t)((ir_rxtx[dir].data_crc>>8)&0xFF);	break;	
		case HEADER_POS_MSG_LENGTH:		next_byte  = ir_rxtx[dir].data_length & DATA_LEN_VAL_bm;
										next_byte |= (ir_rxtx[dir].status & IR_STATUS_COMMAND_bm);
										next_byte |= (ir_rxtx[dir].status & IR_STATUS_TIMED_bm); break;
		case HEADER_POS_TARGET_ID_LOW:	next_byte  = (uint8_t)(ir_rxtx[dir].target_ID&0xFF);		break;
		case HEADER_POS_TARGET_ID_HIGH:	next_byte  = (uint8_t)((ir_rxtx[dir].target_ID>>8)&0xFF);	break;
		case HEADER_POS_SOURCE_DIR:	
									if(!(ir_rxtx[dir].status&IR_STATUS_TIMED_bm)){
										next_byte  = INC_DIR_KEY|dir;								
									}else{
										uint16_t diff = ((uint16_t)(get_time()&0xFFFF))-ir_rxtx[dir].target_ID;
										//if(dir==0||dir==5) printf("(%hu) T: %u\r\n",dir, diff);
										if(diff<255){
											next_byte = (uint8_t)diff;
										}else{
											next_byte = 255;
										}
									}
									break;
		default: next_byte = ir_rxtx[dir].buf[ir_rxtx[dir].curr_pos - HEADER_LEN];
	}
	channel[dir]->DATA = next_byte;
	ir_rxtx[dir].curr_pos++;
	/* CHECK TO SEE IF MESSAGE IS COMPLETE */
	if(ir_rxtx[dir].curr_pos >= (ir_rxtx[dir].data_length+HEADER_LEN)){
		//printf("transmit of %hu-byte long message completed on dir %hu.\r\n\t", ir_rxtx[dir].data_length & DATA_LEN_VAL_bm, dir);
		//for(uint8_t i=0;i<ir_rxtx[dir].data_length & DATA_LEN_VAL_bm; i++){
			//printf("%02hX ", ir_rxtx[dir].buf[i]);
		//}
		//printf("\r\n");
		clear_ir_buffer(dir);
		channel[dir]->CTRLA &= ~USART_DREINTLVL_gm; //Turn off interrupt things.
		
	}

}

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
static void ir_transmit_complete(uint8_t dir){
	// this code is being executed because a TXCIF interrupt was executed, see pg. 305 AU manual:
	//	TXCIF: Transmit Complete Interrupt Flag
	//	This flag is set when the entire frame in the transmit shift register has been shifted out and there
	//	are no new data in the transmit buffer (DATA). TXCIF is automatically cleared when the transmit
	//	complete interrupt vector is executed. The flag can also be cleared by writing a one to its bit location.
	//	To do: this suggests we don't really need to do this a byte at a time? Need to check.
	//  Calling this code signals the end of the transmit process
	//printf("\r\n");
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		TCF2.CTRLB &= ~ir_carrier_bm[dir]; //Turn off the carrier wave.

		ir_rxtx[dir].status = 0;
		ir_rxtx[dir].data_length = 0;
		ir_rxtx[dir].curr_pos = 0;
		ir_rxtx[dir].target_ID = 0;
		ir_rxtx[dir].sender_ID = 0;
	
		channel[dir]->STATUS |= USART_TXCIF_bm;		// writing a 1 to this bit manually clears the TXCIF flag
		channel[dir]->CTRLB |= USART_RXEN_bm;	// this enables receive on the USART
	}
}

uint8_t ir_is_busy(uint8_t dirs_mask){
	uint32_t now = get_time();
	uint8_t hp_block = 0;
	uint8_t transmitting = 0;
	uint8_t receiving = 0;
	uint8_t timed_cmd = 0;
	if(hp_ir_block_bm&dirs_mask){
		hp_block = 1<<3;
	}
	for(uint8_t dir=0; dir<6; dir++){
    	if(dirs_mask&(1<<dir)){
			if((ir_rxtx[dir].status & IR_STATUS_TIMED_bm) && (ir_rxtx[dir].status & IR_STATUS_COMMAND_bm)){
				timed_cmd			= 1<<2;
			}
        	if(ir_rxtx[dir].status & IR_STATUS_TRANSMITTING_bm){
            	transmitting	= 1<<1;
        	}
			if((now - ir_rxtx[dir].last_byte) < IR_MSG_TIMEOUT){
				receiving		= 1<<0;
			}
    	}
	}
	return receiving | transmitting | timed_cmd | hp_block;
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
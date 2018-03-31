#include "ir_comm.h"

static volatile uint8_t processingCmdFlag;
static volatile uint8_t processingFFsyncFlag;

static void clearIrBuffer(uint8_t dir);
static NODE* all_ir_sends(uint8_t dirs, char* data, uint8_t dataLength, id_t target, uint8_t cmdFlag);
static uint8_t all_hp_ir_cmds(uint8_t dirs, char* data, uint8_t dataLength, id_t target);
static void irReceive(uint8_t dir); //Called by Interrupt Handler Only
static void receivedIrCmd(uint8_t dir);
static void addMsgToMsgQueue(uint8_t dir);
static void handleCompletedMsg(uint8_t dir);
static void receivedRnbCmd(uint16_t delay, uint32_t lastByte, id_t senderID);
static void receivedIrSyncCmd(uint16_t delay, uint32_t lastByte, id_t senderID);
static void irTransmit(uint8_t dir);
static void irTransmitComplete(uint8_t dir);

static volatile uint16_t	cmdLength;
static volatile char		cmdBuffer[SRL_BUFFER_SIZE];
//volatile uint8_t byte_array[300];
//int rxbyte_count = 0;

/* Hardware addresses for the port pins with the carrier wave */
static uint8_t ir_carrier_bm[] = { PIN0_bm, PIN1_bm, PIN4_bm, PIN5_bm, PIN6_bm, PIN7_bm };
	
static volatile uint32_t last_return =0;
static volatile uint32_t last_sched =0;
uint32_t first_attempt;
#define timeout_PERIOD 30000

static volatile uint16_t listSize=0;

//#define HARDCORE_DEBUG_DIR 0x00

static void clearIrBuffer(uint8_t dir){
	#ifdef AUDIO_DROPLET
		ir_sense_channels[dir]->INTCTRL = ADC_CH_INTLVL_OFF_gc;
	#endif
	#ifdef HARDCORE_DEBUG_DIR
		if(dir==HARDCORE_DEBUG_DIR) printf("\r\n");
	#endif
	ir_rxtx[dir].data_crc		= 0;
	ir_rxtx[dir].senderID		= 0;
	
	ir_rxtx[dir].target_ID		= 0;	
	ir_rxtx[dir].curr_pos		= 0;
	ir_rxtx[dir].calc_crc		= 0;
	ir_rxtx[dir].data_length	= 0;	
	
	ir_rxtx[dir].status			= 0;	
	
	channel[dir]->CTRLB |= USART_RXEN_bm; //this enables receive on the USART

}

void irCommInit(){
	/* Initialize UARTs */
	// RX pins as inputs:
	PORTC.DIRCLR = PIN2_bm | PIN6_bm;		// DIR 0,1							
	PORTD.DIRCLR = PIN2_bm;					// DIR 2
	PORTE.DIRCLR = PIN2_bm | PIN6_bm;		// DIR 3,4
	PORTF.DIRCLR = PIN2_bm;					// DIR 5

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		hpIrBlock_bm=0x3F;
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

	cmdArrivalTime=0;
	numWaitingMsgs=0;
	userFacingMessagesOvf=0;
	processingCmdFlag = 0;
	processingFFsyncFlag = 0;
	incomingMsgHead = NULL;
	memoryConsumedByBuffer = 0;
	for(uint8_t dir=0; dir<6; dir++) clearIrBuffer(dir); //this initializes the buffer's values to 0.

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		hpIrBlock_bm = 0;
	}
}

void handleCmdWrapper(){
	char local_msg_copy[cmdLength+1];	
	uint16_t local_msg_len;
	//printf("\tIn handle_cmd_wrapper.\r\n");
	//Nothing should touch the cmd_buffer and stuff as long as processing_cmd is still 1.
	memcpy(local_msg_copy, (const void*)cmdBuffer, cmdLength+1);
	local_msg_len = cmdLength;
	handleSerialCommand(local_msg_copy, local_msg_len);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		processingCmdFlag = 0;
	}
}

void send_msg(uint8_t dirs, char *data, uint8_t dataLength, uint8_t hpFlag){
	if(dataLength>IR_BUFFER_SIZE) printf_P(PSTR("ERROR: Message exceeds IR_BUFFER_SIZE.\r\n"));
	
	uint16_t crc = getDropletID();
	for(uint8_t dir=0; dir<6; dir++){
		if(dirs&(1<<dir)){			
			crc = _crc16_update(crc, (ir_rxtx[dir].status & IR_STATUS_CRC_BITS_bm));
			if(!(ir_rxtx[dir].status&IR_STATUS_TIMED_bm)){
				crc = _crc16_update(crc, ir_rxtx[dir].target_ID);
			}
			break;
		}	
	}

	for(uint8_t i=0; i<dataLength; i++) crc = _crc16_update(crc, data[i]); //Calculate CRC of outbound message.
	
	for(uint8_t dir=0; dir<6; dir++){
		if(dirs&(1<<dir)){
			ir_rxtx[dir].status |= IR_STATUS_TRANSMITTING_bm;
			ir_rxtx[dir].data_length = dataLength;
			ir_rxtx[dir].data_crc = crc;
			ir_rxtx[dir].curr_pos = 0;
			ir_rxtx[dir].senderID = getDropletID();
			memcpy((char*)ir_rxtx[dir].buf, data, dataLength);
			TCF2.CTRLB |= ir_carrier_bm[dir];		// Turn on carrier wave on port dir
		}
	}

	for(uint8_t dir=0; dir<6; dir++){
		if(dirs&(1<<dir)){
			ir_rxtx[dir].last_byte = 0;
			if(hpFlag){
				channel[dir]->CTRLA |= USART_DREINTLVL_HI_gc;
			}else{
				channel[dir]->CTRLA |= USART_DREINTLVL_MED_gc;			
			}
		}
	}

	/* The whole transmission will now occur in interrupts. */
}

void sendRtsByte(){
//If this function ever actually gets pushed to master, should probably set and clear the hpIr semaphore.	
	hpIrTargetedCmd(ALL_DIRS, ".", 66, 0);//Super hacky!
}

//dirs_to_send = 0b00111111;
//busy_dirs = 0b00111001;
//dirs_to_send & ~busy_dirs //== 0b00000110;



uint32_t getExponentialBackoff(uint8_t c){
	
	volatile uint32_t k;
	volatile uint32_t N;
	uint32_t return_value;
	
	N= (((uint32_t)1)<<c);
	
	k = ((randQuad()%N)+1);
	return_value = k*IR_MSG_TIMEOUT;
	last_return = return_value;
	
	
	return (return_value);///20000000;
	
}

void removeHeadAndUpdate(){
		volatile NODE * tempNode=BUFFER_HEAD;
		if(BUFFER_HEAD->next == BUFFER_HEAD){ //This means BUFFER_HEAD is only thing in list.
			BUFFER_HEAD = NULL;
		}else{
			BUFFER_HEAD=BUFFER_HEAD->next;
			BUFFER_HEAD->prev = tempNode->prev;
			tempNode->prev->next = BUFFER_HEAD;
		}
		listSize = listSize - ( sizeof(NODE) + tempNode->data_length);
		myFree((NODE*)tempNode);
		tempNode = NULL;
}

/*
 * TODO: 
 *   Could extend this so that, if we are busy transmitting, we wait an amount of time based on
 *   how much longer we expect the message to take to send, instead of the exponential backoff.
 */
void tryAndSendMessage(){//void * msg_temp_node){
	volatile Msg* data;
	//msgNode = (NODE *)msg_temp_node;
	if(irIsBusy(BUFFER_HEAD->channel_id)>0){// && (get_time()-first_attempt < timeout_PERIOD) && (BUFFER_HEAD->no_of_tries < 10)){
		if(BUFFER_HEAD->no_of_tries < IR_MAX_MSG_TRIES){
			BUFFER_HEAD->no_of_tries+=1;
			uint32_t time_backoff = getExponentialBackoff(BUFFER_HEAD->no_of_tries);
			scheduleTask(time_backoff, tryAndSendMessage, NULL);// (void *)BUFFER_HEAD);		
		}else{
			data = (Msg*)(BUFFER_HEAD->data);
			printf("\n\rERROR: VERY BUSY! MESSAGE NUMBER %u DISCARDED\n\r", data->msgId);
			removeHeadAndUpdate();
			if(BUFFER_HEAD){
				scheduleTask(100, tryAndSendMessage, NULL);// (void *)BUFFER_HEAD);
				return;
			}			
		}
	}else{
		for(uint8_t dir=0;dir<6;dir++){
			if(BUFFER_HEAD->channel_id&(1<<dir)){
				channel[dir]->CTRLB &= ~USART_RXEN_bm;
				ir_rxtx[dir].status = IR_STATUS_BUSY_bm;
				if(BUFFER_HEAD->cmd_flag) ir_rxtx[dir].status |= IR_STATUS_COMMAND_bm;
				ir_rxtx[dir].target_ID=BUFFER_HEAD->target;
			}
		}
		if(BUFFER_HEAD->data_length == sizeof(Msg)){
			Msg* msg = (Msg*)(BUFFER_HEAD->data);
			msg->time = getTime() - msg->time;
		}
		send_msg(BUFFER_HEAD->channel_id, BUFFER_HEAD->data, BUFFER_HEAD->data_length, 0);

		//uint8_t dataSender = (uint8_t)(log((data->msgId)>>12)/log(2));
		//uint16_t printID = (data->msgId)&0x0FFF;
		//printf("\n\rsend success %01hu %6u at %lu\r\n", dataSender, printID, getTime());
		uint32_t msgMSlen = MSG_DUR(BUFFER_HEAD->data_length);
		//uint32_t sched_now = (last_sched + msgMSlen)%115;
		//last_sched = sched_now;
		removeHeadAndUpdate();
		if(BUFFER_HEAD)
			scheduleTask(msgMSlen+IR_MSG_TIMEOUT, tryAndSendMessage, NULL);// (void *)BUFFER_HEAD);
	}

}



void printMsgQueue(){
	//printf("Msg Queue contains:\r\n");
	//For debugging:
	uint16_t listLength = 0;
	volatile NODE* tmp = BUFFER_HEAD;
	do{
		listLength++;
		//printf("\t%p: %hu", tmp, ((Msg*)(tmp->data))->msgId);
		//printf("\t\tPREV: %p\tNEXT: %p\r\n", tmp->prev, tmp->next);
		tmp = tmp->next;
	}while(tmp!=BUFFER_HEAD);
	//printf("List Length: %u\r\n", listLength);
}

static NODE* all_ir_sends(uint8_t dir, char * str, uint8_t dataLength, id_t msgTarget, uint8_t cmdFlag){
	
	listSize += sizeof(NODE) + dataLength;
	if(listSize> 500){
		listSize = listSize - (sizeof(NODE) + dataLength);
		return NULL;
		
	}
	//printf("\n\rSize of list node = %d", listSize);
	NODE* bufferPointer = (NODE *) myMalloc(sizeof(NODE)+dataLength);
	
	//make sure there was enough memory
	if(bufferPointer == NULL)
	return NULL;
	
	//channel ID, target, length,cmd_flag
	bufferPointer->channel_id = dir;
	bufferPointer->target = msgTarget;
	bufferPointer->cmd_flag = cmdFlag;
	bufferPointer->data_length = dataLength;
	memcpy(bufferPointer->data, str, dataLength);
	
	
	//printf("\n\rData = %s,Channel_ID = %u", bufferPointer->data, bufferPointer->channel_id);
	//Msg* msg = (Msg*)(str);
	//printf("Adding (%u) at %p\r\n", msg->msgId,bufferPointer);
	bufferPointer->no_of_tries = 1;
	
	//Linked list if required
	
	bufferPointer->next = NULL;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(BUFFER_HEAD == NULL){
			BUFFER_HEAD = bufferPointer;
			BUFFER_HEAD->next = bufferPointer;
			BUFFER_HEAD->prev = bufferPointer;
			tryAndSendMessage();
		}else{
			bufferPointer->next = BUFFER_HEAD;
			bufferPointer->prev = BUFFER_HEAD->prev;
			BUFFER_HEAD->prev->next = bufferPointer;
			BUFFER_HEAD->prev = bufferPointer;
			
		}
		printMsgQueue();
	}
	
	return bufferPointer;
	//bufferPointer;
}

NODE* irTargetedCmd(uint8_t dirs, char *data, uint8_t dataLength, id_t target){
	return all_ir_sends(dirs, data, dataLength, target, 1);
}

NODE* irCmd(uint8_t dirs, char *data, uint8_t dataLength){	
	return all_ir_sends(dirs, data, dataLength, 0, 1);
}

NODE* irTargetedSend(uint8_t dirs, char *data, uint8_t dataLength, id_t target){
	return all_ir_sends(dirs, data, dataLength, target, 0);
}

NODE* irSend(uint8_t dirs, char *data, uint8_t dataLength){
	return all_ir_sends(dirs, data, dataLength, 0, 0);
}

static uint8_t all_hp_ir_cmds(uint8_t dirs, char* data, uint8_t dataLength, id_t target){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(irIsBusy(dirs)>=4){
			//printf_P(PSTR("HP send blocked by other HP. Should only see this rarely.\r\n"));
			return 0;
		}
		uint8_t timed;
		if(dataLength>=64){
			dataLength-=64;
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
	send_msg(dirs, data, dataLength, 1);
	return 1;
}

uint8_t hpIrCmd(uint8_t dirs, char *data, uint8_t dataLength){
	return all_hp_ir_cmds(dirs, data, dataLength, 0);
}

uint8_t hpIrTargetedCmd(uint8_t dirs, char *data, uint8_t dataLength, id_t target){
	return all_hp_ir_cmds(dirs, data, dataLength, target);
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

static void addMsgToMsgQueue(uint8_t dir){
	if(ir_rxtx[dir].data_length == 0){
		printf_P(PSTR("ERROR! Should NOT be adding 0-length message to queue.\r\n"));
	}else if(ir_rxtx[dir].data_length > IR_BUFFER_SIZE){
		printf_P(PSTR("ERROR! Should NOT be adding a message with length greater than buffer size to queue.\r\n"));
	}else if(memoryConsumedByBuffer > 500){
		printf_P(PSTR("ERROR! Buffered incoming messages consuming too much memory. Allow handle_msg to be called more frequently.\r\n"));
	}else{
		volatile MsgNode* node = incomingMsgHead;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			if(incomingMsgHead==NULL){
				incomingMsgHead = (volatile MsgNode*)myMalloc(sizeof(MsgNode) + ir_rxtx[dir].data_length);
				node = (MsgNode*)incomingMsgHead;
			}else{
				while(node->next != NULL){
					node = node->next;
				}
				node->next = (MsgNode*)myMalloc(sizeof(MsgNode) + ir_rxtx[dir].data_length);
				node = node->next;
			}
			char* dataAddr = ((char*)node + sizeof(MsgNode));
			memcpy(dataAddr, (const void*)ir_rxtx[dir].buf, ir_rxtx[dir].data_length);
			node->msg			= dataAddr;
			node->arrivalTime	= ir_rxtx[dir].last_byte;
			node->length		= ir_rxtx[dir].data_length;
			node->senderID		= ir_rxtx[dir].senderID;
			node->crc			= ir_rxtx[dir].calc_crc;
			node->next			= NULL;
			memoryConsumedByBuffer += (sizeof(MsgNode) + ir_rxtx[dir].data_length);
			numWaitingMsgs++;
		}
	}
}

static void handleCompletedMsg(uint8_t dir){
	ir_rxtx[dir].status |= ir_rxtx[dir].target_ID ? IR_STATUS_TARGETED_bm : 0;
	//pre checks.
	const uint8_t crcMismatch = ir_rxtx[dir].calc_crc!=ir_rxtx[dir].data_crc;
	const uint8_t nullCrc	  = ir_rxtx[dir].calc_crc==0;
	const uint8_t selfSender  = ir_rxtx[dir].senderID == getDropletID();
	const uint8_t notTimed	  = !(ir_rxtx[dir].status & IR_STATUS_TIMED_bm);
	const uint8_t wrongTarget = (notTimed && ir_rxtx[dir].target_ID && ir_rxtx[dir].target_ID!=getDropletID());
	if(!((crcMismatch||nullCrc)||(selfSender||wrongTarget))){
		if(ir_rxtx[dir].status & IR_STATUS_COMMAND_bm){
			if(notTimed){
				receivedIrCmd(dir);
			}else{
				switch(ir_rxtx[dir].data_length){
					case 0: receivedIrSyncCmd(ir_rxtx[dir].target_ID, ir_rxtx[dir].last_byte, ir_rxtx[dir].senderID); break;
					case 1: receivedRnbCmd(ir_rxtx[dir].target_ID, ir_rxtx[dir].last_byte, ir_rxtx[dir].senderID); break;
				}
			}
		}else{
			addMsgToMsgQueue(dir);
		}
	}
	clearIrBuffer(dir);
}


// To be called from interrupt handler only. Do not call.
static void irReceive(uint8_t dir){
	uint8_t in_byte = channel[dir]->DATA;				// Some data just came in

	#ifdef AUDIO_DROPLET
		//ir_sense_channels[dir]->INTCTRL = ADC_CH_INTLVL_HI_gc;
	#endif	
	
	uint32_t now = getTime();
	if(now-ir_rxtx[dir].last_byte > IR_MSG_TIMEOUT){
		if(in_byte==RTS_BYTE){
			printf("!");
		}
		clearIrBuffer(dir);	
	}
	ir_rxtx[dir].last_byte = now;
	#ifdef HARDCORE_DEBUG_DIR
		if(dir==HARDCORE_DEBUG_DIR){
			if(in_byte!=0xff){
				printf("%02hx ", in_byte); //Used for debugging - prints raw bytes as we get them.
			}
		}
	#endif	
	switch(ir_rxtx[dir].curr_pos){
		case HEADER_POS_SENDER_ID_LOW:	ir_rxtx[dir].senderID		= (uint16_t)in_byte;		break;
		case HEADER_POS_SENDER_ID_HIGH:	ir_rxtx[dir].senderID	   |= (((uint16_t)in_byte)<<8);	break;
		case HEADER_POS_CRC_LOW:		ir_rxtx[dir].data_crc		= (uint16_t)in_byte;		break;
		case HEADER_POS_CRC_HIGH:		ir_rxtx[dir].data_crc	   |= (((uint16_t)in_byte)<<8); break;																								
		case HEADER_POS_MSG_LENGTH:
										ir_rxtx[dir].status		   |= (in_byte&DATA_LEN_STATUS_BITS_bm);
										ir_rxtx[dir].calc_crc		= _crc16_update(ir_rxtx[dir].senderID, ir_rxtx[dir].status & IR_STATUS_CRC_BITS_bm);
										ir_rxtx[dir].data_length	= in_byte&DATA_LEN_VAL_bm;
										if(ir_rxtx[dir].data_length>IR_BUFFER_SIZE) ir_rxtx[dir].data_length=1; //basically, this will cause the message to get aborted.
																								break;
		case HEADER_POS_TARGET_ID_LOW:  ir_rxtx[dir].target_ID		= (uint16_t)in_byte;		break;
		case HEADER_POS_TARGET_ID_HIGH:
										ir_rxtx[dir].target_ID	   |= (((uint16_t)in_byte)<<8);
										if(!(ir_rxtx[dir].status & IR_STATUS_TIMED_bm)){
											ir_rxtx[dir].calc_crc		= _crc16_update(ir_rxtx[dir].calc_crc, ir_rxtx[dir].target_ID);
										}
										break;								
		default:
			ir_rxtx[dir].buf[ir_rxtx[dir].curr_pos-HEADER_LEN] = in_byte;
			ir_rxtx[dir].calc_crc = _crc16_update(ir_rxtx[dir].calc_crc, in_byte);
	}
	if(ir_rxtx[dir].curr_pos<HEADER_LEN){
		ir_rxtx[dir].buf[0] = in_byte;
	}
	ir_rxtx[dir].curr_pos++;
	if(ir_rxtx[dir].curr_pos>=(ir_rxtx[dir].data_length+HEADER_LEN)){
		handleCompletedMsg(dir);
	}
}

static void receivedIrCmd(uint8_t dir){
	uint8_t processThisCommand = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(!processingCmdFlag){
			processThisCommand = 1;
			memcpy((void*)cmdBuffer, (char*)ir_rxtx[dir].buf, ir_rxtx[dir].data_length);
			cmdBuffer[ir_rxtx[dir].data_length]='\0';
			cmdLength = ir_rxtx[dir].data_length;
			cmdArrivalTime = ir_rxtx[dir].last_byte;	//This is a 'global' value, referenced by other *.c files.
			cmdSenderId = ir_rxtx[dir].senderID;		//This is a 'global' value, referenced by other *.c files.
			cmdArrivalDir = dir;
			processingCmdFlag = 1;
		}
	}
	if(processThisCommand){
		scheduleTask(5, handleCmdWrapper, NULL);
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			for(uint8_t other_dir=0;other_dir<6;other_dir++){
				clearIrBuffer(other_dir);
			}
		}
	}
}

static void receivedIrSyncCmd(uint16_t delay, uint32_t last_byte, id_t senderID){
	uint8_t processThisFFSync = 0;
	uint16_t count;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(!processingFFsyncFlag){
			count = TCE0.CNT;
			if(delay!=0xFFFF){
				processThisFFSync = 1;
				processingFFsyncFlag = 1;
			}
		}
	}
	if(processThisFFSync){
		//printf("senderID: %04X\tdelay: %hu\r\n", ir_rxtx[dir].sender_ID, delay);
		delay = delay+5+(getTime()-last_byte); //The time is measured right before sending and is the last two bytes of the message. Our baud rate means that it takes 5ms to send two bytes.
		updateFireflyCounter(count, delay);
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			for(uint8_t dir=0;dir<6;dir++){
				if(ir_rxtx[dir].senderID==senderID){
					clearIrBuffer(dir);
				}
			}
			processingFFsyncFlag = 0;
		}
	}
	//printf("F\r\n");
}

static void receivedRnbCmd(uint16_t delay, uint32_t last_byte, id_t senderID){
	uint8_t processThisRNB = 0;
	uint32_t rnbCmdSentTime = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(!processing_rnb_flag && (irIsBusy(ALL_DIRS)<8)){
			if(delay!=0xFFFF){
				rnbCmdID = senderID;
				//printf("%04X: %hu\r\n", rnbCmdID, delay+5);			
				if(delay<5) delay = 20-delay;
				rnbCmdSentTime = last_byte-(delay+5);
				processThisRNB = 1;
				processing_rnb_flag = 1;
				hpIrBlock_bm = 0x3F;
			}
		}
	}
	if(processThisRNB){
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			for(uint8_t dir=0;dir<6;dir++){
				if(ir_rxtx[dir].senderID==senderID){
					clearIrBuffer(dir);
				}
			}
		}
		irRangeMeas(rnbCmdSentTime);	
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			hpIrBlock_bm = 0;
		}
		scheduleTask(10, useRnbData, NULL);
	}
	//printf("R\r\n");
}

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
static volatile uint8_t next_byte;
static void irTransmit(uint8_t dir){
	switch(ir_rxtx[dir].curr_pos){
		case HEADER_POS_SENDER_ID_LOW:  next_byte  = (uint8_t)(ir_rxtx[dir].senderID&0xFF);			break;
		case HEADER_POS_SENDER_ID_HIGH: next_byte  = (uint8_t)((ir_rxtx[dir].senderID>>8)&0xFF);	break;	
		case HEADER_POS_CRC_LOW:		next_byte  = (uint8_t)(ir_rxtx[dir].data_crc&0xFF);			break;
		case HEADER_POS_CRC_HIGH:		next_byte  = (uint8_t)((ir_rxtx[dir].data_crc>>8)&0xFF);	break;	
		case HEADER_POS_MSG_LENGTH:		next_byte  = ir_rxtx[dir].data_length & DATA_LEN_VAL_bm;
										next_byte |= (ir_rxtx[dir].status & IR_STATUS_COMMAND_bm);
										next_byte |= (ir_rxtx[dir].status & IR_STATUS_TIMED_bm);	break;
		case HEADER_POS_TARGET_ID_LOW:	if((ir_rxtx[dir].status&IR_STATUS_TIMED_bm)){
											uint32_t truncatedTime = getTime()&0xFFFF;
											uint16_t timeGap = (truncatedTime < ir_rxtx[dir].target_ID) ? ((truncatedTime+0x10000)-truncatedTime) : (truncatedTime-ir_rxtx[dir].target_ID);
											ir_rxtx[dir].target_ID = (timeGap>30000) ? 0xFFFF0 : timeGap;
										}
										next_byte  = (uint8_t)(ir_rxtx[dir].target_ID&0xFF);		break;
		case HEADER_POS_TARGET_ID_HIGH:	next_byte = (uint8_t)((ir_rxtx[dir].target_ID>>8)&0xFF);	break;
		default: next_byte = ir_rxtx[dir].buf[ir_rxtx[dir].curr_pos - HEADER_LEN];
	}
	if((ir_rxtx[dir].status & IR_STATUS_TIMED_bm) && (ir_rxtx[dir].status & IR_STATUS_COMMAND_bm) && ir_rxtx[dir].data_length==2){ //SUPER HACKY WAY TO SEND ONE BYTE.
		channel[dir]->DATA = RTS_BYTE;
		ir_rxtx[dir].curr_pos=(IR_BUFFER_SIZE+HEADER_LEN); //This will force the if statement below to 'true', so the message ends and is cleaned up appropriately.
	}else{
		channel[dir]->DATA = next_byte;
	}
	ir_rxtx[dir].curr_pos++;
	/* CHECK TO SEE IF MESSAGE IS COMPLETE */
	if(ir_rxtx[dir].curr_pos >= (ir_rxtx[dir].data_length+HEADER_LEN)){
		//printf("transmit of %hu-byte long message completed on dir %hu.\r\n\t", ir_rxtx[dir].data_length & DATA_LEN_VAL_bm, dir);
		//for(uint8_t i=0;i<ir_rxtx[dir].data_length & DATA_LEN_VAL_bm; i++){
			//printf("%02hX ", ir_rxtx[dir].buf[i]);
		//}
		//printf("\r\n");
		clearIrBuffer(dir);
		channel[dir]->CTRLA &= ~USART_DREINTLVL_gm; //Turn off interrupt things.	
	}
}

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
static void irTransmitComplete(uint8_t dir){
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
		ir_rxtx[dir].senderID = 0;
		
		//The line below will keep irIsBusy from reporting that the communication medium is free immediately after a message has finished sending.
		//This should ensure that the comm buffers of any receiving Droplets are ready for a new message.
		ir_rxtx[dir].last_byte = getTime(); 
		
		channel[dir]->STATUS |= USART_TXCIF_bm;		// writing a 1 to this bit manually clears the TXCIF flag
		channel[dir]->CTRLB |= USART_RXEN_bm;	// this enables receive on the USART
	}
}

uint8_t irIsBusy(uint8_t dirs_mask){
	uint32_t now = getTime();
	uint8_t hp_block = 0;
	uint8_t transmitting = 0;
	uint8_t receiving = 0;
	uint8_t timed_cmd = 0;
	
	busy_ch = 0;
	
	if(hpIrBlock_bm&dirs_mask){
		hp_block = 1<<3;
	}
	for(uint8_t dir=0; dir<6; dir++){
		if(dirs_mask&(1<<dir)){
			if((ir_rxtx[dir].status & IR_STATUS_TIMED_bm) && (ir_rxtx[dir].status & IR_STATUS_COMMAND_bm)){
				timed_cmd			= 1<<2;
			}
			if(ir_rxtx[dir].status & IR_STATUS_TRANSMITTING_bm){
				transmitting	= 1<<1;
				busy_ch = 1<<dir;
			}
			if((now - ir_rxtx[dir].last_byte) < IR_MSG_TIMEOUT){
				receiving		= 1<<0;
				busy_ch = 1<<dir;
			}

			//printf("IR Sense [%hu]: %d, %d, %d, %d, %d, %d\r\n", receiving, vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
		}
	}
	return receiving | transmitting | timed_cmd | hp_block;
}


// ISRs for IR channel 0
ISR( USARTC0_RXC_vect ) { irReceive(0); }
ISR( USARTC0_TXC_vect ) { irTransmitComplete(0); }
ISR( USARTC0_DRE_vect ) { irTransmit(0); }

// ISRs for IR channel 1
ISR( USARTC1_RXC_vect ) { irReceive(1); }
ISR( USARTC1_TXC_vect ) { irTransmitComplete(1); }
ISR( USARTC1_DRE_vect ) { irTransmit(1); }

// ISRs for IR channel 2
ISR( USARTD0_RXC_vect ) { irReceive(2); }
ISR( USARTD0_TXC_vect ) { irTransmitComplete(2); }
ISR( USARTD0_DRE_vect ) { irTransmit(2); }

// ISRs for IR channel 3
ISR( USARTE0_RXC_vect ) { irReceive(3); }
ISR( USARTE0_TXC_vect ) { irTransmitComplete(3); }
ISR( USARTE0_DRE_vect ) { irTransmit(3); }

// ISRs for IR channel 4
ISR( USARTE1_RXC_vect ) { irReceive(4); }
ISR( USARTE1_TXC_vect ) { irTransmitComplete(4); }
ISR( USARTE1_DRE_vect ) { irTransmit(4); }

// ISRs for IR channel 5
ISR( USARTF0_RXC_vect ) { irReceive(5); }
ISR( USARTF0_TXC_vect ) { irTransmitComplete(5); }
ISR( USARTF0_DRE_vect ) { irTransmit(5); }
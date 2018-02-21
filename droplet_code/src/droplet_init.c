#include "droplet_init.h"

static void initAllSystems(void);
static void calculateIdNumber(void);
static void enableInterrupts(void);
static void checkMessages(void);
uint8_t calculate_page_number(uint16_t addressFrmProgramming);
uint8_t flashBufferPos=0;
/**
 * \brief Initializes all the subsystems for this Droplet. This function MUST be called
 * by the user before using any other functions in the API.
 */ 
static void initAllSystems(void){
	cli();
	Config32MHzClock();
	
	calculateIdNumber();
	
	schedulerInit();			INIT_DEBUG_PRINT("SCHEDULER INIT\r\n");
	pcCommInit();				INIT_DEBUG_PRINT("PC COM INIT\r\n");
	rgbLEDinit();				INIT_DEBUG_PRINT("LED INIT\r\n");
	powerInit();				INIT_DEBUG_PRINT("POWER INIT\r\n");
	i2cInit();					INIT_DEBUG_PRINT("I2C INIT\r\n");
	
	enableInterrupts();	
	
	rangeAlgsInit();			INIT_DEBUG_PRINT("RANGE ALGORITHMS INIT\r\n");
	rgbSensorInit();			INIT_DEBUG_PRINT("RGB SENSE INIT\r\n");
	irLedInit();				INIT_DEBUG_PRINT("IR LED INIT\r\n");
	irSensorInit();			INIT_DEBUG_PRINT("IR SENSE INIT\r\n");
	
	#ifdef AUDIO_DROPLET
		speakerInit();			INIT_DEBUG_PRINT("SPEAKER INIT\r\n");
		micInit();				INIT_DEBUG_PRINT("MIC INIT\r\n"); //Must occur after ir_sensor_init.
	#endif
	
	motorInit();				INIT_DEBUG_PRINT("MOTOR INIT\r\n");
	randomInit();				INIT_DEBUG_PRINT("RAND INIT\r\n"); //This uses adc readings for a random seed, and so requires that the adcs have been initialized.
	localizationInit();		INIT_DEBUG_PRINT("LOCALIZATION INIT\r\n"); 
	
	#ifdef SYNCHRONIZED
		fireflySyncInit();
	#endif

	reprogramming = 0;
	
	setAllirPowers(256);

	startupLightSequence();
	
	irCommInit();				INIT_DEBUG_PRINT("IR COM INIT\r\n");
}

int main(void){
	initAllSystems();
	init();
	while(1){
		loop();
		checkMessages();
		if(taskListCheck()){
			printf_P(PSTR("Error! We got ahead of the task list and now nothing will execute.\r\n"));
			printTaskQueue();
			taskListCleanup();
		}
		delayMS(1);	
	}
	return 0;
}

/*
 * This function loops through all messages this robot has received since the last call
 * to check messages.
 * For each message, it populates an ir_msg struct and calls handle_msg with it.
 */
static void checkMessages(void){
	irMsg msgStruct;	

	if(userFacingMessagesOvf){
		numWaitingMsgs=MAX_USER_FACING_MESSAGES;
		userFacingMessagesOvf=0;
		printf_P(PSTR("Error: Messages overflow. Too many messages received. Try speeding up your loop if you see this a lot.\r\n"));
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){ //We want to amke sure that this block doesn't get interrupted by stuff trying to add more messages to the buffer.
		uint16_t crc;
		while(incomingMsgHead != NULL){
			MsgNode* node = (MsgNode*)incomingMsgHead;
			if(node->length==0){
				printf_P(PSTR("ERROR: Message length 0 for msg_node.\r\n"));
			}
			char msgData[node->length];
			memcpy(msgData, node->msg, node->length);
			msgStruct.msg			= msgData;
			msgStruct.arrivalTime	= node->arrivalTime;
			msgStruct.senderID		= node->senderID;
			msgStruct.length		= node->length;
			crc						= node->crc;
			//While we let user code handle the message we want interrupts to be back on. At this point everything relevant has been copied out of the buffer.
			NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE){
				if(reprogramming)
				{
					setRGB(0,0,250);
					handle_serial_comm(&msgStruct);
				} 
				else handleMsg(&msgStruct);
			}
			
			MsgNode* tmp = node;
			MsgNode* deleteMe;
			while(tmp->next !=NULL){
				uint8_t crcMatches = (tmp->next->crc == crc);
				uint8_t closeTimes = (abs((int32_t)(tmp->next->arrivalTime) - (int32_t)(msgStruct.arrivalTime))) < 30;
				if(crcMatches && closeTimes){
					deleteMe = tmp->next;
					tmp->next = tmp->next->next;
					memoryConsumedByBuffer -= (sizeof(MsgNode) + msgStruct.length);
					numWaitingMsgs--;
					myFree(deleteMe);
				}else{
					tmp = tmp->next;
				}

			}
			incomingMsgHead = (volatile MsgNode*)(node->next);
			numWaitingMsgs--;
			memoryConsumedByBuffer -= (sizeof(MsgNode) + msgStruct.length);
			myFree(node);
			
			
		}
	}
}


void handle_serial_comm(irMsg* msg_struct)
{
	setRGB(0,250,0); // Blinks the led and helps to find out that message has been received
	delayMS(50000);
	printf("Message Received: %s\n\r",msg_struct->msg); // Used for debugging
	
}


uint8_t calculate_page_number(uint16_t addressFrmProgramming)
{
	uint8_t calc_page_number = (addressFrmProgramming/FLASH_PAGE_SIZE);
	
	return calc_page_number;
}

void handle_reprogramming(void)
{
	
	irMsg *msg_struct;
	char actual_struct[sizeof(irMsg)]; //It's like malloc, but on the stack.
	char actual_msg[IR_BUFFER_SIZE+1];
	msg_struct = (irMsg*)actual_struct;
	msg_struct->msg = actual_msg;
	strncpy(msg_struct->msg, "1012200196811532E0930781F740E060E08FEF8F", 21 );
	uint16_t startaddr[32];
	char str[5];//, str1[3];
	
	int i=0;
	for(i=0; i<4; i++)
	{
		str[i] = msg_struct->msg[i+2];
	}
	str[i] = '\0';
	 startaddr[0] = strtoul(str, NULL, 16);
	 uint16_t addrstart = startaddr[0];
	printf("page start : %u\n",addrstart);
	uint32_t pagenumber=0;
	pagenumber = calculate_page_number(addrstart);	
	printf("pageNumber %lu\n\r\n\r", pagenumber);
	
	uint32_t page_address = pagenumber*FLASH_PAGE_SIZE;
	flashBufferPos = 38;
	printf("buffer position offset page address to change: %hu\r\n\r\n ", flashBufferPos);
	uint8_t FlashBuffer[4];
	
	
	
	uint32_t targetAddr = 0x10c2UL;
	nvm_flash_read_buffer(targetAddr, FlashBuffer, 4);
	for(int j=0; j<4; j++)
	{
		printf("%02hx ", FlashBuffer[j]);
		//if((j+1)%16==0){
			//printf("\r\n");
		//}	
	}
	printf("\r\n");
	
	
	// keep on filling the buffer
	/*for(uint8_t i=6;i<37;i+=2)    // 0-5 are length and address, the last two char (1 byte) is for checksum
	{
		//convert pair of chars to byte.
		str1[0] = msg_struct->msg[i];
		str1[1] = msg_struct->msg[i+1];
		str1[2] = '\0';
		FlashBuffer[flashBufferPos] = strtoul(str1, NULL, 16);
		flashBufferPos = flashBufferPos + 1;
		// Converting string to hex value is done successfully
	}*/
	
	//str1[0] = '6';
	//str1[1] = 'F';
	//str1[2] = '\0';
	//FlashBuffer[90] = strtoul(str1, NULL, 16);
	//flashBufferPos = flashBufferPos + 1;
	FlashBuffer[0]=0x6F;
	
	//str1[0] = 'E';
	//str1[1] = 'F';
	//str1[2] = '\0';
	//FlashBuffer[91] = strtoul(str1, NULL, 16);
	//flashBufferPos = flashBufferPos + 1;
	FlashBuffer[1]=0xEF;
	
	//str1[0] = '8';
	//str1[1] = '0';
	//str1[2] = '\0';
	//FlashBuffer[92] = strtoul(str1, NULL, 16);
	//flashBufferPos = flashBufferPos + 1;
	FlashBuffer[2]=0x80;
	
	//str1[0] = 'E';
	//str1[1] = '0';
	//str1[2] = '\0';
	//FlashBuffer[93] = strtoul(str1, NULL, 16);
	FlashBuffer[3]=0xE0;
	
	
	
	printf("Printing written FlashBuffer:\r\n\r\n");
	for(int l=0; l<4; l++)
	{
		printf("%02hx ", FlashBuffer[l]);
		//if((l+1)%16==0){
			//printf("\r\n");
		//}
	}
	printf("\r\n");
	
	//if (NumOfDataBtyes != 16){
		// Indicates that page has ended
		// write the page
		//flashBufferPos = 0;
		uint32_t tableAddress = (pagenumber * FLASH_PAGE_SIZE);
		nvm_flash_flush_buffer();
		//printf("About to write. Page Number: %lu, FlashBuffer: %p\r\n\r\n\r\n", pagenumber, FlashBuffer);
		printf("About to write. Address: %lu\r\n\r\n\r\n", targetAddr);
		nvm_flash_erase_and_write_buffer(targetAddr, FlashBuffer, 4, 1);
		
		nvm_flash_read_buffer(targetAddr, FlashBuffer, 4);
		for(int j=0; j<4; j++)
		{
			printf("%02hx ", FlashBuffer[j]);
			//if((j+1)%16==0){
				//printf("\r\n");
			//}
		}
		printf("\r\n");
		//delayMS(20000);
		//setRGB(0,0,255);
		dropletReboot();
}

static void calculateIdNumber(void){
	INIT_DEBUG_PRINT("get id number\r\n");

	uint32_t pgm_bytes = 0;
	uint16_t crc = 0;
	
	// calculate local id
	// simply crc of user sig row
	// likely to be unique and constant for each chip
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;

	uint32_t addrs[16] = {0x00,0x01,0x02,0x03,0x04,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x10,0x12,0x13,0x14,0x15};

	for (uint8_t i = 0; i < 16; i++){
		pgm_bytes = pgm_read_word_far(addrs[i]);
		crc = _crc16_update(crc, (uint16_t)(pgm_bytes&0xFF));
	}

	NVM_CMD = NVM_CMD_NO_OPERATION_gc;

	droplet_ID = crc;
}

static void enableInterrupts(void){
	PMIC.CTRL |= PMIC_LOLVLEN_bm;	// enable low level interrupts
	PMIC.CTRL |= PMIC_MEDLVLEN_bm;	// enable medium level interrupts	(e.g. TXCIF)
	PMIC.CTRL |= PMIC_HILVLEN_bm;	// enable high level interrupts		(e.g. RTC_OVF)
	PMIC.CTRL |= PMIC_RREN_bm;
	sei();
}

void startupLightSequence(){
	setRGB(100,0,0); delayMS(100); setRGB(0,100,0); delayMS(100); setRGB(0,0,100); delayMS(100); ledOff();
	setRGB(100,0,0); delayMS(100); setRGB(0,100,0); delayMS(100); setRGB(0,0,100); delayMS(100); ledOff();
	setRGB(100,0,0); delayMS(100); setRGB(0,100,0); delayMS(100); setRGB(0,0,100); delayMS(100); ledOff();
}

uint8_t getDropletOrd(id_t id){
	switch(id){
		case 0x0000: return 0;
		case 0x0029: return 1;
		case 0x0120: return 2;
		case 0x01A9: return 3;
		case 0x086B: return 4;
		case 0x0B68: return 5;
		case 0x1064: return 6;
		case 0x11D3: return 7;
		case 0x1266: return 8;
		case 0x12AD: return 9;
		case 0x1361: return 10;
		case 0x14AA: return 11;
		case 0x1562: return 12;
		case 0x1767: return 13;
		case 0x18A2: return 14;
		case 0x1927: return 15;
		case 0x2668: return 16;
		case 0x2826: return 17;
		case 0x2C92: return 18;
		case 0x3062: return 19;
		case 0x32A7: return 20;
		case 0x3493: return 21;
		case 0x382E: return 22;
		case 0x392B: return 23;
		case 0x392C: return 24;
		case 0x3B61: return 25;
		case 0x3D6C: return 26;
		case 0x3F6D: return 27;
		case 0x3F9D: return 28;
		case 0x4327: return 29;
		case 0x46A1: return 30;
		case 0x4E2E: return 31;
		case 0x4ED3: return 32;
		case 0x5161: return 33;
		case 0x5264: return 34;
		case 0x5A2F: return 35;
		case 0x5B2F: return 36;
		case 0x5C68: return 37;
		case 0x5D61: return 38;
		case 0x5E60: return 39;
		case 0x5F2D: return 40;
		case 0x5FEC: return 41;
		case 0x6597: return 42;
		case 0x6B6F: return 43;
		case 0x6C66: return 44;
		case 0x6C6F: return 45;
		case 0x6E67: return 46;
		case 0x7022: return 47;
		case 0x7066: return 48;
		case 0x73AF: return 49;
		case 0x75A1: return 50;
		case 0x7D13: return 51;
		case 0x7EDF: return 52;
		case 0x8521: return 53;
		case 0x8625: return 54;
		case 0x896F: return 55;
		case 0x8F9C: return 56;
		case 0x9029: return 57;
		case 0x9261: return 58;
		case 0x92DA: return 59;
		case 0x9363: return 60;
		case 0x9420: return 61;
		case 0x9463: return 62;
		case 0x9495: return 63;
		case 0x9564: return 64;
		case 0x9669: return 65;
		case 0x97A0: return 66;
		case 0xA0D8: return 67;
		case 0xA165: return 68;
		case 0xA250: return 69;
		case 0xA52F: return 70;
		case 0xAF6A: return 71;
		case 0xAFD8: return 72;
		case 0xB122: return 73;
		case 0xB36F: return 74;
		case 0xB41B: return 75;
		case 0xB561: return 76;
		case 0xBC63: return 77;
		case 0xBC6E: return 78;
		case 0xBCB5: return 79;
		case 0xBD2D: return 80;
		case 0xC051: return 81;
		case 0xC32D: return 82;
		case 0xCB64: return 83;
		case 0xCBAB: return 84;
		case 0xCCD1: return 85;
		case 0xCD6B: return 86;
		case 0xCFA1: return 87;
		case 0xD0AE: return 88;
		case 0xD2D7: return 89;
		case 0xD766: return 90;
		case 0xD76C: return 91;
		case 0xD86C: return 92;
		case 0xD913: return 93;
		case 0xDC62: return 94;
		case 0xDC64: return 95;
		case 0xDC9E: return 96;
		case 0xDD21: return 97;
		case 0xDF64: return 98;
		case 0xFA6F: return 99;
		case 0xFCD0: return 100;  
		case 0x2B4E: return 101;	//This is the first AUDIO_DROPLET
		case 0x7D78: return 102;
		case 0x8B46: return 103;
		case 0xC806: return 104;
		case 0x4177: return 105;
		case 0x0A0B: return 106;
		case 0x3B49: return 107;
		case 0x028C: return 108;
		case 0x1F08: return 109;
		case 0xEEB0: return 110;
		case 0xA649: return 111;
		case 0xA5B5: return 112;
		case 0xF60A: return 113;
		case 0xB944: return 114;
		case 0x3405: return 115;
		case 0x43BA: return 116;
		case 0x6648: return 117;
		case 0x1B4B: return 118;
		case 0xC24B: return 119;
		case 0x4DB0: return 120;
	}
	return 0xFF;
}

const uint16_t OrderedBotIDs[121] PROGMEM =
	{0x0000, 0x0029, 0x0120, 0x01A9, 0x086B, 0x0B68, 0x1064, 0x11D3, 0x1266, 0x12AD, 0x1361,
	 0x14AA, 0x1562, 0x1767, 0x18A2, 0x1927, 0x2668, 0x2826, 0x2C92, 0x3062, 0x32A7,
	 0x3493, 0x382E, 0x392B, 0x392C, 0x3B61, 0x3D6C, 0x3F6D, 0x3F9D, 0x4327, 0x46A1,
	 0x4E2E, 0x4ED3, 0x5161, 0x5264, 0x5A2F, 0x5B2F, 0x5C68, 0x5D61, 0x5E60, 0x5F2D,
	 0x5FEC, 0x6597, 0x6B6F, 0x6C66, 0x6C6F, 0x6E67, 0x7022, 0x7066, 0x73AF, 0x75A1,
	 0x7D13, 0x7EDF, 0x8521, 0x8625, 0x896F, 0x8F9C, 0x9029, 0x9261, 0x92DA, 0x9363,
	 0x9420, 0x9463, 0x9495, 0x9564, 0x9669, 0x97A0, 0xA0D8, 0xA165, 0xA250, 0xA52F,
	 0xAF6A, 0xAFD8, 0xB122, 0xB36F, 0xB41B, 0xB561, 0xBC63, 0xBC6E, 0xBCB5, 0xBD2D,
	 0xC051, 0xC32D, 0xCB64, 0xCBAB, 0xCCD1, 0xCD6B, 0xCFA1, 0xD0AE, 0xD2D7, 0xD766,
	 0xD76C, 0xD86C, 0xD913, 0xDC62, 0xDC64, 0xDC9E, 0xDD21, 0xDF64, 0xFA6F, 0xFCD0,
	 0x2B4E, 0x7D78, 0x8B46, 0xC806, 0x4177, 0x0A0B, 0x3B49, 0x028C, 0x1F08, 0xEEB0,
	 0xA649, 0xA5B5, 0xF60A, 0xB944, 0x3405, 0x43BA, 0x6648, 0x1B4B, 0xC24B, 0x4DB0};
	 

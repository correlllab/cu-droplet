#include "reprogram.h"

uint8_t Numberofbytes, message_bytes;
uint8_t calculate_page_number(uint16_t addressFrmProgramming);
uint8_t flashBufferPos=0;
uint32_t tgadd1=0;
uint16_t addrstart;
uint32_t targetAddr;
uint8_t FlashBuffer[512];

uint8_t firstmessage_flag =0;

//TODO: Unsure if this function is still needed; hasn't been udpated.
void send_hex(void){
	uint8_t len = strlen(dataHEX);
	irSend(ALL_DIRS,dataHEX,len);
	NONATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		waitForTransmission(ALL_DIRS);
	}
}

//TODO: Unsure if this function is still needed; hasn't been udpated.
void send_initial(void){
	uint8_t len = strlen(initial_msg);
	irSend(ALL_DIRS,initial_msg,len);
	NONATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		waitForTransmission(ALL_DIRS);
	}
}

// TODO: This function needs to be updated to handle new format of reprogramming messages from python script 
//       (which needs to be updated to set that format, among other things).
void handleReprogMsg(irMsg* msgStruct)
{
	
	number_of_hex--;
	uint16_t startaddr[2];
	char str[3], str1[5];
	int i=0;
	for(i=0; i<2; i++)
	{
		str[i] = msgStruct->msg[i];
	}
	str[2] = '\0';
	message_bytes = strtoul(str, NULL, 16); 
	Numberofbytes += message_bytes;
	printf("Got message number = %d\r\n", number_of_hex);
	for(i=2; i<6; i++)
	{
		str1[i-2] = msgStruct->msg[i];
	}
	str1[4] = '\0';
	 startaddr[0] = strtoul(str1, NULL, 16);
	 if((firstmessage_flag == 0) && (number_of_hex >0))
	 {
		 addrstart = startaddr[0];
		 targetAddr =addrstart;
		 firstmessage_flag = 1;			//condition indicating first message is already received. following messages won't hold starting address
	 }
	
	//nvm_flash_read_buffer(targetAddr, FlashBuffer, Numberofbytes);
	/*for(int j=0; j<Numberofbytes; j++)
	{
		printf("%02hx ", FlashBuffer[j]);

	}
	printf("\r\n");*/
	
	
	// keep on filling the buffer
	for(uint8_t i=6;i<(6+(2*message_bytes));i+=2)    // 0-5 are length and address, the last two char (1 byte) is for checksum
	{
		//convert pair of chars to byte.
		str[0] = msgStruct->msg[i];
		str[1] = msgStruct->msg[i+1];
		str[2] = '\0';
		FlashBuffer[flashBufferPos] = strtoul(str, NULL, 16);
		flashBufferPos = flashBufferPos + 1;
		// Converting string to hex value is done successfully
	}
	

	if(number_of_hex == 0)
	{
		printf("The whole buffer before writing in memory : \r\n\r\n");
		for(int j=0; j<Numberofbytes; j++)
		{
			printf("%02hx ", FlashBuffer[j]);
			if((j%15 == 0)&&(j != 0)) printf("\r\n");
		}
		printf("\r\n");
		printf("About to write. Address: %lu and line num : %d\r\n\r\n\r\n", targetAddr, number_of_hex);
		nvm_flash_erase_and_write_buffer(targetAddr, FlashBuffer, Numberofbytes, 1);
		delayMS(500);
		printf("About to write. Address: %lu\r\n\r\n", targetAddr);
		nvm_flash_read_buffer(targetAddr, FlashBuffer, Numberofbytes);
		for(int j=0; j<Numberofbytes; j++)
		{
			printf("%02hx ", FlashBuffer[j]);
			if((j%15 == 0)&&(j != 0)) printf("\r\n");
		}
		printf("\r\n");
		
		reprogramming=0;
		printf("Came in num hex is zero!!!!!!!!!!!!!!!!!!!!");
		firstmessage_flag =0;			// Done with all messages
		Numberofbytes=0;
		flashBufferPos=0;
		memset(FlashBuffer, 0, 512);
		dropletReboot();
		
	}
}
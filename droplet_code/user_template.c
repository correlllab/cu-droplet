#include "user_template.h"
#include "sp_driver.h"
#include <stdio.h>

/*uint8_t data[512];
uint8_t read_data[512];
uint32_t address =0x0004;
uint8_t byte;
/*
 * any code in this function will be run once, when the robot starts.
 */

uint8_t flashBufferPos=0;


void init(){
		
	//delayMS(5000);
	setRGB(255,0,0);
	irMsg msg_struct[sizeof(irMsg)];
	strncpy(msg_struct->msg, "101220CDBFDEBF40E06FEF80E00E94682F25E178", 21 );
	uint16_t startaddr[32];
	char str[5], str1[3];
	
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
	printf("pageNumber %ld\n\r\n\r", pagenumber);
	
	uint32_t page_address = pagenumber*FLASH_PAGE_SIZE;
	flashBufferPos = addrstart-page_address;
	printf("buffer position offset page address to change: %d\r\n\r\n ", flashBufferPos);
	uint8_t FlashBuffer[512];
	FLASH_ReadFlashPage(FlashBuffer, pagenumber);
	for(int j=0; j<512; j++)
	{
		printf("%02hx ", FlashBuffer[j]);
		if((j+1)%16==0){
			printf("\r\n");
		}	
	}
	
	// keep on filling the buffer
	/*for(uint8_t i=6;i<19;i+=2)    // 0-5 are length and address, the last two char (1 byte) is for checksum
	{
		//convert pair of chars to byte.
		str1[0] = msg_struct->msg[i];
		str1[1] = msg_struct->msg[i+1];
		str1[2] = '\0';
		FlashBuffer[flashBufferPos] = strtoul(str1, NULL, 16);
		flashBufferPos = flashBufferPos + 1;
		// Converting string to hex value is done successfully
	}*/
	
	str1[0] = '6';
	str1[1] = 'F';
	str1[2] = '\0';
	FlashBuffer[38] = strtoul(str1, NULL, 16);
	flashBufferPos = flashBufferPos + 1;
	
	str1[0] = 'E';
	str1[1] = 'F';
	str1[2] = '\0';
	FlashBuffer[39] = strtoul(str1, NULL, 16);
	flashBufferPos = flashBufferPos + 1;
	
	str1[0] = '8';
	str1[1] = '0';
	str1[2] = '\0';
	FlashBuffer[40] = strtoul(str1, NULL, 16);
	flashBufferPos = flashBufferPos + 1;
	
	str1[0] = 'E';
	str1[1] = '0';
	str1[2] = '\0';
	FlashBuffer[41] = strtoul(str1, NULL, 16);
	
	
	
	printf("Printing loaded FlashBuffer:\r\n\r\n");
	for(int l=0; l<512; l++)
	{
		printf("%02hx ", FlashBuffer[l]);
		if((l+1)%16==0){
			printf("\r\n");
		}
	}
	
	//if (NumOfDataBtyes != 16){
		// Indicates that page has ended
		// write the page
		//flashBufferPos = 0;
		printf("About to write. Page Number: %ld, FlashBuffer: %p\r\n", pagenumber, FlashBuffer);
		writeRead(FlashBuffer, pagenumber);
		//delayMS(20000);
		//setRGB(0,0,255);
	
	
	
}


/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){

}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handleMsg(irMsg* msgStruct){

}

///*
 //*	the function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. it should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t userHandleCommand(char* command_word, char* command_args){
	//return 0;
//}
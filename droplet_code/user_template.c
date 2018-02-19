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
void init(){
		
	//delayMS(5000);
	setRGB(255,0,0);
	irMsg msg_struct[sizeof(irMsg)];
	uint16_t startaddr[32];
	char str[5];
	str[0] = '1';
	str[1] = '2';
	str[2] = '1';
	str[3] = '0';
	str[4] = NULL;
	 startaddr[0] = strtoul(str, NULL, 16);
	 uint16_t pagestart = startaddr[0];
	printf("pagestart : %u\n",pagestart);
	uint32_t pagenumber = calculate_page_number(pagestart);	
	printf("pageNumber %d\n\r\n\r", pagenumber);
	uint8_t FlashBuffer[512];
	FLASH_ReadFlashPage(FlashBuffer, pagenumber);
	for(int j=0; j<32; j++)
	{
		printf("%02hx ", FlashBuffer[j]);
		if((j+1)%16==0){
			printf("\r\n");
		}	
	}
	/*SP_LoadFlashPage(data);
	SP_WaitForSPM();
	SP_EraseWriteApplicationPage(0);
	NVM.CMD = NVM_CMD_ERASE_WRITE_APP_PAGE_gc;
	NVM_EXEC();
	NVM.CMD = NVM_CMD_NO_OPERATION_gc;
	
	
	FLASH_ReadFlashPage(read_data, 0);
	//byte = SP_ReadByte(address);
	for(int i=0; i<512; i++){
		printf("%02hx ", read_data[i]);
		if((i+1)%16==0){
			printf("\r\n");
		}
	}*/
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
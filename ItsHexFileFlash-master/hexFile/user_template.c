#include "user_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */

void init()
{
	set_rgb(255,255,255);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */

void loop()
{
	
}

// uint16_t hexStrtoint(uint8_t *str, uint8_t length)
// {
// 	uint16_t num = 0;
// 	uint8_t i=0;
// 
// 	for (i = 0; i<length; i++)
// 	{
// 		if (str[i]>='0' && str[i]<='9')
// 		{
// 			num = num * 16 + str[i] - '0';  // Converts String to integer
// 		}
// 
// 		else if (str[i]>='a' && str[i]<='f')
// 		{
// 			num = num * 16 + str[i] - 'a' + 10;  // Converts String to integer
// 		}
// 
// 		else if (str[i]>='A' && str[i]<='F')
// 		{
// 			num = num * 16 + str[i] - 'A' + 10;  // Converts String to integer
// 		}
// 
// 		else
// 		{
// 			return -1;
// 		}
// 	}
// 	return num;
// }
/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */


// void handle_msg(ir_msg* msg_struct)
// {		
// 	// Used for debugging
// 	msg_struct->msg[msg_struct->length] = '\0';
// 	printf("%s\n\r",msg_struct->msg); 
// 	set_rgb(50,50,50);
// 	
// 	// converting to integer
// 	uint8_t transmitLength = msg_struct->length/2;//  strlen(msg_struct->msg)/2;  
// 	uint8_t lengthCommand = msg_struct->length; // strlen(msg_struct->msg);
// 	
// 	
// 	char str[3];
// 	//bzero(str, sizeof(str));	// Finding the length of the data in the message 
// 	memset(str,0,sizeof(str));
// 	str[0] = msg_struct->msg[0];
// 	str[1] = msg_struct->msg[1];
// 	str[2] = '\0';
// 	uint8_t lengthData = strtoul(str, NULL, 16);
// 	//uint8_t lengthData = hexStrtoint(str, 2);
// 	char strforAddr[5];
// 	// Finding the address 
// 	strforAddr[0] = msg_struct->msg[2];
// 	strforAddr[1] = msg_struct->msg[3];
// 	strforAddr[2] = msg_struct->msg[4];
// 	strforAddr[3] = msg_struct->msg[5];
// 	strforAddr[4] = '\0';
// 	
// 	Startaddr[addCounter] =  strtoul(strforAddr, NULL, 16);
// 	//Startaddr[addCounter] = hexStrtoint(Startaddr, 2);
// 	printf("corresponding address %u\n\r", Startaddr[addCounter]);
// 	addCounter = addCounter + 1;
// 	printf("add counter %hu \n\r", addCounter);
// 
// 	for(uint8_t i=6;i<lengthCommand-2;i+=2)    // 0-5 are length and address, the last two char (1 byte) is for checksum
// 	{
// 		//convert pair of chars to byte.
// 		str[0] = msg_struct->msg[i];
// 		str[1] = msg_struct->msg[i+1];
// 
// 		FlashBuffer[flashBufferPos] = strtoul(str, NULL, 16);
// 		//FlashBuffer[flashBufferPos] = hexStrtoint(str, 2);
// 		flashBufferPos = flashBufferPos + 1;
// 		// Converting string to hex value is done successfully
// 	}
// 	
// 	// Writing it onto flash
// 	// Used for checking contents of the Flash buffer
// 	if (addCounter%32 == 0 && addCounter>0)
// 	{
// 
// 			addCounter = 0;
// 			flashBufferPos = 0;
// 			writeRead(FlashBuffer, 13);
// 			delay_ms(1000);
// 			writeRead(FlashBuffer, 16);
// 
// 			pageTowrite++;       // Incrementing the address to write into next page
// 	}
// 	
// 	set_rgb(0,0,0);	
// }

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection.
 //*/
 
// TODO: data_pointer has possible currency issues. Add Semaphore.
// int16_t counterForTransmit = -1; 
 
//  void send_code_packet(){
// 	set_rgb(255,0,0);
// 	uint8_t length = strlen(data_pointer);
// 
// 	ir_send(ALL_DIRS,data_pointer,length);
// 					//if (counterForTransmit%32 == 0 && counterForTransmit > 0)
// 					//{
// 					//set_rgb(0,255,0);
// 					//delay_ms(2000);
// 					//}
// 	waitForTransmission(ALL_DIRS);
// 	printf("Done\r\n");
// 	set_rgb(0,0,0);
//  }
 
 //send multiple data and receive

uint8_t user_handle_command(char* command_word, char* command_args) 
{
	
// 	if (command_word[0]=='!' && command_word[1]==0)
// 	{		
// 		//TODO: data_pointer has possible currency issues. Add Semaphore.
// 		strcpy(data_pointer, command_args);
// 		schedule_task(20, send_code_packet, NULL); 
// 				
// 		return 1;
// 	}
	return 0;	
}

///*
 //*	The function below is optional - if it is commented in, and the leg interrupts have been turned on
 //*	with enable_leg_status_interrupt(), this function will get called when that interrupt triggers.
 //*/	
//void	user_leg_status_interrupt()
//{
	//
//}
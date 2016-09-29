#include "user_template.h"
#include "flash.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
uint32_t data = 55;
void init()
{
	writeRead();
	 /*uint8_t buffer[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
	nvm_write(INT_FLASH, test_address, (void *)buffer,
	sizeof(buffer));
	if(nvm_read(INT_FLASH, test_address, (void *)data_read, sizeof(data_read))
	== STATUS_OK) {
		//Check read content
		if(data_read[0] == 0xAA){printf("Success\n\r");}
			else{printf("Fail\n\r");}
	}*/
}


/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
int i = 0;
void loop()
{
	//writeRead();
	
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{

}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args)
//{
	//return 0;
//}

///*
 //*	The function below is optional - if it is commented in, and the leg interrupts have been turned on
 //*	with enable_leg_status_interrupt(), this function will get called when that interrupt triggers.
 //*/	
//void	user_leg_status_interrupt()
//{
	//
//}
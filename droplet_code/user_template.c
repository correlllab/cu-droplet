#include "user_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	//broadcast_rnb_data();
	delay_ms(500);
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{
	
}

/*
 * The function below is optional, and need not be included. If included,
 * it should return '1' if the command_word is a valid command, and '0', 
 * otherwise.
 */
uint8_t user_handle_command(char* command_word, char* command_args)
{
	
}

#include "user_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	task_one();
	delay_ms(121);
	task_two();	
	delay_ms(121);
	task_three();
	delay_ms(121);
	task_four();
}

const char* const_msg = "abcdefghijklmnopqrstuvwxyz";

void task_one()
{
	printf("\tBroadcast Task\r\n");
	broadcast_rnb_data();	
	//ir_send(ALL_DIRS, const_msg, 26);
	//wait_for_ir(ALL_DIRS);
	schedule_task(512, task_one, NULL);
}

void task_two()
{
	printf("\tRnB Task\r\n");
	broadcast_rnb_data();
		////ir_send(ALL_DIRS, const_msg, 26);
	//wait_for_ir(ALL_DIRS);		
	schedule_task(2187,task_two,NULL);
}

void task_three()
{
	printf("\tTask Three\r\n");	
		ir_send(ALL_DIRS, const_msg, 26);
	//delay_ms(100);	
	//handle_cmd_wrapper();
	//wait_for_ir(ALL_DIRS);
	schedule_task(625, task_three, NULL);
}

void task_four()
{	
	printf("\tTask Four\r\n");
	ir_send(ALL_DIRS, const_msg, 26);
	//delay_ms(100);	
	//wait_for_ir(ALL_DIRS);
	schedule_task(343,task_four,NULL);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	//printf("?\r\n");
	delay_ms(500);
	//delay_ms(2000);
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

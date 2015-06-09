#include "user_template.h"

uint8_t happened;

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	motor_adjusts[0][1] = 10;
	motor_adjusts[0][2] = -238;
	
	get_mic_reading(); //first reading tends to be very high?
	//delay_ms(3000);
	happened=0;
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{	
	//if(happened) return;
	delay_ms(500);
	printf("{\r\n");
	//mic_recording(2000,8000);
	printf(",\r\n");
	delay_ms(1000);
	move_steps(0,100);
	//delay_ms(2000);
	//mic_recording(2000,8000);
	printf("\r\n");
	delay_ms(2000);
	stop_move();
	printf("\r\n");
	happened=1;
	delay_ms(5000);
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

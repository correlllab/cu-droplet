#include "user_template.h"

#define LOOP_PERIOD 50
#define BLINK_PERIOD 3000
#define BLINK_DURATION 300
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
	if((get_time()/LOOP_PERIOD)%(BLINK_PERIOD/LOOP_PERIOD)==0)
	{
		set_rgb(255,255,255);
	}
	else if((get_time()/LOOP_PERIOD)%(BLINK_PERIOD/LOOP_PERIOD)==(BLINK_DURATION/LOOP_PERIOD))
	{
		set_rgb(0,0,0);	
	}
	delay_ms(LOOP_PERIOD);
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

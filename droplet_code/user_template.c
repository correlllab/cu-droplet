#include "user_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	//for(uint8_t i=0;i<6;i++) ir_led_on(i);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	if(rnb_updated)
	{
	printf("range: %f, bearing: %f, heading: %f\r\n", last_good_rnb.range, rad_to_deg(last_good_rnb.bearing), rad_to_deg(last_good_rnb.heading));
	rnb_updated=0;
	}
	//broadcast_rnb_data();
	//delay_ms(2000);
	//char msg[6] = "Hello!";
	//ir_send(DIR4, msg, 6);
	//delay_ms(1000);
	//broadcast_rnb_data();
	//delay_ms(5000);
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

#include "user_template.h"

uint8_t turning_on = 1;
uint16_t main_cca;

void enable_sync_blink(uint16_t phase_offset_ms){
	main_cca = ((uint16_t)(phase_offset_ms*FFSYNC_MS_CONVERSION_FACTOR)%FFSYNC_FULL_PERIOD);
	TCE0.CCA = main_cca;
	TCE0.INTCTRLB = TC_CCAINTLVL_MED_gc;
	turning_on = 1;
}

ISR(TCE0_CCA_vect){
	if(turning_on){
		set_rgb(150,0,50);
		TCE0.CCA = (TCE0.CCA+((uint16_t)(300*FFSYNC_MS_CONVERSION_FACTOR)))%FFSYNC_FULL_PERIOD;
		turning_on = 0;
	}else{
		set_rgb(0,0,0);
		TCE0.CCA = main_cca;
		turning_on = 1;
	}
}


/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	enable_sync_blink(100);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	if(rand_byte()==0){
		broadcast_rnb_data();
	}
	if(rnb_updated){
		printf("Got RNB!\r\n");
		rnb_updated = 0;
	}
	delay_ms(40);
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


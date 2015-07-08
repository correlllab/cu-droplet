#include "droplet_programs/firefly.h"

void init()
{
	float b=2.;
	float eps = .25;
	
	TCD1.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCD1.CTRLB = TC_WGMODE_NORMAL_gc;
	TCD1.CCA = 0;
	TCD1.CCB = 0;
	TCD1.CNT = 0;
	TCD1.PER =  63323;	
	TCD1.INTCTRLA = TC_OVFINTLVL_MED_gc;
}

ISR(TCD1_OVF_vect)
{
	ir_cmd(ALL_DIRS, "<3", 2);
	set_hsv(30, 255, 120);
}

void loop()
{
	if(TCD1.CNT>REFRACTORY_PERIOD) led_off();
}

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
	if(strcmp(command_word, "<3") == 0)
	{
		uint16_t the_count = TCD1.CNT;
		if(the_count>REFRACTORY_PERIOD)
		{
			double alpha = exp(b * eps);
			double beta = (alpha - 1) / (exp(b) - 1);
			uint16_t new_count = (uint16_t)(fmin(alpha * the_count/FULL_PERIOD + beta, 1.) * ((float)FULL_PERIOD));
			TCD1.CNT = new_count;
		}
		return 1;
	}
	return 0;
}

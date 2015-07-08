#include "user_template.h"

static uint32_t last_time;

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	last_time = get_time();
}


/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	
	if(get_time()%2048<512){
		if(!get_red_led())		set_red_led(255);
	}else if(get_time()%2048<1024){
		if(get_red_led())		set_red_led(0);
	}else if(get_time()%2048<1536){
		if(!get_green_led())	set_green_led(255);
	}else if(get_green_led())	set_green_led(0);

	//if((TCE0.CNT>REFRACTORY_PERIOD)&&get_red_led()){
		//led_off();
		//uint32_t the_time = get_time();
		//printf("!! %lu !!\r\n",the_time-last_time);
		//last_time = the_time;
	//}
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{
	
}

///*
 //* The function below is optional, and need not be included. If included,
 //* it should return '1' if the command_word is a valid command, and '0', 
 //* otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args)
//{
	//return 0;
//}

#include "user_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	printf("Test.\r\n");
	i2c_count=0;
	i2c_state=RGB_SENSE_POWER;
	TWIE_MASTER_ADDR = RGB_SENSE_ADDR;
	delay_ms(500);
	i2c_state=NONE;
	delay_ms(500);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	uint16_t r, g, b, c;
	get_rgb(&r, &g, &b, &c);
	delay_ms(300);
	set_all_ir_powers(256);
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

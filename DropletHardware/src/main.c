#include "main.h"

volatile float blah;

int main(void)
{
	init_all_systems();
	//uint8_t ir_sense_vals[6];
	blah = 1.6180339887;
	printf("Initialized.\r\n");
	while (1)
	{	
		//printf("1: %hhu, 2: %hhu, 3: %hhu\r\n",leg1_status(), leg2_status(), leg3_status());
		//printf("Test print message.\r\n");
		//for(uint8_t i=0; i<6; i++) ir_sense_vals[i] = get_IR_sensor(i);
		//printf("ir0: %hhu", get_IR_sensor(0));
		//for(uint8_t i=1; i<6; i++) printf(", ir%hhu: %hhu", i, get_IR_sensor(i));
		//printf("\r\n");
		//printf("Beep.\r\n");
		
		//blah = 1.0/blah;
		////blah = cos(blah);
		////printf("A float: %f\r\n",blah);
		//set_green_led(10);
		//delay_ms(500);
		//set_green_led(0);
		//delay_ms(500);
		////check_messages();	
	
		//if(leg1_status()==1) set_red_led(15);
		//else set_red_led(0);
		//
		//if(leg2_status()==1) set_green_led(15);
		//else set_green_led(0);
		//
		//if(leg3_status()==1) set_blue_led(15);
		//else set_blue_led(0);
	}
}

void check_messages()
{
	if(check_for_new_messages()==1)
	{	
		/* Do whatever you want with incoming message, stored in array
		 * global_rx_buffer.buf[i] from i=0 to i<global_rx_buffer.data_len
		 */
		char tempStr[64];
		memcpy(tempStr, global_rx_buffer.buf, global_rx_buffer.data_len);
		tempStr[global_rx_buffer.data_len] = NULL;
		uint16_t senderID = global_rx_buffer.sender_ID;
		//tempStr contains the message.
		global_rx_buffer.read = 1;
	}
	if(rnb_updated==1)
	{
		//do stuff with last_good_rnb
		rnb_updated=0;
	}
}

void color_cycle()
{
	for(uint16_t i=0;i<=360;i++)
	{
		set_hsv(i, 255, 10);
		delay_ms(10);
	}
}


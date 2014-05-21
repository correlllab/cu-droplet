#include "main.h"

int main(void)
{
	init_all_systems();
	//uint8_t ir_sense_vals[6];
	printf("Initialized.\r\n");
	while (1)
	{	
		//printf("Test print message.\r\n");
		//for(uint8_t i=0; i<6; i++) ir_sense_vals[i] = get_IR_sensor(i);
		printf("ir0: %hhu", get_IR_sensor(0));
		for(uint8_t i=1; i<6; i++) printf(", ir%hhu: %hhu", i, get_IR_sensor(i));
		printf("\r\n");
		delay_ms(200);
		check_messages();	
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


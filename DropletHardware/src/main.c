#include "main.h"

int main(void)
{
	init_all_systems();

	printf("Initialized.\r\n");
	// Set theta and tau initially
	rgb_sensor_enable();
	while (1)
	{	
		int16_t red = get_red_sensor();
		int16_t green = get_green_sensor();
		int16_t blue = get_blue_sensor();
		printf("R: %hd, G: %hd, B: %hd\r\n",red,green,blue);
		delay_ms(100);
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


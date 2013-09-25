#include "main.h"

//Experiment with scanf
int main(void)
{
	init_all_systems();
	
	printf("Initialized.\r\n");

	uint32_t last_time = get_32bit_time();
	uint32_t curr_time;
	uint8_t chan = 0;
	
	// Init the global serial out buffer for the "data" command
	memset(serial_data_out_buffer.data, 0, BUFFER_SIZE);
	serial_data_out_buffer.length = 0;
		
	while (1)
	{	
		//check_messages(); //Delete cautiously.
		if(strcmp(serial_data_out_buffer.data, "on") == 0)
		{
			set_rgb(10, 10, 10);
		}
		else if(strcmp(serial_data_out_buffer.data, "off") == 0)
		{
			led_off();
		}
	}
}

void check_messages()
{
	if(check_for_new_messages()==1)
	{
		//Do whatever you want with incoming messages.
		for(uint16_t i = 0 ; i<global_rx_buffer.data_len ; i++)
		{
			printf("%c", global_rx_buffer.buf[i]);
		}
		printf("\r\n");
		if(rnb_updated==1)
		{
			//do stuff with last_good_rnb
			printf("%s",last_good_rnb.range);
		}
		global_rx_buffer.read = 1;
	}
}

#include <avr/io.h>
#include "droplet_init.h"

void act_on_messages()
{
	if(check_for_new_messages())
	{
		global_rx_buffer.read = 1;		// mark new message as read
		global_rx_buffer.buf[global_rx_buffer.data_len] = '\0'; // write null terminator
		printf("Got message: %s\r\n",global_rx_buffer.buf);
		switch (global_rx_buffer.buf[0])
		{
			case 'X': // X test message
			printf("got test message\r\n");
			break;
		}
	}
}

void main()
{
	init_all_systems();
	
	printf("Droplet Initialized.\r\n");
	
	while (1)
	{
// 		delay_ms(1000);
// 		print_task_queue();
		act_on_messages();
	}		
}	
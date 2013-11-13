#include "main.h"

int main(void)
{
	init_all_systems();
	
	printf("Initialized.\r\n");
	//pulse();
	while (1)
	{	
		/* Not including this line seems to break
		 * recieving broadcast commands on this droplet. 
		 * TODO: Why? Fix it.
		 */
		check_messages();		
	}
}

void pulse(){
	set_rgb(50,50,0);
	uint8_t data[2];
	data[0] = 0xFF & (droplet_ID>>8);
	data[1] = 0xFF & droplet_ID;
	ir_broadcast(data, 2);
	delay_ms(100);
	set_rgb(0,0,0);
	schedule_task(5000, pulse, NULL);
}

void check_messages()
{
	if(check_for_new_messages()==1)
	{	
		/* Do whatever you want with incoming message, stored in array
		 * global_rx_buffer.buf[i] from i=0 to i<global_rx_buffer.data_len
		 */
		set_rgb(0,60,40);
		char tempStr[64];
		memcpy(tempStr, global_rx_buffer.buf, global_rx_buffer.data_len);
		tempStr[global_rx_buffer.data_len] = NULL;
		uint16_t senderID = global_rx_buffer.sender_ID;
		printf("Sender ID: %02X, Message: %s\r\n",senderID,tempStr);
		global_rx_buffer.read = 1;
		delay_ms(100);
		set_rgb(0,0,0);
	}
	if(rnb_updated==1)
	{
		//do stuff with last_good_rnb
		rnb_updated=0;
	}
}

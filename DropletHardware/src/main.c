#include "main.h"

int main(void)
{
	init_all_systems();
	
	printf("Initialized.\r\n");
	
	while (1)
	{	
		/* Not including this line seems to break
		 * recieving broadcast commands on this droplet. 
		 * TODO: Why? Fix it.
		 */
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
		global_rx_buffer.read = 1;
	}
	if(rnb_updated==1)
	{
		//do stuff with last_good_rnb
		rnb_updated=0;
	}
}

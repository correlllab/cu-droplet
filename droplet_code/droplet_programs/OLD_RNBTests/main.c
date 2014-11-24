#include "main.h"

//Experiment with scanf
int main(void)
{
	init_all_systems();
	
	printf("Initialized.\r\n");

	uint32_t last_time = get_32bit_time();
	uint32_t curr_time;
	uint8_t chan = 0;
	got_rnb_cmd_flag = 0;
	
	while (1)
	{	
		check_messages(); //Not including this line seems to break recieving broadcast commands on this droplet. TODO: Why? Fix it.
	 	if(rnb_updated==1)
	 	{
			 
		 	//do stuff with last_good_rnb
		 	printf("{Range-> %f,Bearing-> %f,Heading-> %f,ID-> %u,\\[Lambda]->",
			 last_good_rnb.range, last_good_rnb.bearing*180/M_PI, 
			 last_good_rnb.heading*180/M_PI, last_good_rnb.id_number);
		printf("{");
		for(uint8_t emitter_num=0 ; emitter_num<6 ; emitter_num++)
		{
			printf("{");
			for(uint8_t sensor_num=0 ; sensor_num<6 ; sensor_num++)
			{
				printf("%u",last_good_rnb.brightness_matrix_ptr[emitter_num][sensor_num]);
				if(sensor_num<5) printf(",");
			}
			printf("}");
			if(emitter_num<5) printf(",");
		}
		printf("}}\r\n");
		rnb_updated=0;
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
		global_rx_buffer.read = 1;
	}	
}

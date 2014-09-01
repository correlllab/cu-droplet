#include "main.h"
#include "flash_api.h"

volatile float blah;

int main(void)
{
	uint16_t pg_no = 0;
	init_all_systems();
	printf("Initialized.\r\n");
	while (1)
	{	
		//check_messages();
		//delay_ms(100);
		//check_collisions();
		for(uint8_t i=0;i<6;i++) printf("%hhu ", get_IR_sensor(i));
		//uint8_t randDir = rand_byte()%6;
		//int8_t meas = get_IR_sensor(randDir);
		//printf("| %hhu: %3hhd ",randDir, meas);
		//if(randDir==5) printf("\r\n");
		printf("\r\n");
		delay_ms(500);
		//set_rgb(200,0,0);
		//delay_ms(5000);
		//set_rgb(0,200,0);
		//delay_ms(5000);
		//set_rgb(0,0,200);
		//delay_ms(5000);
	}
}

//void turn_on_emitters()
//{
	//if(emitters_on=(!emitters_on))
	//{
		//for(uint8_t i=0;i<6;i++) ir_blast(1000); 
		//set_rgb(20,0,0); 
	//}
	//else
	//{
		//set_rgb(0,0,0);
	//}	
	//schedule_task(1000,turn_on_emitters, NULL);
//}

void check_messages()
{
	uint8_t num_msgs = 0;
	while(last_ir_msg!=NULL)
	{
		printf("\tGot: \"");
		for(uint8_t i=0; i<last_ir_msg->msg_length; i++)
		{
			printf("%c",last_ir_msg->msg[i]);
		}
		printf("\"\r\n\tfrom %hx %ums ago.\r\n",last_ir_msg->sender_ID, get_32bit_time()-last_ir_msg->arrival_time);
		msg_node* temp = last_ir_msg;
		last_ir_msg = last_ir_msg->prev;
		free(temp->msg);
		free(temp);
		num_msgs++;
	}
	if(num_msgs>0) printf("Got %hhu messages.\r\n",num_msgs);
}

void color_cycle()
{
	for(uint16_t i=0;i<=360;i++)
	{
		set_hsv(i, 255, 10);
		delay_ms(10);
	}
}


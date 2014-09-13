#include "main.h"

#define ROLL_AVG_WINDOW_SIZE 10.0
#define LIGHT_VAL_DIFF_THRESH 25.0
#define SINGLE_WAVE_WINDOW 200

int main(void)
{
	init_all_systems();
	
	printf("Initialized.\r\n");
	float lightVal;
	float rollingLightVal=0.0;
	int8_t lastMoveDir, curMoveDir;
	uint32_t lastWave = get_32bit_time();
	while (1)
	{	
		lightVal = (float)((uint16_t)get_red_sensor() + (uint16_t)get_green_sensor() + (uint16_t)get_blue_sensor());
		rollingLightVal*=(ROLL_AVG_WINDOW_SIZE-1.0)/ROLL_AVG_WINDOW_SIZE;
		rollingLightVal+= lightVal/ROLL_AVG_WINDOW_SIZE;

		if(((rollingLightVal-lightVal)>LIGHT_VAL_DIFF_THRESH)&&((get_32bit_time()-lastWave)>SINGLE_WAVE_WINDOW))
		{
			lastWave = get_32bit_time();
			handle_wave();
		}
		
		if(is_moving()<0) set_rgb(0,0,0);
		
		check_messages();	
		delay_ms(10);
	}
}

void handle_wave()
{
	uint16_t rand_color = (((uint16_t)rand_byte())<<8)|((uint16_t)rand_byte());
	rand_color = rand_color%360;
	set_hsv(rand_color,255,20);
	int8_t moveDir = is_moving();
	if(moveDir<0){
		if(rand_byte()%3==0) move_steps(0,100);
		else move_steps(6+(rand_byte()%2),100);
	}
}

void check_messages()
{
	if(check_for_new_messages()==1)
	{	
		/* Do whatever you want with incoming message, stored in array
		 * global_rx_buffer.buf[i] from i=0 to i<global_rx_buffer.data_len
		 */
		//set_rgb(0,60,40);
		//char tempStr[64];
		//memcpy(tempStr, global_rx_buffer.buf, global_rx_buffer.data_len);
		//tempStr[global_rx_buffer.data_len] = NULL;
		//uint16_t senderID = global_rx_buffer.sender_ID;
		//printf("Sender ID: %02X, Message: %s\r\n",senderID,tempStr);
		global_rx_buffer.read = 1;
		//delay_ms(100);
		//set_rgb(0,0,0);
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
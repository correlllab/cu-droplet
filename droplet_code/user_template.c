#include "user_template.h"

uint32_t last_rnb_broadcast;
char motor_values_of_another_droplet[120];
int8_t receiving_motor_settings_from_another_droplet_flag=0;
int8_t sign_bit;
int16_t motor_settings_received_count=0;
int16_t motor_signs_count=0;
/*
 * Any code in this function will be run once, when the robot starts.
 */
void init(){
	set_rgb(0,0,255);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */

void loop(){

	if(get_time()-last_rnb_broadcast > 3000){
		broadcast_rnb_data();
		last_rnb_broadcast=get_time();
	}
//
	if(rnb_updated){
		//printf("RANGE %d Bearing %d Heading %d\n\r",last_good_rnb.range,last_good_rnb.bearing,last_good_rnb.heading);
		//edge_following(last_good_rnb.range,last_good_rnb.bearing);
		if(last_good_rnb.bearing)
		rnb_updated = 0; //Note! This line must be included for things to work properly.
	}

}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){
	if(receiving_motor_settings_from_another_droplet_flag==1)
	{
		if((msg_struct->msg[0]=='+')||(msg_struct->msg[0]=='-')){
		motor_signs_count++;
		motor_values_of_another_droplet[motor_settings_received_count]=msg_struct->msg[0];
		motor_settings_received_count++;
		}
		
		else{
			motor_values_of_another_droplet[motor_settings_received_count]=msg_struct->msg[0];
			motor_settings_received_count++;
		}
		
		if(motor_signs_count==24){

			int temp_count=motor_settings_received_count;
			//printf("DONE\n\r");
			motor_signs_count=0;
		receiving_motor_settings_from_another_droplet_flag=0;
		
		while(motor_settings_received_count>=0){
			if((motor_values_of_another_droplet[temp_count-motor_settings_received_count]=='+') || (motor_values_of_another_droplet[temp_count-motor_settings_received_count]=='-'))
			motor_signs_count++;
			if((motor_signs_count%3==0)&&((motor_values_of_another_droplet[temp_count-motor_settings_received_count+1]=='+') || (motor_values_of_another_droplet[temp_count-motor_settings_received_count+1]=='-')))
			printf("\n\r");
			printf("%c ",motor_values_of_another_droplet[temp_count-motor_settings_received_count]);	
			motor_settings_received_count--;
		}
		
		//custom_atoi(motor_values_of_another_droplet);
		}
	}

	set_rgb(255,255,255);
	//printf("Time :%lu\n\r",msg_struct->arrival_time);
	//printf("Got message of length %hu from %04X:\r\n\t",msg_struct->length, msg_struct->sender_ID);
	//printf("RANGE%d",last_good_rnb.range);
	//for(uint8_t i=0;i<msg_struct->length;i++) printf("\n\r%c ",msg_struct->msg[i]);
	//if(motor_signs_count%3 == 0) printf("\n\r");
	if(msg_struct->msg[0]=='!')// MOTOR SETTINGS RECEIVED
	{
		receiving_motor_settings_from_another_droplet_flag=1;
	}
	
	//printf("\r\n");
}

void edge_following(int16_t range, int16_t bearing)
{
	if(range>50){
		if((0<=bearing) && (bearing<=40))
			move_steps(0,20);
		else if((40<=bearing) && (bearing<=70))
			move_steps(7, 20);
		else if((70<=bearing) && (bearing<=90))
			move_steps(7, 20);
	}
}
void custom_atoi(char* c){
	while(motor_signs_count!=24){
		if((*c)=='+')
		{
			motor_signs_count++;
			sign_bit=0;//positive number
		}
		else if((*c)=='-')
		{
			motor_signs_count++;
			sign_bit=1;//negative number
		}
		while((*(c+1)!='+')||(*(c+1)!='-'))
		{
			c++;	
		}	
	}	
}
char *custom_itoa(int value)
{
	int sze=0;
	uint8_t neg_value=0;
	if(value<0){
		neg_value=1;
		value=value*(-1);
	}
	int tempp_val=value;
	if(tempp_val==0)
	sze=1;
	else{
	 while(tempp_val > 0)
  {
     tempp_val = tempp_val / 10;
     sze = sze + 1;  
  }
	}

	char* temp_val;
	temp_val=(char*)malloc((sze+1)/sizeof(char)); 
	int motor_cnt=0;
	
		while(motor_cnt<sze)
		{
		 temp_val[sze-motor_cnt]=value%10+'0';
		value=value/10;
		//printf("motor:%c\r\n", temp_val[sze-motor_cnt]);
		motor_cnt++;
		}
	if(neg_value==1)
	temp_val[0]='-';
	else
	temp_val[0]='+';		
	return temp_val;
}

int ulti_count=0;
void send_motor_settings(){
	set_rgb(0,101,10);
	char start_char[]="!";
	ir_send(ALL_DIRS,start_char,1);
	delay_ms(200);
	for(uint8_t direction=0;direction<8;direction++)
	{
		for(uint8_t motor=0;motor<3;motor++)
		{	
			char *as;
			as=custom_itoa(motor_adjusts[direction][motor]);
			for(int cntt=0;as[cntt]!=NULL;cntt++)
			{
				while(!ir_is_available(ALL_DIRS));
				ir_send(ALL_DIRS,(as+cntt),1);
				ulti_count++;
				while(!ir_is_available(ALL_DIRS));
				delay_ms(300);
			}
		}
	}
	printf("ULTI COUNT %d\n\r",ulti_count);
	ulti_count=0;
	set_rgb(0,0,0);
}
void check_if_motor_calibrated()
{
	int cnt=0;
	for(uint8_t direction=0;direction<8;direction++)
	{
		for(uint8_t motor=0;motor<3;motor++)
		{
			if(motor_adjusts[direction][motor]==(-1))
			cnt++;
		}
	}
	if(cnt>13)
	printf("Not calibrated\n\r");
	else
	printf("Calibrated\n\r");
}

/*
 *	The function below is optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection. It should return '1' if command_word was a valid command,
 *  '0' otherwise.
 */
uint8_t user_handle_command(char* command_word, char* command_args){
	if(strcmp_P(command_word,PSTR("read_motor_settings"))==0){
		schedule_task(5, send_motor_settings, NULL);
		return 1;
	}
	return 0;
}


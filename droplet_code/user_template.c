#include "user_template.h"

//uint8_t dummyVar[512] __attribute__((section(".USERCODE")));
uint32_t last_rnb_broadcast;
//char motor_values_of_another_droplet[120];
//int8_t receiving_motor_settings_from_another_droplet_flag=0;
int8_t sign_bit;
//int16_t motor_settings_received_count=0;
int16_t motor_signs_count=0;

uint8_t max_value_dir;
int16_t max_ir_value;
/*
 * Any code in this function will be run once, when the robot starts.
 */
void init(){
	set_rgb(0,0,25);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
#define STEPS 5
uint8_t can_follow_edge=0;
void loop(){

	if(get_time()-last_rnb_broadcast > 200){
		//check_collisions();
		//printf("\n\r");
		//broadcast_rnb_data();
		int16_t vals[6];
		check_collision_values(vals);
		printf("\t% 5d % 5d % 5d % 5d % 5d % 5d\r\n", vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
		last_rnb_broadcast=get_time();
	}
	

	//follow_edge(0);
	//check_ir_coll_max(&max_value_dir,&max_ir_value);
		//printf("max dir:%d max value:%d\n\r",max_value_dir,max_ir_value);
	//follow_edge_withrnb(0);
	//move_to_closest();

}
void follow_edge_withrnb(int dir)
{
	//dir=0 ==> move in left direction using IR pair 1
	//dir=1 ==> move in right direction using IR pair 4
	//move_to_closest();
	if(dir==0)
	{
		//check max ir coll dir
		check_ir_coll_max(&max_value_dir,&max_ir_value);
		if(max_value_dir==1)
		{
			move_steps(0,STEPS);
			
		}
		else if((max_value_dir==0)||(max_value_dir==5)){// if 0 or 5 , move anticlockwise
			move_steps(7,STEPS);
			set_rgb(0,0,25);
		}
		else {
			move_steps(6,STEPS);
			set_rgb(25,0,0);
		}
		//while(!is_moving());
		delay_ms(400);
	}
	else if(dir==1)
	{
		check_ir_coll_max(&max_value_dir,&max_ir_value);
		if(max_value_dir==4)
		{
			move_steps(0,STEPS);
		}
		else if((max_value_dir==0)||(max_value_dir==5)){// if 0 or 5 , move clockwise
			move_steps(6,STEPS);
		}
		else {
			move_steps(7,STEPS);
		}
		//while(!is_moving());
		delay_ms(400);
		
	}
	printf("max dir:%d max value:%d\n\r",max_value_dir,max_ir_value);
	//check if dir 1 has max IR -> if yes, move fwd 10 steps
	//									-> if no, move_anticlockwise 10 steps and check again
}
void move_to_closest(){
	if(rnb_updated){
		printf("RANGE %u Bearing %d Heading %d ID: %04x\n\r",last_good_rnb.range,last_good_rnb.bearing,last_good_rnb.heading,last_good_rnb.id);
		if(last_good_rnb.range<=70){
				set_rgb(0,100,0);
			follow_edge_withrnb(0);
			
			
			can_follow_edge=1;
		}
		else{						// if range is too large, make bearing zero and move forward 
			set_rgb(100,0,0);
			if(((last_good_rnb.bearing>=0)&&(last_good_rnb.bearing<=20))||((last_good_rnb.bearing<0)&&(last_good_rnb.bearing>=(-20))))
			move_steps(0,STEPS);
			else if(last_good_rnb.bearing>20)// bearing positive means on left side of arrow, so move anticlockwise
			move_steps(7,STEPS);
			else if(last_good_rnb.bearing<-20)
			move_steps(6,STEPS);
		}
		rnb_updated = 0; //Note! This line must be included for things to work properly.
	}
}


void handle_motor_settings_msg_two(int16_t settings[4][3], uint8_t whichSet){
	for(uint8_t i=0;i<4;i++){
		printf("\t%hu: % 5d | % 5d | % 5d\r\n", i+(whichSet*4), settings[i][0], settings[i][1], settings[i][2]);
	}	
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){

	if(msg_struct->length == sizeof(MotorSettingsMsg)){
		MotorSettingsMsg* msg = (MotorSettingsMsg*)(msg_struct->msg);
		if(msg->flag == 'S'){
			printf("Motor Settings for %04X\r\n", msg_struct->sender_ID);
			printf("\t     0   |   1   |   2  \r\n");
			handle_motor_settings_msg_two(msg->settings, 0);
		}else if(msg->flag == 'T'){
			handle_motor_settings_msg_two(msg->settings, 1);
		}
	}
	
	//printf("\r\n");
}


void send_motor_settings_two_A(){
	MotorSettingsMsg msg;
	msg.flag = 'S';
	for(uint8_t i=0;i<4;i++){
		for(uint8_t j=0;j<3;j++){
			msg.settings[i][j] = motor_adjusts[i][j];
		}
	}
	ir_send(ALL_DIRS, (char*)&msg, sizeof(MotorSettingsMsg));
	schedule_task(200, send_motor_settings_two_B, NULL);
}

void send_motor_settings_two_B(){
	MotorSettingsMsg msg;
	msg.flag = 'T';
	for(uint8_t i=0;i<4;i++){
		for(uint8_t j=0;j<3;j++){
			msg.settings[i][j] = motor_adjusts[4+i][j];
		}
	}
	ir_send(ALL_DIRS, (char*)&msg, sizeof(MotorSettingsMsg));
}


void check_ir_coll_max(uint8_t* input_dir, int16_t* input_val){

	
		int16_t measured_vals[6];
		uint8_t dirs=0;
		if(!ir_is_available(ALL_DIRS)){
			printf_P(PSTR("IR Hardware busy, probably sending a message? Can't check collisions.\r\n"));
			return;
			}
		for(uint8_t i=0;i<6;i++) ir_rxtx[i].status = IR_STATUS_BUSY_bm;
		uint16_t curr_power = get_all_ir_powers();
		set_all_ir_powers(256);
		
		for(uint8_t i=0;i<6;i++) ir_led_on(i);
		busy_delay_us(250);
		get_ir_sensors(measured_vals, 5);
		int16_t max_ir_value=-1000;
		uint8_t max_value_dir;
		for(uint8_t i=0;i<6;i++){
			//printf("%4d ", measured_vals[i]);
			if(measured_vals[i]>max_ir_value){
				max_ir_value=measured_vals[i];
				max_value_dir=i;
			//	printf("MAX %4d  DIR %d\n\r", max_ir_value,max_value_dir);
			}
		}
		//printf("\r\n");
		delay_ms(500);
		for(uint8_t i=0;i<6;i++) ir_led_off(i);
		set_all_ir_powers(curr_power);
		for(uint8_t i=0;i<6;i++) ir_rxtx[i].status = 0;
		*input_dir= max_value_dir;
		*input_val= max_ir_value;
}

void follow_edge(int dir)
{
	//dir=0 ==> move in left direction using IR pair 1
	//dir=1 ==> move in right direction using IR pair 4
	
	if(dir==0)
	{
		
		//check max ir coll dir
		check_ir_coll_max(&max_value_dir,&max_ir_value);
		if(max_value_dir==1)
		{
			move_steps(0,STEPS);
			set_rgb(0,25,0);
		}
		else if((max_value_dir==0)||(max_value_dir==5)){// if 0 or 5 , move anticlockwise
			move_steps(7,STEPS);
			set_rgb(0,0,25);
		}
		else {
			move_steps(6,STEPS);
			set_rgb(25,0,0);
		}
		while(is_moving()>=0);
		//delay_ms(400);
	}
	else if(dir==1)
	{
		check_ir_coll_max(&max_value_dir,&max_ir_value);
		if(max_value_dir==4)
		{
			move_steps(0,STEPS);
		}
		else if((max_value_dir==0)||(max_value_dir==5)){// if 0 or 5 , move clockwise
			move_steps(6,STEPS);
		}
		else {
			move_steps(7,STEPS);
		}
		while(is_moving()>=0);
		delay_ms(400);
		
	}
	printf("max dir:%d max value:%d\n\r",max_value_dir,max_ir_value);
		//check if dir 1 has max IR -> if yes, move fwd 10 steps
//									-> if no, move_anticlockwise 10 steps and check again
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
		schedule_task(5, send_motor_settings_two_A, NULL);
		return 1;
	}
	else if(strcmp_P(command_word,PSTR("white"))==0){
		schedule_task(5, move_to_closest, NULL);
		return 1;
	}
	return 0;
}

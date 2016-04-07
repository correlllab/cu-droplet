#include "serial_handler.h"

//#define IS_SPECIAL 1

uint8_t user_handle_command(char* command_word, char* command_args) __attribute__((weak));

void handle_serial_command(char* command, uint16_t command_length)
{
	//printf("command_time: %lu | last_command_time: %lu\r\n", cmd_arrival_time, last_serial_command_time);	
	//printf("Got command \"%s\".\r\n",command);
	//printf("\tcommand_time: %lu | last_command_time: %lu\r\n", cmd_arrival_time, last_serial_command_time);
	last_serial_command_time = cmd_arrival_time;	
	if(command[0]!='\0') //Not much to handle if we get an empty string.
	{
		char command_word[BUFFER_SIZE];
		char command_args[BUFFER_SIZE];
		get_command_word_and_args(command,command_length,command_word,command_args);
			 if(strcmp_P(command_word,PSTR("move_steps"))==0)			handle_move_steps(command_args);
		else if(strcmp_P(command_word,PSTR("walk"))==0)					handle_walk(command_args);
		else if(strcmp_P(command_word,PSTR("get_rgb"))==0)				handle_get_rgb();
		else if(strcmp_P(command_word,PSTR("set_ir"))==0)				handle_set_ir(command_args);
		else if(strcmp_P(command_word,PSTR("coll"))==0)					handle_check_collisions();
		else if(strcmp_P(command_word,PSTR("stop_walk"))==0)			handle_stop_walk();
		else if(strcmp_P(command_word,PSTR("set_motors"))==0)			handle_set_motors(command_args);
		else if(strcmp_P(command_word,PSTR("adj_motors"))==0)			handle_adjust_motors(command_args);
		else if(strcmp_P(command_word,PSTR("set_dist_per_step"))==0)	handle_set_mm_per_kilostep(command_args);
		else if(strcmp_P(command_word,PSTR("rnb_b"))==0)				handle_rnb_broadcast();
		else if(strcmp_P(command_word,PSTR("rnb_c"))==0)				handle_rnb_collect(command_args);
		else if(strcmp_P(command_word,PSTR("rnb_t"))==0)				handle_rnb_transmit(command_args);
		else if(strcmp_P(command_word,PSTR("rnb_r"))==0)				handle_rnb_receive();
		else if(strcmp_P(command_word,PSTR("set_led"))==0)				handle_set_led(command_args);
		else if(strcmp_P(command_word,PSTR("get_id"))==0)				handle_get_id();
		else if(strcmp_P(command_word,PSTR("broadcast_id"))==0)			handle_broadcast_id();
		else if(strcmp_P(command_word,PSTR("cmd"))==0)					handle_cmd(command_args);
		else if(strcmp_P(command_word,PSTR("tgt_cmd"))==0)				handle_targeted_cmd(command_args);
		else if(strcmp_P(command_word,PSTR("msg"))==0)					handle_shout(command_args);
		else if(strcmp_P(command_word,PSTR("msg_tst"))==0)				handle_msg_test(command_args);
		else if(strcmp_P(command_word,PSTR("tgt"))==0)					handle_target(command_args);
		else if(strcmp_P(command_word,PSTR("tasks"))==0)				print_task_queue();
		else if(strcmp_P(command_word,PSTR("reset"))==0)				handle_reset();
		else if(strcmp_P(command_word,PSTR("write_motor_settings"))==0)	write_motor_settings();
		else if(strcmp_P(command_word,PSTR("print_motor_settings"))==0){
																		print_motor_values();
																		print_dist_per_step();																	
		}else if(user_handle_command) //First, make sure the function is defined
		{
			if(!user_handle_command(command_word, command_args))	printf_P(CMD_NOT_RECOGNIZED_STR,command_word);
		}
		else														printf_P(CMD_NOT_RECOGNIZED_STR,command_word);
	}
}

void handle_check_collisions()
{
	uint8_t dirs = check_collisions();
	uint8_t found=0;
	for(uint8_t i=0;i<6;i++){
		if(dirs&(1<<i)){
			found=1;
			printf("%hhu",i);
		}
	}
	if(!found) printf("None");
	printf("\r\n");
}

void handle_move_steps(char* command_args)
{
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	uint8_t direction = token[0]-'0';

	token = strtok(NULL,delim);
	uint16_t num_steps = (uint16_t)atoi(token);
	if (num_steps > 0)
	{	
		printf_P(PSTR("walk direction %u, num_steps %u\r\n"), direction, num_steps);	
		move_steps(direction, num_steps);
	}	
}	

void handle_walk(char* command_args)
{	
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	uint8_t direction = token[0]-'0';
	
	token = strtok(NULL,delim);
	uint16_t distance_mm = (uint16_t)atoi(token);
	
	walk(direction, distance_mm);
}

void handle_get_rgb()
{
	int16_t r, g, b;
	get_rgb(&r, &g, &b);
	printf_P(PSTR("r: %hu, g: %hu, b: %hu\r\n"), r, g, b);
}

void handle_set_ir(char* command_args)
{
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	uint16_t ir_val = (uint16_t)atoi(token);
	
	schedule_task(10, set_all_ir_powers, (void*)ir_val);
}

void handle_stop_walk()
{
	stop_move(0);
}
//
//extern int16_t tau;
//void handle_set_tau(char* command_args)
//{
	//const char delim[2] = " ";
	//
	//char* token = strtok(command_args,delim);
	//uint16_t val = (uint16_t)atoi(token);
	//
	//tau = val;
//}
//
//extern double theta;
//void handle_set_theta(char* command_args)
//{
	//const char delim[2] = " ";
	//
	//char* token = strtok(command_args,delim);
	//uint16_t val = (uint16_t)atoi(token);
	//
	//theta = val;
//}

void handle_set_motors(char* command_args)
{	
	uint8_t r = get_red_led(), g = get_green_led(), b = get_blue_led();
	set_rgb(0,0,255);
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on direction.\r\n")); return;}
	uint8_t direction = atoi(token);
	if(direction> 7){ printf_P(PSTR("Bad direction. Got: %hhu.\r\n"), direction); return;}

	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on first val.\r\n")); return;}	
	motor_adjusts[direction][0] = atoi(token);
	
	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on second val.\r\n")); return;}
	motor_adjusts[direction][1] = atoi(token);
	
	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on third val.\r\n")); return;}
	motor_adjusts[direction][2] = atoi(token);	

	printf_P(PSTR("Got set_motors command. direction: %hhu, vals: (%d, %d, %d)\r\n"), direction, motor_adjusts[direction][0], motor_adjusts[direction][1], motor_adjusts[direction][2]);
	set_rgb(r,g,b);
}

void handle_adjust_motors(char* command_args)
{
	uint8_t r = get_red_led(), g = get_green_led(), b = get_blue_led();
	set_rgb(0,0,255);
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on direction.\r\n")); return;}
	uint8_t direction = atoi(token);
	if(direction> 7){ printf_P(PSTR("Bad direction. Got: %hhu.\r\n"), direction); return;}

	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on first val.\r\n")); return;}
	if(motor_adjusts[direction][0]>=0)
		motor_adjusts[direction][0]+= atoi(token);
	else
		motor_adjusts[direction][0]-= atoi(token);
	
	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on second val.\r\n")); return;}
	if(motor_adjusts[direction][1]>=0)
	motor_adjusts[direction][1]+= atoi(token);
	else
	motor_adjusts[direction][1]-= atoi(token);
	
	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on third val.\r\n")); return;}
	if(motor_adjusts[direction][2]>=0)
	motor_adjusts[direction][2]+= atoi(token);
	else
	motor_adjusts[direction][2]-= atoi(token);

	printf_P(PSTR("Got adjust_motors command. direction: %hhu, New Settings: (%d, %d, %d)\r\n"), direction, motor_adjusts[direction][0], motor_adjusts[direction][1], motor_adjusts[direction][2]);
	set_rgb(r,g,b);
}

void handle_set_mm_per_kilostep(char* command_args)
{
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	uint8_t direction = token[0]-'0';
	
	token = strtok(NULL,delim);
	uint16_t mm_per_kilostep = atoi(token);

	set_mm_per_kilostep(direction, mm_per_kilostep);
	
}

/* This tells the droplet that it should tell other droplets nearby their rnb to it.
 * In other words, this tells nearby droplets to listen, and then performs an ir_range_blast.
 */
void handle_rnb_broadcast()
{
	schedule_task(5,broadcast_rnb_data,NULL);
}

/* This tells the droplet that it should ask nearby droplets to do an 
 * ir_range_blast so it can learn their rnb.
 */
void handle_rnb_collect(char* command_args)
{
	const char delim[2] = " ";
		
	uint16_t id_val = atoi(strtok(command_args, delim));
	uint8_t power_val = atoi(strtok(NULL, delim));
	
	uint32_t wrapper_arg = (((uint32_t)id_val)|(((uint32_t)power_val)<<16));
	schedule_task(5,collect_rnb_data_wrapper, (void*)&wrapper_arg);
}

// This function is used so we can make a transparent call to collect_rnb_data with 
// multiple arguments, but still leave the interrupt handler.
void collect_rnb_data_wrapper(void* arg)
{
	uint32_t wrapper_arg = *((uint32_t*)arg);
	uint16_t id_val = (wrapper_arg&0xFFFF);
	uint8_t power_val = ((wrapper_arg>>16)&0xFF);
	collect_rnb_data(id_val, power_val);
}

/* This should only be called when another droplet asks this droplet 
 * to do an ir_range_blast (ie., by using handle_rnb_collect).
 */
void handle_rnb_transmit(char* command_args)
{
	uint32_t time_since_arrival = (get_time()-cmd_arrival_time+6);
	if(time_since_arrival<POST_MESSAGE_DELAY) delay_ms(POST_MESSAGE_DELAY - time_since_arrival);
	uint16_t power = (uint16_t)command_args[0] + 2;
	ir_range_blast(power);
}

/* This should only be called when another droplet is about to 
 * broadcast its rnb_data (ie., by using handle_rnb_broadcast()).
 */
void handle_rnb_receive()
{
	uint32_t time_since_arrival = (get_time()-cmd_arrival_time+5);
	if(time_since_arrival<POST_MESSAGE_DELAY) delay_ms(POST_MESSAGE_DELAY - time_since_arrival);	
	receive_rnb_data();
}

void handle_set_led(char* command_args)
{
	const char delim[2] = " ";
	char* token;

	uint8_t successful_read = 0;
	
	char* colors = strtok(command_args,delim);
	
	int length = strlen(colors);

	if(strcmp(colors,"hsv")==0)
	{
		uint16_t hVal;
		uint8_t sVal, vVal;
		token = strtok(NULL,delim);
		hVal = atoi(token);
		token = strtok(NULL,delim);
		sVal = atoi(token);
		token = strtok(NULL,delim);	
		vVal = atoi(token);
		set_hsv(hVal,sVal,vVal);
		successful_read=1;
	}
	else
	{
		for(int i=0 ; i < length ; i++)
		{
			token = strtok(NULL,delim);
			if(colors[i]=='r')
			{
				set_red_led(atoi(token));
				successful_read=1;
			}
			else if(colors[i]=='g')
			{
				set_green_led(atoi(token));
				successful_read=1;
			}
			else if(colors[i]=='b')
			{
				set_blue_led(atoi(token));
				successful_read=1;
			}
			else
			{
				successful_read=0;
				break;
			}
		}
	}

	if(successful_read!=1)
	{
		printf_P(PSTR("\tGot command set_led, but arguments (%s) were invalid. Format should be:\r\n"),command_args);
		printf_P(PSTR("\t Letters r,g,b, in any order, followed by values 0-255, in same \r\n"));
		printf_P(PSTR("\t order, indicating the brightness of the associated LEDs. Example: \r\n"));
		printf_P(PSTR("\t \"set_led bgr 5 30 0\" gives a bluish green.\r\n"));
	}
}
void handle_broadcast_id()
{
	schedule_task(5, send_id, NULL);
}

void handle_get_id()
{
	printf_P(PSTR("My ID is: %04X\r\n"),get_droplet_id());
}

void send_id()
{
	char msg[5];
	sprintf(msg, "%04X", get_droplet_id());
	ir_send(ALL_DIRS, msg, 4);
}

void handle_cmd(char* command_args)
{
	printf_P(PSTR("Broadcasting command: \"%s\", of length %i.\r\n"),(uint8_t*)command_args, strlen(command_args));
	#ifdef IS_SPECIAL
	ir_cmd(DIR0|DIR1|DIR3|DIR4, command_args,strlen(command_args));
	#else
	ir_cmd(ALL_DIRS, command_args,strlen(command_args));
	#endif
	//if(0==ir_send_command(0,(uint8_t*)command_args,strlen(command_args)))
	//printf("\tSent command \"%s\", of length %i\r\n",command_args,strlen(command_args));
	//else
	//printf("\tFailed to send \"%s\", of length %i\r\n",command_args,strlen(command_args));
	//
	//else
	//{
		//printf("\tIt wasn't OK to send command\r\n");
	//}
}

void handle_targeted_cmd(char* command_args)
{
	uint8_t loc = strcspn(command_args, " ");
	char targetString[5];
	char cmdString[32];
	
	strncpy(targetString, command_args, loc);
	strcpy(cmdString, command_args+loc+1);
	
	uint16_t target = strtoul(targetString, NULL, 16);
	printf_P(PSTR("command string: %s, length: %d\r\n"),cmdString, strlen(cmdString));
	ir_targeted_cmd(ALL_DIRS, cmdString,strlen(cmdString), target);
}

void handle_shout(char* command_args)
{
	if(strlen(command_args)==0) command_args = "Unique New York.";
	else if(strlen(command_args)>IR_BUFFER_SIZE)
	{ 
		printf_P(PSTR("Message length was %d chars, which exceeds the maximum of %d"), strlen(command_args), IR_BUFFER_SIZE);
		return;
	}
	ir_send(ALL_DIRS, command_args,strlen(command_args));
}

void handle_msg_test(char* command_args)
{
	uint8_t dir_mask = atoi(command_args);
	char msg[16] = "Unique New York.";
	
	ir_send(dir_mask, msg,16);
}

void handle_target(char* command_args)
{
	uint8_t loc = strcspn(command_args, " ");
	char targetString[5];
	char msgString[32];
	
	strncpy(targetString, command_args, loc);
	strcpy(msgString, command_args+loc);
	
	
	uint16_t target = strtoul(targetString, NULL, 16);
	
	//printf("Target: %04X\r\n",target);
	#ifdef IS_SPECIAL
	ir_targeted_send(DIR0|DIR1|DIR3|DIR4, msgString,strlen(msgString), target);
	#else	
	ir_targeted_send(ALL_DIRS, msgString,strlen(msgString), target);
	#endif
} 


void get_command_word_and_args(char* command, uint16_t command_length, char* command_word, char* command_args)
{
	//printf("\tIn gcwaa.\r\n");
	uint16_t write_index = 0;
	uint8_t writing_word_boole = 1;
	for(uint16_t i=0 ; i<command_length ; i++)
	{
		if(command[i]=='\0')
		{
			break;
		}
		else if(command[i]==' ' && writing_word_boole == 1)
		{
			command_word[write_index]='\0';
			write_index = 0;
			writing_word_boole = 0;
		}
		else
		{
			if(writing_word_boole==1)
			{
				command_word[write_index] = command[i];
			}
			else
			{
				command_args[write_index] = command[i];
			}
			write_index++;
		}
	}
	if(writing_word_boole==1)
	{
		command_word[write_index] = '\0';
		command_args[0] = '\0';
	}
	else
	{
		command_args[write_index] = '\0';
	}
}

void handle_reset()
{
	droplet_reboot();
}
#include "serial_handler.h"

extern rnb last_good_rnb;

void handle_serial_command(char* command, uint16_t command_length)
{
	uint32_t command_time = get_32bit_time();
	if((command_time - last_serial_command_time) < 5)
	{
		//printf("Ignoring a command: %s.\r\n",command);
		return;
	}
	last_serial_command_time = command_time;
	//printf("Got command \"%s\".\r\n",command);
	//printf("command_time: %lu | last_command_time: %lu\r\n", command_time, last_serial_command_time);
	if(command[0]!='\0') //Not much to handle if we get an empty string.
	{
		char command_word[BUFFER_SIZE];
		char command_args[BUFFER_SIZE];
		get_command_word_and_args(command,command_length,command_word,command_args);
		if(strcmp(command_word,"data")==0)
		{
			handle_data(command_args);
		}
		else if(strcmp(command_word,"move_steps")==0)
		{
			handle_move_steps(command_args);
		}
		else if(strcmp(command_word,"walk")==0)
		{
			handle_walk(command_args);
		}
		else if(strcmp(command_word,"stop_walk")==0)
		{
			handle_stop_walk();
		}
		else if(strcmp(command_word,"run_motor")==0)
		{
			handle_run_motor(command_args);	
		}
		else if(strcmp(command_word, "run_motors")==0)
		{
			handle_run_motors(command_args);
		}
		else if(strcmp(command_word, "set_motors_on_off")==0)
		{
			handle_set_motors_on_off(command_args);	
		}
		else if(strcmp(command_word, "set_motors_flipped")==0)
		{
			handle_set_motors_flipped(command_args);	
		}
		else if(strcmp(command_word,"set_motor")==0)
		{
			handle_set_motor(command_args);
		}
		else if(strcmp(command_word,"set_dist_per_step")==0)
		{
			handle_set_mm_per_kilostep(command_args);
		}
		else if(strcmp(command_word,"rnb_b")==0)
		{
			handle_rnb_broadcast();
		}
		else if(strcmp(command_word,"rnb_c")==0)
		{
			handle_rnb_collect(command_args);
		}
		else if(strcmp(command_word, "rnb_t")==0)
		{
			handle_rnb_transmit(command_args);
		}
		else if(strcmp(command_word,"rnb_r")==0)
		{
			handle_rnb_receive();	
		}
		else if(strcmp(command_word,"set_led")==0)
		{
			handle_set_led(command_args);
		}
		else if(strcmp(command_word,"get_id")==0)
		{
			handle_get_id();
		}
		else if(strcmp(command_word,"broadcast_id")==0)
		{
			handle_broadcast_id();
		}
		else if(strcmp(command_word,"cmd")==0)
		{
			handle_cmd(command_args, 1);
		}
		else if(strcmp(command_word,"tgt_cmd")==0)
		{
			handle_targeted_cmd(command_args);
		}
		else if(strcmp(command_word,"tasks")==0)
		{
			print_task_queue();
		}
		else if(strcmp(command_word,"msg")==0)
		{
			handle_shout(command_args);
		}	
		else if(strcmp(command_word,"tgt")==0)
		{
			handle_target(command_args);
		}
		else if(strcmp(command_word,"reset")==0)
		{
			handle_reset();
		}
		else if(strcmp(command_word,"print_motor_settings")==0)
		{
			print_motor_adjusts();
			print_dist_per_step();
		}
		else if(strcmp(command_word,"write_motor_settings")==0)
		{
			write_motor_settings();
		}
		else
		{
			printf("\tCommand ( %s ) not recognized.\r\n",command_word);
		}
	}
}

void handle_data(char *command_args)
{
	memset(serial_data_out_buffer.data, 0, BUFFER_SIZE);
	serial_data_out_buffer.length = strlen(command_args);
	if(serial_data_out_buffer.length < BUFFER_SIZE)
	{
		memcpy(serial_data_out_buffer.data, command_args, serial_data_out_buffer.length);
	}
	else
	{
		memcpy(serial_data_out_buffer.data, command_args, BUFFER_SIZE);
	}
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
		printf("walk direction %u, num_steps %u\r\n", direction, num_steps);		
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

void handle_stop_walk()
{
	stop();
}

void handle_run_motor(char* command_args)
{
	uint8_t prescalar, wg_mode;
	uint16_t period, offset, duty_cycle, duration;
	
	char* token;
	const char delim[2] = " ";
	
	token = strtok(command_args, delim);
	if(token==NULL){ printf("strtok returned NULL on clk_sel.\r\n"); return;}
	prescalar = atoi(token); //1: 1, 2: 2, 3: 4, 4: 8, 5: 64, 6: 256, 7: 1024
	if((prescalar<1)||(prescalar>7)){ printf("Bad clk_sel value, got: %hhu\r\n",prescalar); return;}
		
	token = strtok(NULL, delim);
	if(token==NULL){ printf("strtok returned NULL on wg_mode.\r\n"); return;}	
	wg_mode = atoi(token); //0: normal, 1: frequency, 3: singleslope, 5-7: dualslope.
	if((wg_mode>7)){ printf("Bad wg_mode value, got: %hhu\r\n",wg_mode); return;}
	
	token = strtok(NULL, delim);
	if(token==NULL){ printf("strtok returned NULL on period.\r\n"); return;}	
	period = atoi(token);
	
	token = strtok(NULL, delim);
	if(token==NULL){ printf("strtok returned NULL on offset.\r\n"); return;}
	offset = atoi(token);
	
	token = strtok(NULL, delim);
	if(token==NULL){ printf("strtok returned NULL on duty_cycle.\r\n"); return;}
	duty_cycle = atoi(token);
	if(duty_cycle> period){ printf("Bad duty_cycle. Got: %hu. Must be less than period.\r\n"); return;}
		
	token = strtok(NULL, delim);
	if(token==NULL){printf("strtok returned NULL on duration.\r\n"); return;}
	duration = atoi(token);
	
	//Strictly going to do motor 3?
	TCC0.CTRLA |= prescalar;
	TCC0.PER = period;
	TCC0.CCA = TCC0.CCB = duty_cycle;
	TCC0.CNTL=offset;//If two motors have different values here, their waves will be offset from eachother.
	TCC0.CTRLB = TC0_CCBEN_bm | wg_mode;
	
	uint16_t dur=0;
	while((dur<(duration)) && !(motor_status & MOTOR_STATUS_CANCEL))
	{
		delay_ms(5);
		dur+=5;
	}
	TCC0.CTRLB = wg_mode;
}

void handle_run_motors(char* command_args)
{
	uint16_t period, offset0, offset1, duty_cycle0, duty_cycle1, duration;
	
	char* token;
	const char delim[2] = " ";
	
	token = strtok(command_args, delim);
	if(token==NULL){ printf("strtok returned NULL on period0.\r\n"); return;}	
	period = atoi(token);
	
	token = strtok(NULL, delim);
	if(token==NULL){ printf("strtok returned NULL on offset0.\r\n"); return;}
	offset0 = atoi(token);
	
	token = strtok(NULL, delim);
	if(token==NULL){ printf("strtok returned NULL on offset1.\r\n"); return;}
	offset1 = atoi(token);
	
	token = strtok(NULL, delim);
	if(token==NULL){ printf("strtok returned NULL on duty_cycle0.\r\n"); return;}
	duty_cycle0 = atoi(token);
	if(duty_cycle0> period){ printf("Bad duty_cycle0. Got: %hu. Must be less than period.\r\n"); return;}
		
	token = strtok(NULL, delim);
	if(token==NULL){ printf("strtok returned NULL on duty_cycle1.\r\n"); return;}
	duty_cycle1 = atoi(token);
	if(duty_cycle1> period){ printf("Bad duty_cycle1. Got: %hu. Must be less than period.\r\n"); return;}		
		
	token = strtok(NULL, delim);
	if(token==NULL){printf("strtok returned NULL on duration.\r\n"); return;}
	duration = atoi(token);
	
	//Strictly going to do motor 3?
	TCC0.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCC1.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCC0.PER = period;
	TCC1.PER = period;
	
	TCC0.CCA = TCC0.CCB = duty_cycle0;
	TCC1.CCA = TCC1.CCB = duty_cycle1;
	
	TCC0.CNT=offset0;
	TCC1.CNT=offset1;
	
	TCC0.CTRLB = TC0_CCBEN_bm | TC_WGMODE_SS_gc;
	TCC1.CTRLB = TC1_CCAEN_bm | TC_WGMODE_SS_gc;
	
	uint16_t dur=0;
	while((dur<(duration)) && !(motor_status & MOTOR_STATUS_CANCEL))
	{
		delay_ms(5);
		dur+=5;
	}
	TCC0.CTRLB = TC_WGMODE_SS_gc;
	TCC1.CTRLB = TC_WGMODE_SS_gc;
}

void handle_set_motors_on_off(char* command_args)
{
	const char delim[2] = " ";
	
	char* token = strtok(command_args, delim);
	motor_on_time = atoi(token);
	
	token = strtok(command_args, delim);
	motor_off_time = atoi(token);
	
	printf("motor_on_time: %d, motor_off_time: %d.\r\n",motor_on_time, motor_off_time);
}

void handle_set_motors_flipped(char* command_args)
{
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	motor_flipped = atoi(token) ? MOTOR_0_FLIPPED_bm : 0;
	
	token = strtok(NULL,delim);	
	motor_flipped |= atoi(token) ? MOTOR_1_FLIPPED_bm : 0;
	
	token = strtok(NULL,delim);	
	motor_flipped |= atoi(token) ? MOTOR_2_FLIPPED_bm : 0;
	
	//printf("motor_flipped changed to: %d.\r\n", motor_flipped);
}


void handle_set_motor(char* command_args)
{
	
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	if(token==NULL){ printf("strtok returned NULL on motor_num.\r\n"); return;}
	uint8_t motor_num = atoi(token);
	if(motor_num> 2){ printf("Bad motor_num. Got: %hhu.\r\n", motor_num); return;}

	token = strtok(NULL,delim);
	if(token==NULL){ printf("strtok returned NULL on ccw adjust.\r\n"); return;}	
	motor_adjusts[motor_num][0] = atoi(token);
	if(motor_adjusts[motor_num][0]<(motor_on_time*-32)){ printf("Can't adjust by %d when motor is only on for %d.",motor_adjusts[motor_num][0], motor_on_time*32); }
	
	token = strtok(NULL,delim);
	if(token==NULL){ printf("strtok returned NULL on cw adjust.\r\n"); return;}	
	motor_adjusts[motor_num][1] = atoi(token);
	if(motor_adjusts[motor_num][1]<(motor_on_time*-32)){ printf("Can't adjust by %d when motor is only on for %d.",motor_adjusts[motor_num][1], motor_on_time*32); }
	
	
	//There should always be at least one motor with strength 1.0. It(they) must be the strongest motor(s).

	printf("Got set_motors command. motor_num: %hhu, ccw_adjust: %d, cw_adjust: %d.\r\n", motor_num, motor_adjusts[motor_num][0], motor_adjusts[motor_num][1]);
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
 * In other words, this tells nearby droplets to listen, and then performs an IR_range_blast.
 */
void handle_rnb_broadcast()
{
	schedule_task(5,broadcast_rnb_data,NULL);
}

/* This tells the droplet that it should ask nearby droplets to do an 
 * IR_range_blast so it can learn their rnb.
 */
void handle_rnb_collect(char* command_args)
{
	uint8_t power = atoi(command_args);	
	schedule_task(5,collect_rnb_data, power);
}

/* This should only be called when another droplet asks this droplet 
 * to do an IR_range_blast (ie., by using handle_rnb_collect).
 */
void handle_rnb_transmit(char* command_args)
{
	uint16_t power = (uint16_t)command_args[0] + 2;
	IR_range_blast(power);
	got_rnb_cmd_flag = 1;
}

/* This should only be called when another droplet is about to 
 * broadcast its rnb_data (ie., by using handle_rnb_broadcast()).
 */
void handle_rnb_receive()
{
	receive_rnb_data();
	rnb_updated = 0;
	last_good_rnb.id_number = (uint16_t)last_command_source_id;
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
		printf("\tGot command set_led, but arguments (%s) were invalid. Format should be:\r\n",command_args);
		printf("\t Letters r,g,b, in any order, followed by values 0-255, in same \r\n");
		printf("\t order, indicating the brightness of the associated LEDs. Example: \r\n");
		printf("\t \"set_led bgr 5 30 0\" gives a bluish green.\r\n");
	}
}
void handle_broadcast_id()
{
	schedule_task(5,send_id, NULL);
}

void handle_get_id()
{
	printf("My ID is: %X\r\n",droplet_ID);
}

void send_id()
{
	if(OK_to_send())
	{
		set_rgb(50,50,0);
		uint8_t data[2];
		data[0] = 0xFF & (droplet_ID>>8);
		data[1] = 0xFF & droplet_ID;
		ir_broadcast(data, 2);
		delay_ms(100);
		set_rgb(0,0,0);
	}
}

void handle_cmd(char* command_args, uint8_t should_broadcast)
{
	if(OK_to_send())
	{
		
		if(should_broadcast)
		{
			//printf("Broadcasting command: \"%s\", of length %i.\r\n",(uint8_t*)command_args, strlen(command_args));
			ir_broadcast_cmd((uint8_t*)command_args,strlen(command_args));
		}
		else
		{
			//printf("Transmitting command: \"%s\", of length %i.\r\n",(uint8_t*)command_args, strlen(command_args));
			ir_send_cmd(1,(uint8_t*)command_args,strlen(command_args));
		}

		//if(0==ir_send_command(0,(uint8_t*)command_args,strlen(command_args)))
		//printf("\tSent command \"%s\", of length %i\r\n",command_args,strlen(command_args));
		//else
		//printf("\tFailed to send \"%s\", of length %i\r\n",command_args,strlen(command_args));
	}
	
	else
	{
		printf("\tIt wasn't OK to send command\r\n");
	}
}

void handle_targeted_cmd(char* command_args)
{
	uint8_t loc = strcspn(command_args, " ");
	char targetString[5];
	char cmdString[32];
	
	strncpy(targetString, command_args, loc);
	strcpy(cmdString, command_args+loc+1);
	
	uint16_t target = strtoul(targetString, NULL, 16);
	printf("command string: %s, length: %d\r\n",cmdString, strlen(cmdString));
	if(OK_to_send())
	{
		ir_targeted_broadcasted_cmd(cmdString,strlen(cmdString), target);
	}
}

void handle_shout(char* command_args)
{
	if(strlen(command_args)==0)
	{
		command_args = "Hello over there.";
	}
	
	if(OK_to_send())
	{
		ir_broadcast(command_args,strlen(command_args));
	}
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
	
	if(OK_to_send())
	{
		ir_targeted_broadcast(msgString,strlen(msgString), target);
	}
} 


void get_command_word_and_args(char* command, uint16_t command_length, char* command_word, char* command_args)
{
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
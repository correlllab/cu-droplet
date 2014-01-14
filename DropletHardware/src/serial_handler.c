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
		else if(strcmp(command_word,"walk")==0)
		{
			handle_walk(command_args);
		}
		else if(strcmp(command_word,"set_motor_dc")==0)
		{
			handle_set_motor_dc(command_args);
		}
		else if(strcmp(command_word,"set_motor_ot")==0)
		{
			handle_set_motor_ot(command_args);
		}
		else if(strcmp(command_word,"stop_walk")==0)
		{
			handle_stop_walk();
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
			print_motor_duty_cycles();
			print_motor_on_times();
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

void handle_walk(char* command_args)
{
	const char delim[2] = " ";
	uint8_t direction, num_steps;
	uint8_t successful_read = 1;
	
	char* token = strtok(command_args,delim);
	
	switch (token[0])
	{
		case '0': direction = 0; break;
		case '1': direction = 1; break;
		case '2': direction = 2; break;
		case '3': direction = 3; break;
		case '4': direction = 4; break;
		case '5': direction = 5; break;
		case '6': direction = 6; break;
		case '7': direction = 7; break;
		case 'N':
			if (token[1] == 'E') { direction = 1;}
			else if (token[1] == 'W') { direction = 5; }
			else { direction = 0;}
			break;
		case 'S':
			if (token[1] == 'E') { direction = 2; }
			else if (token[1] == 'W') { direction = 4; }
			else { direction = 3; }
			break;
		case 'C':
			if (token[2] == 'W') { direction = 7; }
			else { direction = 6; }
			break;
				
		default:
			successful_read = 0;		
	}

	token = strtok(NULL,delim);
	num_steps = (uint8_t)atoi(token);
	if (num_steps < 1) successful_read = 0;

	if (successful_read)
	{
		move_steps(direction, num_steps);
		printf("walk direction %u, num_steps %u\r\n", direction, num_steps);
	}		
	else
	{
		printf("\tGot command walk, but arguments (%s) were invalid. Format should be:\r\n",command_args);
		printf("\t Direction (1-6), followed by number of steps (uint8_t).\r\n");
	}
}

void handle_stop_walk()
{
	cancel_move();
}

void handle_set_motor_dc(char* command_args)
{
	const char delim[2] = " ";

	uint8_t direction;
	int8_t m[3];

	uint8_t successful_read = 1;
	
	char* token = strtok(command_args,delim);
	
	switch (token[0])
	{
		case '0': direction = 0; break;
		case '1': direction = 1; break;
		case '2': direction = 2; break;
		case '3': direction = 3; break;
		case '4': direction = 4; break;
		case '5': direction = 5; break;
		case '6': direction = 6; break;
		case '7': direction = 7; break;
		case 'N':
			if (token[1] == 'E') { direction = 1;}
			else if (token[1] == 'W') { direction = 5; }
			else { direction = 0;}
			break;
		case 'S':
			if (token[1] == 'E') { direction = 2; }
			else if (token[1] == 'W') { direction = 4; }
			else { direction = 3; }
			break;
		case 'C':
			if (token[1] == 'W') { direction = 6; }
			else { direction = 7; }
			break;
				
		default:
			successful_read = 0;		
	}
	
	token = strtok(NULL,delim);
	m[0] = atoi(token);
	token = strtok(NULL,delim);
	m[1] = atoi(token);
	token = strtok(NULL,delim);
	m[2] = atoi(token);

	printf("Got set_motor_dc command: direction is %u, settings are %i %i %i\r\n", direction, m[0], m[1], m[2]);
	set_motor_duty_cycle(1, direction, m[0]);
	set_motor_duty_cycle(2, direction, m[1]);
	set_motor_duty_cycle(3, direction, m[2]);

	if(successful_read!=1)
	{
		printf("\tGot command set_motor, but arguments (%s) were invalid. Format should be:\r\n",command_args);
		printf("\tset_motor direction m1 m2 m3\r\n");
		printf("\twhere direction is 1-8 or one of N NE SE S SW NW or CW or CCW\r\n");
		printf("\tand m1, m2, and m3 are ints between -100 and 100 specifying duty cycle percentage\r\n");
	}

}

void handle_set_motor_ot(char* command_args)
{
	const char delim[2] = " ";

	uint8_t direction;
	int8_t m[3];

	uint8_t successful_read = 1;
	
	char* token = strtok(command_args,delim);
	
	switch (token[0])
	{
		case '0': direction = 0; break;
		case '1': direction = 1; break;
		case '2': direction = 2; break;
		case '3': direction = 3; break;
		case '4': direction = 4; break;
		case '5': direction = 5; break;
		case '6': direction = 6; break;
		case '7': direction = 7; break;
		case 'N':
	if (token[1] == 'E') { direction = 1;}
else if (token[1] == 'W') { direction = 5; }
			else { direction = 0;}
			break;
		case 'S':
			if (token[1] == 'E') { direction = 2; }
			else if (token[1] == 'W') { direction = 4; }
			else { direction = 3; }
			break;
		case 'C':
			if (token[1] == 'W') { direction = 6; }
			else { direction = 7; }
			break;
				
		default:
			successful_read = 0;		
	}
	
	token = strtok(NULL,delim);
	m[0] = atoi(token);
	token = strtok(NULL,delim);
	m[1] = atoi(token);
	token = strtok(NULL,delim);
	m[2] = atoi(token);

	printf("Got set_motor_ot command: direction is %u, settings are %i %i %i\r\n", direction, m[0], m[1], m[2]);
	set_motor_on_time(1, direction, m[0]);
	set_motor_on_time(2, direction, m[1]);
	set_motor_on_time(3, direction, m[2]);

	if(successful_read!=1)
	{
		printf("\tGot command set_motor, but arguments (%s) were invalid. Format should be:\r\n",command_args);
		printf("\tset_motor direction m1 m2 m3\r\n");
		printf("\twhere direction is 1-8 or one of N NE SE S SW NW or CW or CCW\r\n");
		printf("\tand m1, m2, and m3 are ints between -100 and 100 specifying duty cycle percentage\r\n");
	}

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
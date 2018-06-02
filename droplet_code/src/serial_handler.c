#include "serial_handler.h"
#include "droplet_init.h"

static const char CMD_NOT_RECOGNIZED_STR[] PROGMEM = "\tCommand ( %s ) not recognized.\r\n";

static void handle_prog(char* command_args);
static void handle_ir_prog(char* command_args);
static void handle_check_collisions(void);
static void handle_move_steps(char* command_args);
static void handle_walk(char* command_args);
static void handle_get_rgb(void);
static void handle_set_ir(char* command_args);
static void handle_stop_walk(void);
static void handle_set_motors(char* command_args);
static void handle_adjust_motors(char* command_args);
static void handle_set_mm_per_kilostep(char* command_args);
static void handle_rnb_broadcast(void);
static void handle_set_led(char* command_args);
static void handle_broadcast_id(void);
static void handle_get_id(void);
static void handle_cmd(char* command_args);
static void handle_targeted_cmd(char* command_args);
static void handle_shout(char* command_args);
static void handle_msg_test(char* command_args);
static void handle_target(char* command_args);
static void handle_reset(void);
static void get_command_word_and_args(char* command, uint16_t command_length, char* command_word, char* command_args);

uint8_t userHandleCommand(char* command_word, char* command_args) __attribute__((weak));

/*
 * This is where incoming commands are interpreted. The actual command is the string inside the PSTR function.
 * For example, move_steps, walk, or get_rgb. If the command matches the string, the function on the same line
 * will get called. See individual functions for further documentation on that command's syntax.
 */
void handleSerialCommand(char* command, uint16_t command_length){
	if(command[0]!='\0'){ //Not much to handle if we get an empty string.
		char command_word[SRL_BUFFER_SIZE];
		char command_args[SRL_BUFFER_SIZE];
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
		else if(strcmp_P(command_word,PSTR("set_led"))==0)				handle_set_led(command_args);
		else if(strcmp_P(command_word,PSTR("get_id"))==0)				handle_get_id();
		else if(strcmp_P(command_word,PSTR("broadcast_id"))==0)			handle_broadcast_id();
		else if(strcmp_P(command_word,PSTR("cmd"))==0)					handle_cmd(command_args);
		else if(strcmp_P(command_word,PSTR("tgt_cmd"))==0)				handle_targeted_cmd(command_args);
		else if(strcmp_P(command_word,PSTR("msg"))==0)					handle_shout(command_args);
		else if(strcmp_P(command_word,PSTR("msg_tst"))==0)				handle_msg_test(command_args);
		else if(strcmp_P(command_word,PSTR("tgt"))==0)					handle_target(command_args);
		else if(strcmp_P(command_word,PSTR("tasks"))==0)				printTaskQueue();
		else if(strcmp_P(command_word,PSTR("reset"))==0)				handle_reset();
		else if(strcmp_P(command_word,PSTR("write_motor_settings"))==0)	writeMotorSettings();
		else if(strcmp_P(command_word,PSTR("print_motor_settings"))==0){
																		printMotorValues();
																		printDistPerStep();																	
		}else if(strcmp_P(command_word,PSTR("prog"))==0)				handle_prog(command_args);
		else if(strcmp_P(command_word,PSTR("ir_prog"))==0)				handle_ir_prog(command_args);
		else if(userHandleCommand){ //First, make sure the function is defined
			if(!userHandleCommand(command_word, command_args))	printf_P(CMD_NOT_RECOGNIZED_STR,command_word);
		}
		else														printf_P(CMD_NOT_RECOGNIZED_STR,command_word);

	}
}

//This cmd is sent by computer to the Droplet which will be sending all the code over IR.
static void handle_prog(char* command_args){
	ProgData* progData = (ProgData*)command_args;
	if(progData->flag != 'D'){
		printf("Error: Unexpected ir programming data!\r\n");
		return;
	}
	printf("Got ir_prog init command:\r\n");
	printf("\t%p: %u\r\n", progData->secAaddr, progData->secAsize);
	printf("\t%p: %u\r\n", progData->secBaddr, progData->secBsize);
	printf("\t%p: %u\r\n", progData->secCaddr, progData->secCsize);

	ProgMsg msg;
	strcpy( msg.cmdWord, "ir_prog ");
	msg.data = *progData;

	irCmd(ALL_DIRS, (char*)(&msg), sizeof(ProgMsg));
	NONATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		waitForTransmission(ALL_DIRS);
		delayMS(500);
	}
	irCmd(ALL_DIRS, (char*)(&msg), sizeof(ProgMsg));
	NONATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		waitForTransmission(ALL_DIRS);
		delayMS(500);
	}
	irCmd(ALL_DIRS, (char*)(&msg), sizeof(ProgMsg));
	NONATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		waitForTransmission(ALL_DIRS);
		delayMS(500);
	}
}


// After getting the prog cmd from a computer, the Droplet plugged in to the computer 
// sends this command over IR to force receiving Droplets to enter programming mode.
static void handle_ir_prog(char* command_args){
	ProgData* data = (ProgData*)command_args;
	if(data->flag != 'D'){
		setRGB(200,0,0);
		return;
	}
	setRGB(0,50,0);
	reprogramming = 1;
	progData = *data;
}

/*
 * No arguments.
 * Prints each direction in which a collision was detected, or None.
 */
static void handle_check_collisions(void){
	uint8_t dirs = checkCollisions();
	uint8_t found=0;
	for(uint8_t i=0;i<6;i++){
		if(dirs&(1<<i)){
			found=1;
			printf("%hu",i);
		}
	}
	if(!found) printf("None");
	printf("\r\n");
}

/*
 * Format:
 *     move_steps <dir> <num_steps>
 * Calls the move_steps function with arguments as described above.
 */
static void handle_move_steps(char* command_args){
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	uint8_t direction = token[0]-'0';

	token = strtok(NULL,delim);
	uint16_t num_steps = (uint16_t)atoi(token);
	if (num_steps > 0){	
		printf_P(PSTR("walk direction %hu, num_steps %u\r\n"), direction, num_steps);	
		moveSteps(direction, num_steps);
	}	
}	

/*
 * Format:
 *     walk <dir> <distance>
 * Calls the walk function with arguments as described above.
 */
static void handle_walk(char* command_args){	
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	uint8_t direction = token[0]-'0';
	
	token = strtok(NULL,delim);
	uint16_t distance_mm = (uint16_t)atoi(token);
	
	walk(direction, distance_mm);
}

/*
 * No arguments.
 * Prints the sensed colors, as reported by get_rgb.
 */
static void handle_get_rgb(void){
	int16_t r, g, b;
	getRGB(&r, &g, &b);
	printf_P(PSTR("r: %hu, g: %hu, b: %hu\r\n"), r, g, b);
}

/*
 * Format:
 *     set_ir <power>
 * Calls the set_all_ir_powers function with arguments as described above.
 */
static void handle_set_ir(char* command_args){
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	uint16_t ir_val = (uint16_t)atoi(token);

	scheduleTask(10, (arg_func_t)setAllirPowers, (void*)ir_val);
}

/*
 * No arguments.
 * Calls the stop_move function; the droplet stops moving.
 */
static void handle_stop_walk(void){
	stopMove();
}

/*
 * Format:
 *     set_motors <dir> <mot0val> <mot1val> <mot2val>
 * Adjusts the (volatile-memory-copy) motor settings for <dir> to be the three motor values indicated.
 * The sign of the motor value indicates what direction the motor should spin, the magnitude effects
 * how long it spins for.
 * Note that Audio Droplets don't have a motor 0, so <mot0val> should always be 0 for them.
 */
static void handle_set_motors(char* command_args){	
	uint8_t r = getRedLED(), g = getGreenLED(), b = getBlueLED();
	setRGB(0,0,255);
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on direction.\r\n")); return;}
	uint8_t direction = atoi(token);
	if(direction> 7){ printf_P(PSTR("Bad direction. Got: %hu.\r\n"), direction); return;}

	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on first val.\r\n")); return;}	
	motorAdjusts[direction][0] = atoi(token);
	
	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on second val.\r\n")); return;}
	motorAdjusts[direction][1] = atoi(token);
	
	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on third val.\r\n")); return;}
	motorAdjusts[direction][2] = atoi(token);	

	printf_P(PSTR("Got set_motors command. direction: %hu, vals: (%d, %d, %d)\r\n"), direction, motorAdjusts[direction][0], motorAdjusts[direction][1], motorAdjusts[direction][2]);
	setRGB(r,g,b);
}

/*
 * Format:
 *     adj_motors <dir> <mot0val> <mot1val> <mot2val>
 * Same as set_motors, described above, except that the motor settings currently stored in volatile 
 * memory are changed by the indicated values.
 */
static void handle_adjust_motors(char* command_args){
	uint8_t r = getRedLED(), g = getGreenLED(), b = getBlueLED();
	setRGB(0,0,255);
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on direction.\r\n")); return;}
	uint8_t direction = atoi(token);
	if(direction> 7){ printf_P(PSTR("Bad direction. Got: %hu.\r\n"), direction); return;}

	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on first val.\r\n")); return;}
	if(motorAdjusts[direction][0]>=0)
		motorAdjusts[direction][0]+= atoi(token);
	else
		motorAdjusts[direction][0]-= atoi(token);
	
	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on second val.\r\n")); return;}
	if(motorAdjusts[direction][1]>=0)
	motorAdjusts[direction][1]+= atoi(token);
	else
	motorAdjusts[direction][1]-= atoi(token);
	
	token = strtok(NULL,delim);
	if(token==NULL){ printf_P(PSTR("strtok returned NULL on third val.\r\n")); return;}
	if(motorAdjusts[direction][2]>=0)
	motorAdjusts[direction][2]+= atoi(token);
	else
	motorAdjusts[direction][2]-= atoi(token);

	printf_P(PSTR("Got adjust_motors command. direction: %hu, New Settings: (%d, %d, %d)\r\n"), direction, motorAdjusts[direction][0], motorAdjusts[direction][1], motorAdjusts[direction][2]);
	setRGB(r,g,b);
}

/*
 * Format:
 *     set_dist_per_step <dir> <value>
 * Adjusts the (volatile-memory-copy) walking distance settings for <dir>; the value should
 * be the distance in mm that the robot travels per thousand steps taken. For spinning
 * directions (6 & 7), the distance is in degrees.
 */
static void handle_set_mm_per_kilostep(char* command_args){
	const char delim[2] = " ";
	
	char* token = strtok(command_args,delim);
	uint8_t direction = token[0]-'0';
	
	token = strtok(NULL,delim);
	uint16_t mm_per_kilostep = atoi(token);

	setMMperKilostep(direction, mm_per_kilostep);
}

/*
 * No arguments.
 * Calls broadcast_rnb_data.
 */
static void handle_rnb_broadcast(void){
	scheduleTask(5,broadcastRnbData,NULL);
}

/*
 * Format:
 *     set_led <colors> <values>
 *     <colors>: A string containing some combination of 'r', 'g', 'b', up to once each, in any order,
 *               with no spaces, or 'hsv'.
 *     <values>: If <colors> was 'hsv':
 *                   <hue> <saturation> <value>
 *                   <hue>: 0-360
 *               Otherwise:
 *                   Some combination of <r>, <g>, and <b> values, in the same order and quantity as 'colors'.
 * Examples:
 *     set_led rgb 255 255 255 //Full white.
 *     set_led rgb 255 0 0     //Full red, green, and blue off.
 *     set_led r 255           //Full red without effecting green or blue.
 *     set_led br 127 64       //Half-strength blue, quarter-strength red. (order is flexible)
 *     set_led hsv 0 255 255   //Set to fully saturated and bright hue 0 (red).
 */
static void handle_set_led(char* command_args){
	const char delim[2] = " ";
	char* token;	
	char* colors = strtok(command_args,delim);
	int length = strlen(colors);

	if(strcmp(colors,"hsv")==0){
		uint16_t hVal;
		uint8_t sVal, vVal;
		token = strtok(NULL,delim);
		hVal = atoi(token);
		token = strtok(NULL,delim);
		sVal = atoi(token);
		token = strtok(NULL,delim);	
		vVal = atoi(token);
		setHSV(hVal,sVal,vVal);
	}else{
		for(int i=0 ; i < length ; i++){
			token = strtok(NULL,delim);
			if(colors[i]=='r'){
				setRedLED(atoi(token));
			}else if(colors[i]=='g'){
				setGreenLED(atoi(token));
			}else if(colors[i]=='b'){
				setBlueLED(atoi(token));
			}else{
				break;
			}
		}
	}
}

/*
 * No arguments.
 * Broadcasts the four-character readable form of this Droplet's ID.
 */
static void handle_broadcast_id(void){
	scheduleTask(5, sendID, NULL);
}

/*
 * No arguments.
 * prints this Droplet's ID.
 */
static void handle_get_id(void){
	printf_P(PSTR("My ID is: %04X\r\n"),getDropletID());
}


/*
 * Format:
 *     cmd <CMD>
 * <CMD>: Anything.
 *
 * This Droplet broadcasts <CMD> as an IR command.
 * IR commands are communicated over infrared, but then interpreted by the same code which interprets 
 * commands over serial, in this file.
 *
 * Example:
 *     cmd reset
 *     All receiving Droplets will act as if they had 'reset' typed to them over the terminal, and reset.
 */
static void handle_cmd(char* command_args){
	printf_P(PSTR("Broadcasting command: \"%s\", of length %i.\r\n"), (uint8_t*)command_args, strlen(command_args));
	irCmd(ALL_DIRS, command_args,strlen(command_args));
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

/*
 * Format:
 *     tgt_cmd <ID> <CMD>
 * <ID>: A droplet ID.
 * <CMD>: Anything.
 *
 * This works the same as 'cmd', described above, except the command is targetted to a specific Droplet ID, <ID>. 
 * All other Droplets will ignore the message.
 *
 * Example:
 *     tgt_cmd af6a reset
 *     The Droplet with ID 0xAF6A will act as if it had 'reset' typed to it over the terminal, and reset. Other
 *     Droplets will do nothing.)
 */
static void handle_targeted_cmd(char* command_args){
	uint8_t loc = strcspn(command_args, " ");
	char targetString[5];
	char cmdString[32];
	
	strncpy(targetString, command_args, loc);
	strcpy(cmdString, command_args+loc+1);
	
	uint16_t target = strtoul(targetString, NULL, 16);
	printf_P(PSTR("Broadcasting command to %04X: \"%s\", of length %i.\r\n"), target, (uint8_t*)cmdString, strlen(cmdString));
	irTargetedCmd(ALL_DIRS, cmdString,strlen(cmdString), target);
}

/*
 * Format:
 *     msg <MSG>
 *     <MSG>: Any test. This argument is optional.
 *
 * <MSG> will be treated as a string and broadcast over IR.
 * If <MSG> is not present, a simple test string will be broadcast instead.
 */
static void handle_shout(char* command_args){
	if(strlen(command_args)==0){
		command_args = "Unique New York.";
	}else if(strlen(command_args)>IR_BUFFER_SIZE){ 
		printf_P(PSTR("Message length was %z chars, which exceeds the maximum of %u"), strlen(command_args), IR_BUFFER_SIZE);
		return;
	}
	irSend(ALL_DIRS, command_args,strlen(command_args));
}

/*
 * Format:
 *     msg_tst <dir_mask>
 *     <dir_mask>: A direction mask indicating which direction(s) to send the message in.
 * Sends a simple test message in the indicated directions.
 */
static void handle_msg_test(char* command_args){
	uint8_t dir_mask = atoi(command_args);
	char msg[16] = "Unique New York.";
	
	irSend(dir_mask, msg,16);
}

/*
 * Format:
 *      tgt <ID> <MSG>
 *	<ID>: A Droplet ID.
 * Behaves the same as the 'msg' command, described above, except the message is broadcast
 * to a specific Droplet with <ID>, and will be ignored by other Droplets. Also, this function
 * does not have a default message.
 */
static void handle_target(char* command_args){
	uint8_t loc = strcspn(command_args, " ");
	char targetString[5];
	char msgString[32];
	
	strncpy(targetString, command_args, loc);
	strcpy(msgString, command_args+loc);
	
	
	uint16_t target = strtoul(targetString, NULL, 16);
	
	//printf("Target: %04X\r\n",target);
	irTargetedSend(ALL_DIRS, msgString,strlen(msgString), target);
} 

/*
 * No arguments.
 * The Droplet resets.
 */
static void handle_reset(void){
	dropletReboot();
}

void sendID(){
	char msg[5];
	sprintf(msg, "%04X", getDropletID());
	irSend(ALL_DIRS, msg, 4);
}

static void get_command_word_and_args(char* command, uint16_t command_length, char* command_word, char* command_args){
	//printf("\tIn gcwaa.\r\n");
	uint16_t write_index = 0;
	uint8_t writing_word_boole = 1;
	for(uint16_t i=0 ; i<command_length ; i++){
		if(command[i]=='\0'){
			break;
		}else if(command[i]==' ' && writing_word_boole == 1){
			command_word[write_index]='\0';
			write_index = 0;
			writing_word_boole = 0;
		}else{
			if(writing_word_boole==1){
				command_word[write_index] = command[i];
			}else{
				command_args[write_index] = command[i];
			}
			write_index++;
		}
	}
	if(writing_word_boole==1){
		command_word[write_index] = '\0';
		command_args[0] = '\0';
	}else{
		command_args[write_index] = '\0';
	}
}

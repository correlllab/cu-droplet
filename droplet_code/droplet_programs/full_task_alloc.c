// //#include "droplet_programs/full_task_alloc.h"
// //
// ///*
 // //* any code in this function will be run once, when the robot starts.
 // //*/
// //void init()
// //{
	// //prev_move_dir=-1;
// //}
// //
// ///*
 // //* the code in this function will be called repeatedly, as fast as it can execute.
 // //*/
// //void loop()
// //{
	// //if(rnb_updated)
	// //{
		// //printf("%x: %4.3f %3.1f %3.1f\r\n", last_good_rnb.id_number, last_good_rnb.range, rad_to_deg(last_good_rnb.bearing), rad_to_deg(last_good_rnb.heading));
		// //rnb_updated=0;
	// //}
// //}
// //
// ///*
 // //* after each pass through loop(), the robot checks for all messages it has 
 // //* received, and calls this function once for each message.
 // //*/
// //void handle_msg(ir_msg* msg_struct)
// //{
	// //if(msg_struct->length!=20) return;
    // //uint8_t my_index = get_droplet_ord(get_droplet_id()) - 100;
    // //uint8_t *commands = (uint8_t *)(msg_struct->msg);
    // //uint8_t state = commands[my_index] - '`';
	// //uint8_t new_dir = (state>>2)&0x07;
	// //uint8_t adjust = (state&0x03);
    // //printf("got msg!\r\n\t");
    // ////for(uint8_t i=0;i<msg_struct->length;i++) printf("%c",msg_struct->msg[i]);
    // //printf("\r\nmy_index: %hu\r\n\tstate: %hu (%hu, %hu)\r\n",my_index,(uint8_t)state, new_dir, adjust);
    // //stop_move();
    // //led_off();
    // //
    // //int8_t dir = -1;
    // //uint16_t num_steps = 1000;
    // //switch (new_dir)
    // //{
        // //case walk_forward:
			// //dir = 0;
		    // //break;
        // //case walk_backward:
			// //dir = 3;
	        // //break;
        // //case turn_right:
			// //dir = 6;
			// //break;
        // //case turn_left:
			// //dir = 7;
			// //break; 
	// //}
	// //
	// //switch(adjust)
	// //{
		// //case one_over_two:
			// //if(prev_move_dir>=0)
			// //{
				// //if(motor_adjusts[prev_move_dir][1]>0)
					// //motor_adjusts[prev_move_dir][1] = motor_adjusts[prev_move_dir][1]-10;
				// //else if(motor_adjusts[prev_move_dir][1]<0)
					// //motor_adjusts[prev_move_dir][1] = motor_adjusts[prev_move_dir][1]+10;
				// //if(motor_adjusts[prev_move_dir][2]>0)
					// //motor_adjusts[prev_move_dir][2] = motor_adjusts[prev_move_dir][2]+10;
				// //else if(motor_adjusts[prev_move_dir][2]<0)
					// //motor_adjusts[prev_move_dir][2] = motor_adjusts[prev_move_dir][2]-10;			
			// //}
			// //break;
		// //case two_over_one:
			// //if(prev_move_dir>=0)
			// //{
				// //if(motor_adjusts[prev_move_dir][1]>0)
					// //motor_adjusts[prev_move_dir][1] = motor_adjusts[prev_move_dir][1]+10;
				// //else if(motor_adjusts[prev_move_dir][1]<0)
					// //motor_adjusts[prev_move_dir][1] = motor_adjusts[prev_move_dir][1]-10;
				// //if(motor_adjusts[prev_move_dir][2]>0)
					// //motor_adjusts[prev_move_dir][2] = motor_adjusts[prev_move_dir][2]-10;
				// //else if(motor_adjusts[prev_move_dir][2]<0)
					// //motor_adjusts[prev_move_dir][2] = motor_adjusts[prev_move_dir][2]+10;
			// //}
			// //break;
		// //case write_settings:
			// //write_motor_settings();
			// //break;
	// //}
// //
	// //prev_move_dir=dir;    
    // //if (dir >=0)
    // //{
        // //move_steps(dir, num_steps);
    // //}
// //}
// //
// ///*
 // //*	the function below is optional - commenting it in can be useful for debugging if you want to query
 // //*	user variables over a serial connection.
 // //*/
// //uint8_t user_handle_command(char* command_word, char* command_args)
// //{
	// //if(strcmp_p(command_word,pstr("set_d_p_30"))!=0) return 0;
	// //{
		// //const char delim[2] = " ";
		// //
		// //char* token = strtok(command_args,delim);
		// //uint8_t direction = token[0]-'0';
		// //
		// //token = strtok(null,delim);
		// //float mm_per_thirty = atof(token);
// //
		// //uint16_t step_dur = 0;
		// //for(uint8_t mot=0 ; mot<3 ; mot++)
		// //{
			// //if(motor_adjusts[direction][mot]!=0){
				// //step_dur += 32*motor_on_time + abs(motor_adjusts[direction][mot]) + 32*motor_off_time;
			// //}
		// //}
		// //
		// //
		// //uint16_t mm_per_kilostep = ((uint16_t)((mm_per_thirty*((float)step_dur))/960.0));
// //
		// //printf("setting mm_per_kilostep for dir: %hu to %u.\r\n", direction, mm_per_kilostep);
		// //set_mm_per_kilostep(direction, mm_per_kilostep);
		// //return 1;
	// //}
	// //
// //}
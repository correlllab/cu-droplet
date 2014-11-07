#include "DropletCustomEight.h"

DropletCustomEight::DropletCustomEight(ObjectPhysicsData *objPhysics)
	: DSimDroplet(objPhysics)
{
	return;
}

DropletCustomEight::~DropletCustomEight() { return; }

void DropletCustomEight::DropletInit()
{
	init_all_systems();
	char buffer[64];
	//sprintf(buffer,"C:\\Users\\Colab\\Desktop\\dropletSimDumps\\%04hxdat.txt",get_droplet_id());
	//sprintf(buffer,"C:\\Users\\Colab\\Desktop\\dropletSimDumps\\%04hxdat.txt",get_droplet_id());
	//file_handle = fopen (buffer,"w");
	set_rgb_led(0, 0, 0);

	state = STATE_PRE_ASSEMBLY;
	moving_state = MOVING_NORMAL;

	my_type = TYPE__;
	my_type_value = 0;
	my_filled_spots = 0;

	last_move_r = 0;
	last_move_theta = 0;
	last_move_dist = 0;
	last_goal_r = BIG_NUMBER;

	last_greater_val_time = 0;

	reset_before_waiting_for_msgs();
}

void DropletCustomEight::DropletMainLoop()
{
	if (state&STATE_IN_ASSEMBLY){
		if (state == STATE_AWAITING_CONFIRMATION){
			set_rgb_led(0, 250, 250);
			awaiting_confirmation();
		}
		else if (state == STATE_ADJ_SPOTS_TO_BE_FILLED){
			set_rgb_led(250, 250, 250);
			if (my_type&DISPLAY_0_TYPES) set_rgb_led(0, 0, 50);
			else if (my_type&DISPLAY_1_TYPES) set_rgb_led(255, 150, 0);
			adj_spots_to_fill();
		}
		else if (state == STATE_ALL_ADJ_SPOTS_FILLED){
			//no spots to fill.
			set_rgb_led(0, 0, 0);
			if (my_type&DISPLAY_0_TYPES) set_rgb_led(0, 0, 50);
			else if (my_type&DISPLAY_1_TYPES) set_rgb_led(255, 150, 0);
		}
		if (my_type != TYPE__){
			bool hard_spots_filled = !(get_spots_from_type(my_type) - (my_filled_spots & ~get_soft_spots_from_type(my_type)));
			if ((!hard_spots_filled || my_type_value == 0) && rand_byte()<8) broadcast_claim_msg(move_target, move_target_dir);
		}
		if (move_target != NULL) maintain_position(move_target, move_target_dir);
	}
	else{ //not in assembly
		if (state == STATE_MOVING_TO_SPOT){
			set_rgb_led(250, 0, 250);
			handle_move_to_spot();
		}
		else if (state == STATE_DECIDING_SHOULD_MOVE){
			set_rgb_led(250, 250, 0);
			if ((get_32bit_time() - moving_delay_start)>DELAY_BEFORE_MOVING_MS){
				check_ready_to_move();
			}
			else{
				decide_if_should_move();
			}
		}
		else if (state == STATE_ADJUSTING_PHI){
			set_rgb_led(0, 0, 250);
			handle_adjusting_phi();
		}
		else if (state == STATE_PRE_ASSEMBLY){
			set_rgb_led(0, 250, 0);
			handle_pre_assembly();
		}
		else if (state == STATE_WAITING_FOR_MSGS){ //essentially just waiting around for a message?
			waiting_for_message();
			set_rgb_led(250, 0, 0);
		}

	}
}

void DropletCustomEight::check_ready_to_move(){
	bool id_not_in_map = (recruiting_robots.find(closestID) == recruiting_robots.end());
	if (id_not_in_map || (!((recruiting_robots[closestID]->desiredNeighbors)&(1 << closestDir)))){ //our closest isn't in the dictionary anymore, so we need to pick a closest, broadcast, and reset countdown
		if (recruiting_robots.size() == 0){ //If the recruiting_robots dictionary is empty, then all the spots have been taken and we should just go back to waiting for messages.
			reset_before_waiting_for_msgs();
			state = STATE_WAITING_FOR_MSGS;
		}
		else{
			calculate_distance_to_target_positions();
			moving_delay_start = get_32bit_time();
		}
	}
	else{
		state = STATE_MOVING_TO_SPOT;
		move_target = closestID;
		move_target_dir = closestDir;
	}
}

void DropletCustomEight::decide_if_should_move(){
	if (rand_byte()<64){
		std::map<droplet_id_type, recruitingRobot*>::iterator iter;
		iter = recruiting_robots.find(closestID);
		if (iter != recruiting_robots.end()){ //if our target of choice hasn't already been completely claimed.
			if (iter->second->desiredNeighbors & (1 << closestDir)){ //and if our direction of choice hasn't been claimed.
				broadcast_favorite_target();
			}
		}
	}
	favTgtMsg* msg;
	while (check_for_new_messages()){
		if ((global_rx_buffer.data_len == sizeof(favTgtMsg)) && ((msg = ((favTgtMsg*)global_rx_buffer.buf))->type == FAV_TGT_MSG_TYPE)){
			favTgtMsg* msg = (favTgtMsg*)global_rx_buffer.buf;
			std::map<droplet_id_type, recruitingRobot*>::iterator iter;
			iter = recruiting_robots.find(msg->id);
			if (iter == recruiting_robots.end()){
				//the key isn't in our map, so we can ignore it.
			}
			else{
				if ((msg->id == closestID) && (msg->dir == closestDir)){ //if we both want the same one,
					if (iter->second->toNeighborDist[msg->dir] >= msg->dist){ //if my dist is as far or farther.
						if (iter->second->toNeighborDist[msg->dir] == msg->dist){ //if there was a tie.
							if (get_droplet_id()<global_rx_buffer.sender_ID){ //my id is lower, so keep it in the list.
							}
							else{ //their id is lower, so we need to remove the dir from our list.
								remove_dir_from_spots_map(msg->dir, msg->id);
							}
						}
						else{ //no tie, we're just worse, so remove.
							remove_dir_from_spots_map(msg->dir, msg->id);
						}
					}
					else{
						//We're closer, so keep that in the list.
					}
				}
				else{ //else, remove the dir from our list.
					remove_dir_from_spots_map(msg->dir, msg->id);
				}
			}
		}
		global_rx_buffer.read = 1;
	}
}

void DropletCustomEight::awaiting_confirmation(){
	claimMsg* claim;
	confMsg* conf;
	while (check_for_new_messages()){
		if ((global_rx_buffer.data_len == sizeof(confMsg)) && ((conf = (confMsg*)global_rx_buffer.buf)->type == CONF_MSG_TYPE)){			//someone sent out a confirmation message.
			if (conf->target == get_droplet_id()){
				if (global_rx_buffer.sender_ID == move_target) my_type_value = conf->value; //this is a hard confirmation

				if (my_type == TYPE__) my_type = conf->bot_type;
				else my_type &= conf->bot_type;

				if (my_type == TYPE__); //ERROR: mutually exclusive types given!
			}
		}
		else if ((global_rx_buffer.data_len == sizeof(claimMsg)) && ((claim = (claimMsg*)global_rx_buffer.buf)->type == CLAIM_MSG_TYPE)){
			//if (claim->bot_type_value>my_type_value) last_greater_val_time = get_32bit_time();
			//if (my_type&ALL_N_TYPES){
			//	if (claim->bot_type&ALL_W_TYPES){
			//		if (claim->bot_type_value>0){
			//			last_greater_val_time = get_32bit_time();
			//		}
			//	}
			//}
		}
		global_rx_buffer.read = 1;
	}
	if (one_bit_set(my_type)){
		if ((my_type_value <= 0) && (get_soft_spots_from_type(my_type) == 0)){
			state = STATE_ALL_ADJ_SPOTS_FILLED;
		}else if ((get_32bit_time() - last_greater_val_time)>WAIT_FOR_LAYER_DELAY){
			state = STATE_ADJ_SPOTS_TO_BE_FILLED;
			call_for_neighbors();
		}
	}else{
		if (rand_byte()<16){
			confReqMsg* msg = new confReqMsg;
			msg->type = CONF_REQ_MSG_TYPE;
			msg->target = move_target;
			msg->dir = move_target_dir;
			ir_broadcast((char*)msg, sizeof(confReqMsg)); //send request for confirmation.
			delete msg;
		}
	}
}

void DropletCustomEight::adj_spots_to_fill(){
	confReqMsg* confReq;
	while (check_for_new_messages()){
		if ((global_rx_buffer.data_len == sizeof(confReqMsg)) && ((confReq = ((confReqMsg*)global_rx_buffer.buf))->type == CONF_REQ_MSG_TYPE)){
			if (confReq->target == get_droplet_id()){
				handle_hard_confirm(global_rx_buffer.sender_ID, confReq->dir);
			}
			else{
				handle_soft_confirm(global_rx_buffer.sender_ID);
			}
		}
		global_rx_buffer.read = 1;
	}
	if (((get_spots_from_type(my_type) | get_soft_spots_from_type(my_type)) - my_filled_spots) == 0){
		state = STATE_ALL_ADJ_SPOTS_FILLED;
	}
	else{
		if (get_32bit_time() - neighbor_call_timeout_start>NEIGHBOR_CALL_TIMEOUT_TIMER_DELAY_MS){
			call_for_neighbors();
		}
	}

}

void DropletCustomEight::handle_move_to_spot(){
	if (is_moving(NULL)){
		//already moving, great.
	}
	else{
		float move_target_dist, move_target_theta, move_target_phi;
		range_and_bearing(move_target, &move_target_dist, &move_target_theta, &move_target_phi);
		float adj_move_target_dist, adj_move_target_theta;
		get_relative_neighbor_position(1 << move_target_dir, move_target_dist, move_target_theta, move_target_phi, &adj_move_target_dist, &adj_move_target_theta);
		if (moving_state == MOVING_NORMAL){
			float this_move_dist, this_move_theta;
			sub_polar_vec(adj_move_target_dist, adj_move_target_theta, last_move_r, last_move_theta, &this_move_dist, &this_move_theta);
			//fprintf(file_handle, "move_target: %hu, move_target_dir: %hhu, adj_move_target_dist: %f\n", move_target, move_target_dir, adj_move_target_dist);
			//fflush(file_handle);
			if (adj_move_target_dist<(PROXIMITY_THRESHOLD)){ //we're done moving!
				state = STATE_ADJUSTING_PHI;
				last_move_r = 0;
				last_move_dist = 0;
			}
			else if ((fabs(last_move_r - adj_move_target_dist))<STUCK_DIST_THRESHOLD){ //we started slowing down
				moving_state = MOVING_BACKING_UP;
				avoid_target = adj_move_target_dist;
				move_steps(TURN_ANTICLOCKWISE, ROTATE_STEPS_AMOUNT);
				last_move_r = 0;
				last_move_dist = 0;
				backing_up_start = get_32bit_time();
			}
			else if (adj_move_target_dist<DROPLET_RADIUS){ //we're almost done moving. Take smaller steps.
				move_steps(get_best_move_dir(adj_move_target_theta), MOVE_STEPS_AMOUNT / 3);
				last_move_r = adj_move_target_dist;
				last_move_dist = this_move_dist;
				last_move_theta = adj_move_target_theta;
			}
			else{
				handle_rotate_to_straight(adj_move_target_theta);
				move_steps(get_best_move_dir(adj_move_target_theta), MOVE_STEPS_AMOUNT);
				last_move_r = adj_move_target_dist;
				last_move_dist = this_move_dist;
				last_move_theta = adj_move_target_theta;
			}
		}
		else if (moving_state == MOVING_BACKING_UP){
			if (adj_move_target_dist >= (1.5*avoid_target) || ((get_32bit_time() - backing_up_start)>BACK_UP_TIMER_DELAY_MS)){

				moving_state = MOVING_SIDESTEPPING;
				side_step_angle = 60.0*(((rand_byte() & 0x1) ? -1.0 : 1.0)*((rand_byte() & 0x1) ? 2.0 : 1.0));

				backup_duration = get_32bit_time() - backing_up_start;
				if (backup_duration>SIDESTEPPING_DELAY_MS) backup_duration = SIDESTEPPING_DELAY_MS;
				sidestep_start = get_32bit_time();
			}
			else{
				move_steps(get_best_move_dir(adj_move_target_theta + 180), MOVE_STEPS_AMOUNT);
			}
		}
		else if (moving_state == MOVING_SIDESTEPPING){
			if ((get_32bit_time() - sidestep_start)>backup_duration){
				moving_state = MOVING_NORMAL;
			}
			else{
				if (adj_move_target_dist<3 * DROPLET_RADIUS){
					move_steps(get_best_move_dir(side_step_angle), MOVE_STEPS_AMOUNT);
				}
				else{
					move_steps(get_best_move_dir(adj_move_target_theta + 90), MOVE_STEPS_AMOUNT);
				}

			}
		}
	}
	bool stopping_move = false;
	claimMsg* claim;
	neighborCallMsg* neighbCall;
	while (check_for_new_messages()){
		if ((global_rx_buffer.data_len == sizeof(claimMsg)) && ((claim = (claimMsg*)global_rx_buffer.buf)->type == CLAIM_MSG_TYPE)){
			//got a message from someone claiming a spot.
			if ((move_target == claim->parent_id) && (move_target_dir == claim->dir)) 	stopping_move = true;
		}
		else if ((global_rx_buffer.data_len == sizeof(neighborCallMsg)) && ((neighbCall = (neighborCallMsg*)global_rx_buffer.buf)->type == NEIGHBOR_CALL_MSG_TYPE)){
			if ((global_rx_buffer.sender_ID == move_target) && (neighbCall->dir_mask&(1 << move_target_dir))) stopping_move = true;
		}
		global_rx_buffer.read = 1;
	}
	if (stopping_move){
		reset_before_waiting_for_msgs();
		state = STATE_WAITING_FOR_MSGS;
		move_steps(get_best_move_dir(last_move_theta + 180), 1000);
	}
}

void DropletCustomEight::handle_adjusting_phi(){
	if (is_moving(NULL)){
		//Do nothing.
	}
	else{
		float r, theta, phi;
		range_and_bearing(move_target, &r, &theta, &phi);
		float delta_phi = phi; //this is how much we need to change our phi by.
		delta_phi = quick_and_dirty_mod(delta_phi);
		if (fabs(delta_phi)<ORIENT_THRESHOLD_DEG){ //Are we done?
			state = STATE_AWAITING_CONFIRMATION;
		}
		else{
			if (delta_phi<0){
				move_steps(TURN_CLOCKWISE, ROTATE_STEPS_AMOUNT);
			}
			else{
				move_steps(TURN_ANTICLOCKWISE, ROTATE_STEPS_AMOUNT);
			}
		}
	}
}

void DropletCustomEight::handle_pre_assembly(){
	if (get_red_sensor()>10){
		my_type = TYPE_SEED;
		my_type_value = SEED_TYPE_VALUE;
		state = STATE_ADJ_SPOTS_TO_BE_FILLED;
		call_for_neighbors();
		return;
	}
	claimMsg* claim;
	confMsg* conf;
	neighborCallMsg* neighbCall;
	while (check_for_new_messages()){
		if (global_rx_buffer.data_len == sizeof(claimMsg) && ((claim = ((claimMsg*)global_rx_buffer.buf))->type == CLAIM_MSG_TYPE)){
			state = STATE_WAITING_FOR_MSGS;
			char msg = NOT_IN_ASSEMBLY_INDICATOR_BYTE;
			ir_broadcast(&msg, 1); //a little chirp to call other wandering bots in.
		}
		else if (global_rx_buffer.data_len == sizeof(confMsg) && ((conf = ((confMsg*)global_rx_buffer.buf))->type == CONF_MSG_TYPE)){
			state = STATE_WAITING_FOR_MSGS;
			char msg = NOT_IN_ASSEMBLY_INDICATOR_BYTE;
			ir_broadcast(&msg, 1); //a little chirp to call other wandering bots in.
		}
		else if (global_rx_buffer.data_len == sizeof(neighborCallMsg) && ((neighbCall = ((neighborCallMsg*)global_rx_buffer.buf))->type == NEIGHBOR_CALL_MSG_TYPE)){
			state = STATE_WAITING_FOR_MSGS;
			char msg = NOT_IN_ASSEMBLY_INDICATOR_BYTE;
			ir_broadcast(&msg, 1); //a little chirp to call other wandering bots in.
			add_recruiting_robot(global_rx_buffer.sender_ID, neighbCall->dir_mask);
		}
		else if (!is_moving(NULL)){
			float r, theta, phi;
			range_and_bearing(global_rx_buffer.sender_ID, &r, &theta, &phi);
			handle_rotate_to_straight(theta);
			move_steps(get_best_move_dir(theta), 3 * MOVE_STEPS_AMOUNT);
		}
		global_rx_buffer.read = 1;
	}
}

void DropletCustomEight::waiting_for_message(){
	if ((get_32bit_time() - deciding_delay_start) <= DELAY_BEFORE_DECIDING_MS || recruiting_robots.empty()){
		neighborCallMsg* neighbCall;
		while (check_for_new_messages()){
			if ((global_rx_buffer.data_len == sizeof(neighborCallMsg)) && ((neighbCall = ((neighborCallMsg*)global_rx_buffer.buf))->type == NEIGHBOR_CALL_MSG_TYPE)){ //the talking bot is in the assembly. We want to be in the assembly. Lets listen! 
				add_recruiting_robot(global_rx_buffer.sender_ID, neighbCall->dir_mask);
			}
			global_rx_buffer.read = 1;
		}
	}
	else{
		cancel_move(); //in case we were moving from backing up after our spot was claimed.
		state = STATE_DECIDING_SHOULD_MOVE;
	}
}

void DropletCustomEight::get_neighbor_type(uint16_t type, int8_t value, uint8_t dir, uint16_t* neighbor_type, int8_t* neighbor_value){
	switch (type){
	case TYPE_SEED:
		switch (dir){
		case DIR_MASK_N:	*neighbor_type = TYPE_N10;	 *neighbor_value = value; break;
		case DIR_MASK_W:	*neighbor_type = TYPE_W0; *neighbor_value = value - 1; break;
		}
		break;
	case TYPE_N11:
		switch (dir){
		case DIR_MASK_N: *neighbor_type = TYPE_N10; *neighbor_value = value; break;
		case DIR_MASK_W: *neighbor_type = (TYPE_W10 | TYPE_W11); *neighbor_value = value - 1; break;
		}
		break;
	case TYPE_N10:
		switch (dir){
		case DIR_MASK_N: *neighbor_type = TYPE_N11; *neighbor_value = value; break;
		case DIR_MASK_W: *neighbor_type = (TYPE_W00 | TYPE_W01); *neighbor_value = value - 1; break;
		}
		break;
	case TYPE_W0:
		switch (dir){
		case DIR_MASK_N: *neighbor_type = (TYPE_W00 | TYPE_W10); *neighbor_value = 0; break;
		case DIR_MASK_W: *neighbor_type = TYPE_W0; *neighbor_value = value - 1; break;
		}
		break;
	case TYPE_W00:
		switch (dir){
		case DIR_MASK_N: *neighbor_type = (TYPE_W00 | TYPE_W10); *neighbor_value = 0; break;
		case DIR_MASK_W: *neighbor_type = (TYPE_W00 | TYPE_W01); *neighbor_value = value - 1; break;
		}
		break;
	case TYPE_W01:
		switch (dir){
		case DIR_MASK_N: *neighbor_type = (TYPE_W01 | TYPE_W11); *neighbor_value = 0; break;
		case DIR_MASK_W: *neighbor_type = (TYPE_W00 | TYPE_W01); *neighbor_value = value - 1; break;
		}
		break;
	case TYPE_W10:
		switch (dir){
		case DIR_MASK_N: *neighbor_type = (TYPE_W01 | TYPE_W11); *neighbor_value = 0; break;
		case DIR_MASK_W: *neighbor_type = (TYPE_W00 | TYPE_W01); *neighbor_value = value - 1; break;
		}
		break;
	case TYPE_W11:
		switch (dir){
		case DIR_MASK_N: *neighbor_type = (TYPE_W00 | TYPE_W10); *neighbor_value = 0; break;
		case DIR_MASK_W: *neighbor_type = (TYPE_W10 | TYPE_W11); *neighbor_value = value - 1; break;
		}
		break;
	}
	//fprintf(file_handle, "\n\n\nERROR UNEXPECTED TYPE/DIR COMBO T:%hhx D:%hhx\n\n\n", type, dir);
	//fflush(file_handle);
}

//A 'soft' spot is a neighbor position about which you care what type goes there, 
//but aren't going to be directly responsible for recruiting robots to that spot.
uint8_t DropletCustomEight::get_soft_spots_from_type(uint16_t type){
	switch (type){
	case TYPE_W0: return DIR_MASK_N;
	case TYPE_W00: return DIR_MASK_N;
	case TYPE_W01: return DIR_MASK_N;
	case TYPE_W10: return DIR_MASK_N;
	case TYPE_W11: return DIR_MASK_N;
	default: return 0; //unexpected type, so no spots returned.
	}
}

uint8_t DropletCustomEight::get_spots_from_type(uint16_t type){
	switch (type){
	case TYPE_N10: return (DIR_MASK_N | DIR_MASK_W);
	case TYPE_N11: return (DIR_MASK_N | DIR_MASK_W);
	case TYPE_W0: return DIR_MASK_W;
	case TYPE_W00: return DIR_MASK_W;
	case TYPE_W01: return DIR_MASK_W;
	case TYPE_W10: return DIR_MASK_W;
	case TYPE_W11: return DIR_MASK_W;
	case TYPE_SEED: return (DIR_MASK_N | DIR_MASK_W); //all dirs
	default: return 0; //unexpected type, so no spots returned.
	}
}

//void DropletCustomEight::get_neighbor_type(uint16_t type, int8_t value, uint8_t dir, uint16_t* neighbor_type, int8_t* neighbor_value){
//	switch(type){
//		case TYPE_SEED:
//			switch(dir){
//				case DIR_MASK_N:	*neighbor_type=TYPE_N;	*neighbor_value=value-1; break;
//				case DIR_MASK_E:	*neighbor_type=TYPE_E;	*neighbor_value=value-1; break;
//				case DIR_MASK_W:	*neighbor_type=TYPE_W;	*neighbor_value=value-1; break;
//				case DIR_MASK_S:	*neighbor_type=TYPE_S;	*neighbor_value=value-1; break;
//				case DIR_MASK_NE:	*neighbor_type=TYPE_NE;	*neighbor_value=value-1; break;
//				case DIR_MASK_NW:	*neighbor_type=TYPE_NW;	*neighbor_value=value-1; break;
//				case DIR_MASK_SE:	*neighbor_type=TYPE_SE;	*neighbor_value=value-1; break;
//				case DIR_MASK_SW:	*neighbor_type=TYPE_SW;	*neighbor_value=value-1; break; 
//			}
//			break;
//		case TYPE_N:
//			switch(dir){
//				case DIR_MASK_N: *neighbor_type=TYPE_N; *neighbor_value=value-1; break;
//			}
//			break;
//		case TYPE_S:
//			switch(dir){
//				case DIR_MASK_S: *neighbor_type=TYPE_S; *neighbor_value=value-1; break;
//			}
//			break;
//		case TYPE_E:
//			switch(dir){
//				case DIR_MASK_E: *neighbor_type=TYPE_E; *neighbor_value=value-1; break;
//			}
//			break;
//		case TYPE_W:
//			switch(dir){
//				case DIR_MASK_W: *neighbor_type=TYPE_W; *neighbor_value=value-1; break;
//			}
//			break;
//		case TYPE_NE:
//			switch(dir){ 
//				case DIR_MASK_N:  *neighbor_type=TYPE_N; *neighbor_value=value-1; break;
//				case DIR_MASK_NE: *neighbor_type=TYPE_NE; *neighbor_value=value-1; break;
//				case DIR_MASK_E:  *neighbor_type=TYPE_E; *neighbor_value=value-1; break;
//			}
//			break;
//		case TYPE_NW:
//			switch(dir){
//				case DIR_MASK_N:  *neighbor_type=TYPE_N; *neighbor_value=value-1; break;
//				case DIR_MASK_NW: *neighbor_type=TYPE_NW; *neighbor_value=value-1; break;
//				case DIR_MASK_W:  *neighbor_type=TYPE_W; *neighbor_value=value-1; break;
//			}
//			break;
//		case TYPE_SE:
//			switch(dir){
//				case DIR_MASK_S:  *neighbor_type=TYPE_S; *neighbor_value=value-1; break;
//				case DIR_MASK_SE: *neighbor_type=TYPE_SE; *neighbor_value=value-1; break;
//				case DIR_MASK_E:  *neighbor_type=TYPE_E; *neighbor_value=value-1; break;
//			}
//			break;
//		case TYPE_SW:
//			switch(dir){
//				case DIR_MASK_S:  *neighbor_type=TYPE_S; *neighbor_value=value-1; break;
//				case DIR_MASK_SW: *neighbor_type=TYPE_SW; *neighbor_value=value-1; break;
//				case DIR_MASK_W:  *neighbor_type=TYPE_W; *neighbor_value=value-1; break;
//			}
//			break;
//	}
//	//fprintf(file_handle, "\n\n\nERROR UNEXPECTED TYPE/DIR COMBO T:%hhx D:%hhx\n\n\n", type, dir);
//	//fflush(file_handle);
//}
//
//uint8_t DropletCustomEight::get_spots_from_type(uint16_t type){
//	switch (type){
//	case TYPE_N:	return DIR_MASK_N;
//	case TYPE_E:	return DIR_MASK_E;
//	case TYPE_W:	return DIR_MASK_W;
//	case TYPE_S:	return DIR_MASK_S;
//	case TYPE_NE:	return (DIR_MASK_N | DIR_MASK_NE | DIR_MASK_E);
//	case TYPE_NW:	return (DIR_MASK_N | DIR_MASK_NW | DIR_MASK_W);
//	case TYPE_SE:	return (DIR_MASK_S | DIR_MASK_SE | DIR_MASK_E);
//	case TYPE_SW:	return (DIR_MASK_S | DIR_MASK_SW | DIR_MASK_W);
//	case TYPE_SEED: return 0xFF; //all dirs
//	default: return 0; //unexpected type, so no spots returned.
//	}
//}
//
////A 'soft' spot is a neighbor position about which you care what type goes there, 
////but aren't going to be directly responsible for recruiting robots to that spot.
//uint8_t DropletCustomEight::get_soft_spots_from_type(uint16_t type){
//	switch(type){
//		default: return 0; //unexpected type, so no spots returned.
//	}	
//}



void DropletCustomEight::handle_soft_confirm(droplet_id_type sender){
	float rOther, thetaOther, phiOther;
	range_and_bearing(sender, &rOther, &thetaOther, &phiOther);
	if (rOther<7){ //close enough to be an immediate neighbor.
		uint8_t dirFromMe = getDirMaskFromAngle(thetaOther);
		if (get_soft_spots_from_type(my_type)&dirFromMe){
			//this means we care about this direction, even if it isn't our child directly.
			uint16_t bot_type;
			int8_t bot_value;



			get_neighbor_type(my_type, my_type_value, dirFromMe, &bot_type, &bot_value);
			confMsg* msg = new confMsg;
			msg->type = CONF_MSG_TYPE;
			msg->target = sender;
			msg->bot_type = bot_type;
			msg->value = bot_value;
			//fprintf(file_handle, "Sending (soft) confirmation msg to %hx, type: %hx, val: %hhd.\n",msg->target, msg->bot_type, msg->value);
			//fflush(file_handle);
			ir_broadcast((char*)msg, sizeof(confMsg));
			delete msg;
			my_filled_spots |= dirFromMe; //mark that spot as done.

		}
	}
}

void DropletCustomEight::handle_hard_confirm(droplet_id_type sender, uint8_t dir){
	float rOther, thetaOther, phiOther;
	range_and_bearing(sender, &rOther, &thetaOther, &phiOther);
	float desiredR = 2.*DROPLET_RADIUS + FORMATION_GAP;
	if (ANGLED_DIR&(1 << dir)) desiredR *= M_SQRT2; //the angled sides should be farther away.
	float desiredTh = getAngleFromDirMask(1 << dir);
	float dist, th;
	sub_polar_vec(rOther, thetaOther, desiredR, desiredTh, &dist, &th);
	if (dist <= 2.0*PROXIMITY_THRESHOLD){
		//close enough! send confirmation message.
		uint16_t bot_type;
		int8_t bot_value;

		get_neighbor_type(my_type, my_type_value, 1 << dir, &bot_type, &bot_value);
		confMsg* msg = new confMsg;
		msg->type = CONF_MSG_TYPE;
		msg->target = sender;
		msg->bot_type = bot_type;
		msg->value = bot_value;
		//fprintf(file_handle, "Sending (hard) confirmation msg to %hx, type: %hx, val: %hhd.\n",msg->target, msg->bot_type, msg->value);
		//fflush(file_handle);
		ir_broadcast((char*)msg, sizeof(confMsg));
		delete msg;
		my_filled_spots |= (1 << dir); //mark that spot as done.
	}
	else{
		//Not close enough. Need to do something?
	}
}

void DropletCustomEight::get_dir_string_from_dir(uint8_t dir_mask, char* dirStr){
	dirStr[1] = ' ';
	switch (dir_mask){
	case DIR_MASK_N:	dirStr[0] = 'N';	return;
	case DIR_MASK_NE:	dirStr[0] = 'N'; dirStr[1] = 'E';	return;
	case DIR_MASK_E:	dirStr[0] = 'E';	return;
	case DIR_MASK_SE:	dirStr[0] = 'S'; dirStr[1] = 'E';	return;
	case DIR_MASK_S:	dirStr[0] = 'S';	return;
	case DIR_MASK_SW:	dirStr[0] = 'S'; dirStr[1] = 'W';	return;
	case DIR_MASK_W:	dirStr[0] = 'W';	return;
	case DIR_MASK_NW:	dirStr[0] = 'N'; dirStr[1] = 'W';	return;
	}
}

void DropletCustomEight::add_recruiting_robot(droplet_id_type id, uint8_t dirs){
	//Once we've added a recruiter, other recruiters have DELAY_BEFORE_DECIDING_MS to make themselves known.
	if (recruiting_robots.empty()) deciding_delay_start = get_32bit_time();
	float r, theta, phi;
	range_and_bearing(id, &r, &theta, &phi);

	recruitingRobot* target = new recruitingRobot;
	target->desiredNeighbors = dirs;
	target->range = r;
	target->bearing = theta;
	target->heading = phi;
	recruiting_robots[id] = target;
}

void DropletCustomEight::reset_before_waiting_for_msgs(){
	move_target = NULL;
	move_target_dir = 0;
	closestDir = 0;
	closestID = 0;
	closestDist = 0;
	recruiting_robots.clear();
}

void DropletCustomEight::handle_rotate_to_straight(float theta){
	if ((-30 <= theta) && (theta<30)){
		//do nothing
	}
	else if ((-90 <= theta) && (theta<-30)){
		theta += 60.0;
	}
	else if ((-150 <= theta) && (theta<-90)){
		theta += 120.0;
	}
	else if (((-180 <= theta) && (theta<-150)) || ((150 <= theta) && (theta <= 180))){
		if (theta<0) theta += 180.0;
		else theta -= 180.0;
	}
	else if ((90 <= theta) && (theta<150)){
		theta -= 120;
	}
	else if ((30 <= theta) && (theta<90)){
		theta -= 60;
	}
	if (theta<(ORIENT_THRESHOLD_DEG / 2.)) move_steps(TURN_CLOCKWISE, ROTATE_STEPS_AMOUNT);
	else if (theta>(ORIENT_THRESHOLD_DEG / 2.)) move_steps(TURN_ANTICLOCKWISE, ROTATE_STEPS_AMOUNT);
}


void DropletCustomEight::call_for_neighbors(){
	if (my_type_value <= 0) return;
	//fprintf(file_handle, "Calling for neighbors. My filled spots: %hhx, hard_spots: %hhx, soft_spots: %hhx.\n", my_filled_spots, get_spots_from_type(my_type), get_soft_spots_from_type(my_type));
	neighborCallMsg* msg = new neighborCallMsg;
	msg->type = NEIGHBOR_CALL_MSG_TYPE;
	msg->dir_mask = get_spots_from_type(my_type) - (my_filled_spots & ~get_soft_spots_from_type(my_type));
	//fprintf(file_handle, "\tasking for: %hhx\n", msg->dir_mask);
	//fflush(file_handle);
	if (msg->dir_mask) ir_broadcast((char*)msg, sizeof(neighborCallMsg));
	delete msg;
	neighbor_call_timeout_start = get_32bit_time();
}

bool DropletCustomEight::check_if_stuck(float delta, float last_delta){
	if (last_delta<1){ //no bot moves that fast; this checks for the weird cases at the very start of motion.
		if (last_delta*0.5>delta){ //we slowed down by more than 50%
			return true;
		}
		else if (delta<STUCK_DIST_THRESHOLD){ //we're barely moving.
			return true;
		}
	}
	return false;
}

void DropletCustomEight::maintain_position(droplet_id_type bot, uint8_t dir){
	if (is_moving(NULL)){
		//already moving, great.
	}
	else{
		float bot_r, bot_theta, bot_phi;
		range_and_bearing(bot, &bot_r, &bot_theta, &bot_phi);
		float goal_r, goal_theta;
		get_relative_neighbor_position((1 << dir), bot_r, bot_theta, bot_phi, &goal_r, &goal_theta);
		if (goal_r<(PROXIMITY_THRESHOLD / 5.0)){
			//We're close enough! Keep it up. Check our rotation.
			float delta_phi = bot_phi/* + getAngleFromDirMask((1<<dir))*/; //this is how much we need to change our phi by.
			delta_phi = quick_and_dirty_mod(delta_phi);
			if (fabs(delta_phi)<ORIENT_THRESHOLD_DEG){
				//Rotation good too! Nice.
			}
			else{
				if (delta_phi<0) move_steps(TURN_CLOCKWISE, ROTATE_STEPS_AMOUNT);
				else move_steps(TURN_ANTICLOCKWISE, ROTATE_STEPS_AMOUNT);
			}
		}
		else{
			move_steps(get_best_move_dir(goal_theta), 2);
		}
	}
}

void DropletCustomEight::broadcast_favorite_target(){
	favTgtMsg* msg = new favTgtMsg;
	msg->type = FAV_TGT_MSG_TYPE;
	msg->id = closestID;
	msg->dir = closestDir;
	msg->dist = closestDist;
	ir_broadcast((char*)msg, sizeof(favTgtMsg));
	delete msg;
}

void DropletCustomEight::broadcast_claim_msg(droplet_id_type parent, uint8_t dir){
	claimMsg* msg = new claimMsg;
	msg->type = CLAIM_MSG_TYPE;
	msg->parent_id = parent;
	msg->dir = dir;
	msg->bot_type = my_type;
	msg->bot_type_value = my_type_value;
	ir_broadcast((char*)msg, sizeof(claimMsg));
}

move_direction DropletCustomEight::get_best_move_dir(float theta){
	theta = quick_and_dirty_mod(theta);
	if ((-30 <= theta) && (theta<30)){
		return NORTH;
	}
	else if ((-90 <= theta) && (theta<-30)){
		return NORTH_EAST;
	}
	else if ((-150 <= theta) && (theta<-90)){
		return SOUTH_EAST;
	}
	else if (((-180 <= theta) && (theta<-150)) || ((150 <= theta) && (theta <= 180))){
		return SOUTH;
	}
	else if ((90 <= theta) && (theta<150)){
		return SOUTH_WEST;
	}
	else if ((30 <= theta) && (theta<90)){
		return NORTH_WEST;
	}
	//fprintf(file_handle,"\n\n\n\n\nERROR ERROR UNEXPECTED DIRECTION FOR THETA: %f ERROR ERROR \n\n\n\n\n",theta);
	//fflush(file_handle);
	return 7;
}

void DropletCustomEight::calculate_distance_to_target_positions(){
	closestDist = BIG_NUMBER;
	closestDir = 0;
	closestID = 0;
	//need to convert from a list of bots and desired directions to a list of positions.
	//fprintf(file_handle, "Vectors from me:\n");
	//for(std::map<droplet_id_type, recruitingRobot*>::iterator iter=recruiting_robots.begin() ; iter!=recruiting_robots.end(); iter++){
	//	fprintf(file_handle, "\t%hx: %f, %f\n", iter->first, iter->second->range, iter->second->bearing);
	//}
	//fprintf(file_handle, "Dists:\n      ");
	//for(std::map<droplet_id_type, recruitingRobot*>::reverse_iterator iter=recruiting_robots.rbegin() ; iter!=recruiting_robots.rend(); iter++){
	//	fprintf(file_handle, "     %hx     | ", iter->first);
	//}
	//fprintf(file_handle, "\n");
	//for(std::map<droplet_id_type, recruitingRobot*>::iterator iterA=recruiting_robots.begin() ; iterA!=recruiting_robots.end(); iterA++){
	//	fprintf(file_handle, "%hx  ", iterA->first);
	//	for(std::map<droplet_id_type, recruitingRobot*>::reverse_iterator iterB=recruiting_robots.rbegin() ; iterB->first!=iterA->first; iterB++){
	//		float r, th;
	//		sub_polar_vec(iterB->second->range, iterB->second->bearing, iterA->second->range, iterA->second->bearing, &r, &th);
	//		//gives a vector pointing from a to b.
	//		fprintf(file_handle, "%+3.2f,%+3.2f | ", r, th);
	//	}
	//	fprintf(file_handle, "\n");
	//}
	//fprintf(file_handle, "\n");
	//for(std::map<droplet_id_type, recruitingRobot*>::iterator iterA=recruiting_robots.begin() ; iterA!=recruiting_robots.end(); iterA++){
	//	for(std::map<droplet_id_type, recruitingRobot*>::reverse_iterator iterB=recruiting_robots.rbegin() ; iterB->first!=iterA->first; iterB++){
	//		float r, th;
	//		sub_polar_vec(iterB->second->range, iterB->second->bearing, iterA->second->range, iterA->second->bearing, &r, &th);
	//		if(r<=7){ //these two bots are neighbors.
	//			char dirStr[2];
	//			get_dir_string_from_dir(getDirMaskFromAngle(th), dirStr);
	//			fprintf(file_handle, "%hx is %c%c of %hx\n", iterA->first, dirStr[0], dirStr[1], iterB->first);
	//			fflush(file_handle);
	//		}
	//	}
	//}
	//fflush(file_handle);

	for (std::map<droplet_id_type, recruitingRobot*>::iterator iter = recruiting_robots.begin(); iter != recruiting_robots.end(); iter++){
		for (uint8_t dir = 0; dir<8; dir++){
			if (iter->second->desiredNeighbors & (0x1 << dir)){
				get_relative_neighbor_position(0x1 << dir, iter->second->range, iter->second->bearing, iter->second->heading, &(iter->second->toNeighborDist[dir]), &(iter->second->toNeighborTheta[dir]));
			}
			else{
				iter->second->toNeighborDist[dir] = BIG_NUMBER;
			}
			if (iter->second->toNeighborDist[dir]<closestDist){
				closestDist = iter->second->toNeighborDist[dir];
				closestID = iter->first;
				closestDir = dir;
			}
		}
	}
	////fprintf(file_handle, "Closest: {ID: %04hx, DIR: %02hhx, DIST: %f}\n",closestID, closestDir, closestDist);
}

float DropletCustomEight::getAngleFromDirMask(uint8_t dir_mask){
	float angle = 0;
	switch (dir_mask){
	case DIR_MASK_N:	angle = 0.0;	break;
	case DIR_MASK_NE:	angle = -45.0;	break;
	case DIR_MASK_E:	angle = -90.0;	break;
	case DIR_MASK_SE:	angle = -135.0;	break;
	case DIR_MASK_S:	angle = -180.0;	break;
	case DIR_MASK_SW:	angle = 135.0;	break;
	case DIR_MASK_W:	angle = 90.0;	break;
	case DIR_MASK_NW:	angle = 45.0;	break;
	}
	return angle;
}

uint8_t DropletCustomEight::getDirMaskFromAngle(float theta){
	//theta = quick_and_dirty_mod(theta);
	if ((-22.5 <= theta) && (theta<22.5)) return DIR_MASK_N;
	else if ((-67.5 <= theta) && (theta<-22.5)) return DIR_MASK_NW;
	else if ((-112.5 <= theta) && (theta<-67.5)) return DIR_MASK_W;
	else if ((-157.5 <= theta) && (theta<-112.5)) return DIR_MASK_SW;
	else if (((-180. <= theta) && (theta<-157.5)) || ((157.5 <= theta) && (theta <= 180.))) return DIR_MASK_S;
	else if ((112.5 <= theta) && (theta<157.5)) return DIR_MASK_SE;
	else if ((67.5 <= theta) && (theta<112.5)) return DIR_MASK_E;
	else if ((22.5 <= theta) && (theta<67.5)) return DIR_MASK_NE;
	else		return 0xFF;
}

void DropletCustomEight::get_relative_neighbor_position(uint8_t dir_mask, float neighbor_r, float neighbor_theta, float neighbor_phi, float* target_r, float* target_theta){
	float delta_r = DROPLET_RADIUS * 2 + FORMATION_GAP;
	if (ANGLED_DIR&dir_mask) delta_r *= M_SQRT2; //the angled sides should be farther away.
	float delta_theta = neighbor_phi + getAngleFromDirMask(dir_mask);
	add_polar_vec(neighbor_r, neighbor_theta, delta_r, delta_theta, target_r, target_theta);
}

void DropletCustomEight::add_polar_vec(float r1, float th1, float r2, float th2, float* rs, float* ths){
	float x1 = r1*cos(deg2rad(th1));
	float y1 = r1*sin(deg2rad(th1));
	float x2 = r2*cos(deg2rad(th2));
	float y2 = r2*sin(deg2rad(th2));
	float xs = x1 + x2;
	float ys = y1 + y2;
	*rs = sqrt(xs*xs + ys*ys);
	*ths = rad2deg(atan2(ys, xs));
}

void DropletCustomEight::sub_polar_vec(float r1, float th1, float r2, float th2, float* rs, float* ths){
	float x1 = r1*cos(deg2rad(th1));
	float y1 = r1*sin(deg2rad(th1));
	float x2 = r2*cos(deg2rad(th2));
	float y2 = r2*sin(deg2rad(th2));
	float xs = x1 - x2;
	float ys = y1 - y2;
	*rs = sqrt(xs*xs + ys*ys);
	*ths = rad2deg(atan2(ys, xs));
}

void DropletCustomEight::remove_dir_from_spots_map(uint8_t dir, droplet_id_type id){
	////fprintf(file_handle, "want neighb.: %hhx; ", recruiting_robots[id]->desiredNeighbors);
	recruiting_robots[id]->desiredNeighbors &= ~((1 << dir));
	////fprintf(file_handle, " removed %hhu; now want neighb: %hhx\n", dir, recruiting_robots[id]->desiredNeighbors);
	if ((recruiting_robots[id]->desiredNeighbors) == 0){ //if no neighbors left, remove this robot from our list.
		////fprintf(file_handle, "\tNo neighbors left!\n");
		delete recruiting_robots[id];
		recruiting_robots.erase(recruiting_robots.find(id));
	}
	//fflush(file_handle);
}

float DropletCustomEight::deg2rad(float deg){
	return (float)((deg*((float)M_PI)) / 180.0);
}

float DropletCustomEight::rad2deg(float rad){
	return (float)((rad*180.0) / ((float)M_PI));
}

float DropletCustomEight::quick_and_dirty_mod(float theta){
	while (theta>180.0){
		theta = theta - 360.0;
	}
	while (theta<-180.0){
		theta = theta + 360.0;
	}
	return theta;
}
bool DropletCustomEight::one_bit_set(uint16_t n){
	//fprintf(file_handle, "n: %hx, bool: %s\n", n, ((n && !(n & (n - 1))) ? "True" : "False"));
	//fflush(file_handle);
	return (n && !(n & (n - 1)));
}
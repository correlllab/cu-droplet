#include "DropletCustomEight.h"

DropletCustomEight::DropletCustomEight(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics) 
{ return; }

DropletCustomEight::~DropletCustomEight() { return; }

void DropletCustomEight::DropletInit()
{
	init_all_systems();
	char buffer[64];
	//sprintf(buffer,"C:\\Users\\Colab\\Desktop\\dropletSimDumps\\%04hxdat.txt",get_droplet_id());
	//file_handle = fopen (buffer,"w");
	set_rgb_led(0,0,0);

	state = STATE_START;
	moving_state=MOVING_NORMAL;

	min_id = get_droplet_id();

	my_type = TYPE__;
	my_type_value=0;
	my_filled_spots=0;

	last_move_r=0;
	last_move_theta=0;
	last_move_dist=0;
	last_goal_r=BIG_NUMBER;

	other_bots.clear();
	heard_the_assembly=false;

	got_go_from_parent=false;
	smallest_master_value= 0xFF;
	reset_before_waiting_for_msgs();
	set_timer(START_DELAY_MS,START_DELAY_TIMER);
}

void DropletCustomEight::DropletMainLoop()
{
	if(state&STATE_START){
		handle_start_broadcast();
	}else if(state&STATE_IN_ASSEMBLY){
		if(state==STATE_AWAITING_CONFIRMATION){
			set_rgb_led(0,250,250);
			awaiting_confirmation();
		}else if(state==STATE_ADJ_SPOTS_TO_BE_FILLED){
			set_rgb_led(250,250,250);
			adj_spots_to_fill();
		}else if(state==STATE_MOVING_TO_CENTER){
			set_rgb_led(0,250,0);
			handle_move_to_center();
		}else if(state==STATE_ALL_ADJ_SPOTS_FILLED){
			//no spots to fill.
			set_rgb_led(0,0,0);
		}
		if(move_target!=NULL){
			if(state!=STATE_ALL_ADJ_SPOTS_FILLED){ //if this bot is in the outer layer.
				if(rand_byte()<16) broadcast_claim_msg(move_target, move_target_dir);
			}
			maintain_position(move_target, move_target_dir);
		}
	}else{ //not in assembly
		if(state==STATE_MOVING_TO_SPOT){
			set_rgb_led(250,0,250);
			handle_move_to_spot();
		}
		else if(state==STATE_DECIDING_SHOULD_MOVE){
			set_rgb_led(250,250,0);
			if(check_timer(MOVING_DELAY_TIMER)){
				check_ready_to_move();
			}else{
				decide_if_should_move();
			}
		}else if(state==STATE_ADJUSTING_PHI){
			set_rgb_led(0,0,250);
			handle_adjusting_phi();
		}else if (state==STATE_WAITING_FOR_MSGS){ //essentially just waiting around for a message?
				waiting_for_message();
				set_rgb_led(250,0,0);
		}
			
	}
}

void DropletCustomEight::handle_start_broadcast(){	
	if(check_timer(START_DELAY_TIMER)){
		if(min_id == get_droplet_id()){ //I'm the seed, hurray!
			//fprintf(file_handle, "I AM THE SEED. My id: %hx.\n", get_droplet_id());
			//fflush(file_handle);
			my_type=TYPE_SEED;
			my_type_value=SEED_TYPE_VALUE;
			state=STATE_MOVING_TO_CENTER;
		}else{
			reset_before_waiting_for_msgs();
			state=STATE_WAITING_FOR_MSGS;
			set_timer(RANDOM_WALK_DELAY_MS, RANDOM_WALK_TIMER);
		}
	}else{
		if(rand_byte()<64){
			char id_msg[1+sizeof(droplet_id_type)];
			id_msg[0]=START_INDICATOR_BYTE;
			*((droplet_id_type*)(id_msg+1)) = min_id;
			ir_broadcast(id_msg, 1+sizeof(droplet_id_type));
		}
		while(check_for_new_messages()){
			if(((global_rx_buffer.data_len-2 /*JOHN: data_len has a bug?*/)==1+sizeof(droplet_id_type))&&(global_rx_buffer.buf[0]==START_INDICATOR_BYTE)){
				droplet_id_type in_id = *((droplet_id_type*)(global_rx_buffer.buf+1));
				if(in_id<get_droplet_id()){
					min_id = in_id;
					//I'm not the smallest, so don't need to keep track of other bots anymore.
					for(std::vector<botPos*>::iterator it=other_bots.begin(); it!=other_bots.end();it++){
						delete (*it);
					}
					other_bots.clear();
				}else{
					botPos* bot = new botPos;
					bot->id = global_rx_buffer.sender_ID;
					float phi;
					range_and_bearing(bot->id, &(bot->r), &(bot->theta), &phi);
					other_bots.push_back(bot);
				}
			}
			global_rx_buffer.read=1;
		}
	}
}

void DropletCustomEight::handle_move_to_center(){
	if(is_moving()){
		//do nothing.
	}else{
		float dx=0, dy=0, phi;
		for(std::vector<botPos*>::iterator it=other_bots.begin(); it!=other_bots.end();it++){
			range_and_bearing((*it)->id,&((*it)->r), &((*it)->theta), &phi);
			dx+=(*it)->r*cos(deg2rad((*it)->theta));
			dy+=(*it)->r*sin(deg2rad((*it)->theta));
		}
		dx/=other_bots.size();
		dy/=other_bots.size();
		float goal_r = sqrt(dx*dx+dy*dy);
		float goal_theta = rad2deg(atan2(dy, dx));
		
		if(moving_state==MOVING_NORMAL){
			float this_move_dist=sub_polar_vec_mag(goal_r, goal_theta, last_move_r, last_move_theta);
			if(goal_r<(PROXIMITY_THRESHOLD)){
				for(std::vector<botPos*>::iterator it=other_bots.begin(); it!=other_bots.end();it++){
					delete (*it);
				}
				other_bots.clear();
				state=STATE_ADJ_SPOTS_TO_BE_FILLED;
				last_move_r=0;
				last_move_dist=0;
				call_for_neighbors();
			}
			else if(fabs(last_move_r-goal_r)<STUCK_DIST_THRESHOLD){ //we barely moved at all! probably stuck.
				moving_state = MOVING_BACKING_UP;
				avoid_target = goal_r;
				last_move_r=BIG_NUMBER;
				last_move_dist=0;
			}else if(goal_r<DROPLET_RADIUS){ //we're almost done moving. Take smaller steps.
				move_steps(get_best_move_dir(goal_theta), 5);
				last_move_r = goal_r;
				last_move_dist = this_move_dist;
				last_move_theta = goal_theta;
			}
			else{
				move_steps(get_best_move_dir(goal_theta), MOVE_STEPS_AMOUNT);
				last_move_r = goal_r;
				last_move_theta = goal_theta;
				last_move_dist=this_move_dist;
			}
		}else if(moving_state==MOVING_BACKING_UP){
			if(goal_r>=(1.2*avoid_target)){
				moving_state = MOVING_SIDESTEPPING;
				set_timer(SIDESTEPPING_DELAY_MS, SIDESTEP_TIMER);
			}else{
				move_steps(get_best_move_dir(goal_theta+180), MOVE_STEPS_AMOUNT);
			}
		}else if(moving_state==MOVING_SIDESTEPPING){
			if(check_timer(SIDESTEP_TIMER)){
				moving_state=MOVING_NORMAL;
			}else{
				move_steps(get_best_move_dir(goal_theta-90), MOVE_STEPS_AMOUNT);
			}
		}
	}
}

void DropletCustomEight::check_ready_to_move(){
	bool id_not_in_map = (recruiting_robots.find(closestID) == recruiting_robots.end());
	if(id_not_in_map||(!((recruiting_robots[closestID]->desiredNeighbors)&(1<<closestDir)))){ //our closest isn't in the dictionary anymore, so we need to pick a closest, broadcast, and reset countdown
		if(recruiting_robots.size()==0){ //If the recruiting_robots dictionary is empty, then all the spots have been taken and we should just go back to waiting for messages.
			reset_before_waiting_for_msgs();
			state = STATE_WAITING_FOR_MSGS;
		}else{
			calculate_distance_to_target_positions();
			set_timer(DELAY_BEFORE_MOVING_MS,MOVING_DELAY_TIMER);
		}
	}else{
		state = STATE_MOVING_TO_SPOT;
		move_target = closestID;
		move_target_dir = closestDir;
		set_timer(NEIGHBOR_CALL_TIMEOUT_TIMER_DELAY_MS, MOVE_TIMEOUT_TIMER);
	}
}

void DropletCustomEight::decide_if_should_move(){
	if(rand_byte()<64){
		std::map<droplet_id_type, recruitingRobot*>::iterator iter;
		iter = recruiting_robots.find(closestID);
		if(iter!=recruiting_robots.end()){ //if our target of choice hasn't already been completely claimed.
			if(iter->second->desiredNeighbors & (1<<closestDir)){ //and if our direction of choice hasn't been claimed.
				broadcast_favorite_target();
			}else{
			}
		}else{
		}
	}

	while(check_for_new_messages()){
		if(((global_rx_buffer.data_len-2 /*JOHN: data_len has a bug?*/)==sizeof(favTgtMsg))){
			favTgtMsg* msg = (favTgtMsg*)global_rx_buffer.buf;
			std::map<droplet_id_type, recruitingRobot*>::iterator iter;
			iter = recruiting_robots.find(msg->id);
			if(iter == recruiting_robots.end()){
				//the key isn't in our map, so we can ignore it.
			}else{
				if((msg->id==closestID)&&(msg->dir==closestDir)){ //if we both want the same one,
					if(iter->second->toNeighborDist[msg->dir]>=msg->dist){ //if my dist is as far or farther.
						if(iter->second->toNeighborDist[msg->dir]==msg->dist){ //if there was a tie.
							if(get_droplet_id()<global_rx_buffer.sender_ID){ //my id is lower, so keep it in the list.
							}else{ //their id is lower, so we need to remove the dir from our list.
								remove_dir_from_spots_map(msg->dir, msg->id);
							}
						}else{ //no tie, we're just worse, so remove.
							remove_dir_from_spots_map(msg->dir, msg->id);
						}
					}else{
						//We're closer, so keep that in the list.
					}
				}else{ //else, remove the dir from our list.
					remove_dir_from_spots_map(msg->dir, msg->id);
				}
			}
		}
		global_rx_buffer.read = 1;
	}
}

void DropletCustomEight::awaiting_confirmation(){
	if(my_type==TYPE__){ //if we haven't gotten a type yet.
		if(rand_byte()<64){
			char msg[4];
			msg[0] = NOT_IN_ASSEMBLY_INDICATOR_BYTE;
			*((droplet_id_type*)(msg+1)) = move_target;
			msg[3] = move_target_dir;
			ir_broadcast(msg, 4); //send request for confirmation.
		}
	}else{
		set_rgb_led(250, 250, 250);
	}
	while(check_for_new_messages()){
		if((global_rx_buffer.buf[0]==IN_ASSEMBLY_INDICATOR_BYTE)&&((global_rx_buffer.data_len-2 /*JOHN: data_len has a bug?*/)==6)){
			//someone sent out a confirmation message.
			droplet_id_type ack_tgt = *((droplet_id_type*)(global_rx_buffer.buf+1));
			uint16_t type_from_msg = *((uint16_t*)(global_rx_buffer.buf+3));
			int8_t type_val_from_msg = *((int8_t*)(global_rx_buffer.buf+5));
			if((my_type==TYPE__)&&(ack_tgt==get_droplet_id())){
				//the message is for me, and I don't have a type yet. So I get a type, and confirmation that I'm in the right spot.
				my_type = type_from_msg;
				my_type_value = type_val_from_msg;
			}
			if(check_if_slave(my_type, type_from_msg)){
				if(type_val_from_msg<smallest_master_value){
					smallest_master_value = type_val_from_msg;
				}
			}
		}else if((global_rx_buffer.buf[0]==GO_INDICATOR_BYTE)&&(global_rx_buffer.sender_ID==move_target)){
			//our parent sent out a 'go' message.
			got_go_from_parent=true;
		}
		global_rx_buffer.read=1;
	}
	if(got_go_from_parent){
		//fprintf(file_handle, "got 'go' from parent, and ");
		if(my_type_value<=0){
			state=STATE_ALL_ADJ_SPOTS_FILLED;
			//fprintf(file_handle, "all done.\n");

		}else if((my_type_value>=smallest_master_value)||(!check_if_slave(my_type, NULL))){
			//fprintf(file_handle, "calling for neighbors.\n");
			state=STATE_ADJ_SPOTS_TO_BE_FILLED;
			call_for_neighbors();
		}else{
			//fprintf(file_handle, "that's it.\n\tmy_type: %hhx, my_val: %hhu, min_master_val: %hhu, slave_check: %hhx\n",my_type, my_type_value, smallest_master_value, check_if_slave(my_type, NULL));
		}
		//fflush(file_handle);
	}
}

void DropletCustomEight::adj_spots_to_fill(){
	while(check_for_new_messages()){
		//fprintf(file_handle,"In adjacent spots to fill..\n");
		//fflush(file_handle);
		if((global_rx_buffer.buf[0]==NOT_IN_ASSEMBLY_INDICATOR_BYTE)&&((global_rx_buffer.data_len-2 /*JOHN: data_len has a bug?*/)==4)){
			droplet_id_type ack_tgt = *((droplet_id_type*)(global_rx_buffer.buf+1));
			////fprintf(file_handle,"\tack_tgt: %02hx\n", ack_tgt);
			////fflush(file_handle);
			uint8_t dir = global_rx_buffer.buf[3];
			if(ack_tgt==get_droplet_id()){
				float rOther, thetaOther, phiOther;
				range_and_bearing(global_rx_buffer.sender_ID, &rOther, &thetaOther, &phiOther);

				float desiredR = 2.*DROPLET_RADIUS+FORMATION_GAP;
				if(ANGLED_DIR&(1<<dir)) desiredR*=M_SQRT2; //the angled sides should be farther away.
				float desiredTh=getAngleFromDirMask(1<<dir);
				float dist = get_distance(rOther, thetaOther, desiredR, desiredTh);

				if(dist<=2.0*PROXIMITY_THRESHOLD){
					//close enough! send confirmation message.
					char msg[6];
					msg[0] = IN_ASSEMBLY_INDICATOR_BYTE;
					*((droplet_id_type*)(msg+1)) = global_rx_buffer.sender_ID;
					get_neighbor_type(my_type, my_type_value, (1<<dir), (uint16_t*)(msg+3), (int8_t*)(msg+5));
					ir_broadcast(msg, 6);
					my_filled_spots |= (1<<dir); //mark that spot as done.
					////fprintf(file_handle, "Just confirmed a bot in dir: %hhx. filled_spots now: %02hhx.\r\n", dir, my_filled_spots);
					////fflush(file_handle);
				}else{
					////fprintf(file_handle, "Not close enough. Dist: %f\n", dist);
					////fflush(file_handle);
					//Not close enough. Need to do something?
				}
			}
		}
		global_rx_buffer.read=1;
	}
	if((get_spots_from_type(my_type)-my_filled_spots)==0){
		////fprintf(file_handle,"ALL NEIGHBORS FULL.\n");
		//fflush(file_handle);
		state = STATE_ALL_ADJ_SPOTS_FILLED;
		char msg[1];
		msg[0] = GO_INDICATOR_BYTE;
		ir_broadcast(msg, 1);
	}else{
		if(check_timer(NEIGHBOR_CALL_TIMEOUT_TIMER)){
			call_for_neighbors();
		}
	}
}

void DropletCustomEight::handle_move_to_spot(){
	if(is_moving()){
		//already moving, great.
		float move_target_dist, move_target_theta, move_target_phi;
		range_and_bearing(move_target, &move_target_dist, &move_target_theta, &move_target_phi);
		float adj_move_target_dist, adj_move_target_theta;
		get_relative_neighbor_position(1<<move_target_dir, move_target_dist, move_target_theta, move_target_phi, &adj_move_target_dist, &adj_move_target_theta);
		if(check_timer(MOVE_TIMEOUT_TIMER)){
			//call_for_neighbors();
			reset_before_waiting_for_msgs();
			state = STATE_WAITING_FOR_MSGS;
			cancel_move();
			cancel_rotate();
			move_steps(get_best_move_dir(adj_move_target_theta+180), 2000);
		}
	}else{
		float move_target_dist, move_target_theta, move_target_phi;
		range_and_bearing(move_target, &move_target_dist, &move_target_theta, &move_target_phi);
		float adj_move_target_dist, adj_move_target_theta;
		get_relative_neighbor_position(1<<move_target_dir, move_target_dist, move_target_theta, move_target_phi, &adj_move_target_dist, &adj_move_target_theta);
		if(check_timer(MOVE_TIMEOUT_TIMER)){
			//call_for_neighbors();
			reset_before_waiting_for_msgs();
			state = STATE_WAITING_FOR_MSGS;
			move_steps(get_best_move_dir(adj_move_target_theta+180), 2000);
		}
		if(moving_state==MOVING_NORMAL){
			float this_move_dist=sub_polar_vec_mag(adj_move_target_dist, adj_move_target_theta, last_move_r, last_move_theta);
			//fprintf(file_handle, "this_move_dist: %f\n", this_move_dist);
			//fflush(file_handle);
			if(adj_move_target_dist<(PROXIMITY_THRESHOLD)){ //we're done moving!
				////fprintf(file_handle, "\tDone moving!\n");
				////fflush(file_handle);
				//cancel_move();
				//cancel_rotate();
				state = STATE_ADJUSTING_PHI;
				last_move_r=0;
				last_move_dist=0;
			}else if((fabs(last_move_r- adj_move_target_dist))<STUCK_DIST_THRESHOLD){ //we started slowing down
				moving_state = MOVING_BACKING_UP;
				avoid_target = adj_move_target_dist;
				rotate_steps(TURN_COUNTERCLOCKWISE, ROTATE_STEPS_AMOUNT);
				last_move_r=0;
				last_move_dist=0;
				set_timer(BACK_UP_TIMER_DELAY_MS, BACK_UP_TIMER);
			}else if(adj_move_target_dist<DROPLET_RADIUS){ //we're almost done moving. Take smaller steps.
				move_steps(get_best_move_dir(adj_move_target_theta), 5);
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
		}else if(moving_state==MOVING_BACKING_UP){
			if(adj_move_target_dist>=(1.5*avoid_target) || check_timer(BACK_UP_TIMER)){
				
				moving_state = MOVING_SIDESTEPPING;
				side_step_angle = 60.0*(((rand_byte()&0x1) ? -1.0 : 1.0)*((rand_byte()&0x1) ? 2.0 : 1.0));

				uint32_t backup_duration; 
				//fprintf(file_handle, "About to call get_timer_time_remaining.\n");
				//fflush(file_handle);
				if(!check_timer(BACK_UP_TIMER)) backup_duration = BACK_UP_TIMER_DELAY_MS - get_timer_time_remaining(BACK_UP_TIMER);
				//backup_duration = backup_duration/2;
				if(backup_duration>SIDESTEPPING_DELAY_MS) backup_duration = SIDESTEPPING_DELAY_MS;
				//fprintf(file_handle, "Called it.\n");
				//fflush(file_handle);
				set_timer(backup_duration, SIDESTEP_TIMER);
			}else{
				move_steps(get_best_move_dir(adj_move_target_theta+180), MOVE_STEPS_AMOUNT);
			}
		}else if(moving_state==MOVING_SIDESTEPPING){
			if(check_timer(SIDESTEP_TIMER)){
				moving_state=MOVING_NORMAL;
			}else{
				move_steps(get_best_move_dir(adj_move_target_theta+side_step_angle), MOVE_STEPS_AMOUNT);
				//move_steps(get_best_move_dir(adj_move_target_theta+90), MOVE_STEPS_AMOUNT);
			}
		}
		while(check_for_new_messages()){
			if((global_rx_buffer.data_len-2 /*JOHN: data_len has a bug?*/)==sizeof(claimMsg)){
				//got a message from someone claiming a spot.
				claimMsg* msg = (claimMsg*)global_rx_buffer.buf;
				if(msg->type == CLAIM_MSG_TYPE){
					if((move_target==msg->parent_id)&&(move_target_dir==msg->dir)){
						//someone has already claimed the spot I was going to! I should just stop moving.
						//(Something went wrong and I didn't hear when someone else was claiming that spot.)
						reset_before_waiting_for_msgs();
						state = STATE_WAITING_FOR_MSGS;
						move_steps(get_best_move_dir(adj_move_target_theta+180), 2000);
					
					}
				}
			}
			global_rx_buffer.read=1;
		}
	}
}

void DropletCustomEight::handle_adjusting_phi(){
	if(is_moving()){
		//Do nothing.
	}else{
		float r, theta, phi;
		range_and_bearing(move_target, &r, &theta, &phi);
		float delta_phi = phi; //this is how much we need to change our phi by.
		delta_phi = quick_and_dirty_mod(delta_phi);
		if(fabs(delta_phi)<ORIENT_THRESHOLD_DEG){ //Are we done?
			state = STATE_AWAITING_CONFIRMATION;
		}else{
			if(delta_phi<0){
				rotate_steps(TURN_CLOCKWISE, ROTATE_STEPS_AMOUNT);
			}else{
				rotate_steps(TURN_COUNTERCLOCKWISE, ROTATE_STEPS_AMOUNT);
			}
		}
	}
}

void DropletCustomEight::waiting_for_message(){

	if((!check_timer(DECIDING_DELAY_TIMER)) || recruiting_robots.empty()){
		while(check_for_new_messages()){
			set_timer(RANDOM_WALK_DELAY_MS, RANDOM_WALK_TIMER); //if we got a message, reset timer for random_walk.
			if((((claimMsg*)global_rx_buffer.buf)->type==CLAIM_MSG_TYPE)||(global_rx_buffer.buf[0] == IN_ASSEMBLY_INDICATOR_BYTE)){
				heard_the_assembly=true;
			}
			if(((global_rx_buffer.data_len-2 /*JOHN: data_len has a bug?*/)==2) && (global_rx_buffer.buf[0] == IN_ASSEMBLY_INDICATOR_BYTE)){ //the talking bot is in the assembly. We want to be in the assembly. Lets listen! 
				
				//We add that bot to our list of recruiting droplets and wait TO_DECIDING_COUNTDOWN passes for other potential recruiters to make themselves known.
				uint8_t desired_dirs = global_rx_buffer.buf[1];
				droplet_id_type target_droplet = global_rx_buffer.sender_ID;
				float r, theta, phi;
				range_and_bearing(target_droplet, &r, &theta, &phi);

				recruitingRobot* target = new recruitingRobot;
				target->desiredNeighbors = desired_dirs;
				target->range = r;
				target->bearing = theta;
				target->heading = phi;
				if(recruiting_robots.empty()){
					set_timer(DELAY_BEFORE_DECIDING_MS,DECIDING_DELAY_TIMER);
				}
				recruiting_robots[target_droplet] = target;
			}else{ //The talking bot is not in the assembly. Who cares?
				//do nothing.
			}
			if(!(heard_the_assembly||is_moving())){ //if we haven't gotten a call for neighbors.
				float r, theta, phi;
				range_and_bearing(global_rx_buffer.sender_ID, &r, &theta, &phi);
				handle_rotate_to_straight(theta);
				move_steps(get_best_move_dir(theta), MOVE_STEPS_AMOUNT);
			}
			global_rx_buffer.read = 1;
		}
		if(check_timer(RANDOM_WALK_TIMER)){
			if (!is_moving()) move_steps(((rand_byte()%5+1)), MOVE_STEPS_AMOUNT);	
		}
	}else{
		cancel_move(); //in case we were moving from backing up after our spot was claimed.
		state=STATE_DECIDING_SHOULD_MOVE;
	}
}

//bool DropletCustomNine::check_if_slave(uint16_t slave_q, uint16_t master_q){
//	//note that, since NULL==TYPE__, nothing should ever be slave to TYPE__ (the null type).
//	if(master_q==NULL){ //then the function should return if slave_q type is slave to ANY type.
//		if(slave_q==TYPE_W||slave_q==TYPE_E) return true;
//	}else{ //the function should return if slave_q type is slave to master_q type
//		if((slave_q==TYPE_W||slave_q==TYPE_E)&&(master_q==TYPE_N||master_q==TYPE_S)) return true;
//	}
//	return false;
//}
//
//void DropletCustomNine::get_neighbor_type(uint16_t type, int8_t value, uint8_t dir, uint16_t* neighbor_type, int8_t* neighbor_value){
//	switch(type){
//		case TYPE_SEED:
//			switch(dir){
//				case DIR_MASK_N: *neighbor_type=TYPE_N; *neighbor_value=value-1; break;
//				case DIR_MASK_E: *neighbor_type=TYPE_E; *neighbor_value=value-1; break;
//				case DIR_MASK_W: *neighbor_type=TYPE_W; *neighbor_value=value-1; break;
//				case DIR_MASK_S: *neighbor_type=TYPE_S; *neighbor_value=value-1; break;
//			}
//			break;
//		case TYPE_N:
//			switch(dir){
//				case DIR_MASK_N: *neighbor_type=TYPE_N; *neighbor_value=value-1; break;
//				case DIR_MASK_E: *neighbor_type=TYPE_E; *neighbor_value=value-1; break;
//				case DIR_MASK_W: *neighbor_type=TYPE_W; *neighbor_value=value-1; break;
//			}
//			break;
//		case TYPE_S:
//			switch(dir){
//				case DIR_MASK_S: *neighbor_type=TYPE_S; *neighbor_value=value-1; break;
//				case DIR_MASK_E: *neighbor_type=TYPE_E; *neighbor_value=value-1; break;
//				case DIR_MASK_W: *neighbor_type=TYPE_W; *neighbor_value=value-1; break;
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
//	}
//	//fprintf(file_handle, "\n\n\nERROR UNEXPECTED TYPE/DIR COMBO T:%hhx D:%hhx\n\n\n", type, dir);
//	//fflush(file_handle);
//}
//
//uint8_t DropletCustomNine::get_spots_from_type(uint16_t type){
//	if(type==TYPE_N){
//		return (DIR_MASK_N | DIR_MASK_W | DIR_MASK_E);
//	}else if(type==TYPE_S){
//		return (DIR_MASK_S | DIR_MASK_W | DIR_MASK_E);
//	}else if(type==TYPE_E){
//		return DIR_MASK_E;
//	}else if(type==TYPE_W){
//		return DIR_MASK_W;
//	}else if(type==TYPE_SEED){
//		return (DIR_MASK_N | DIR_MASK_E | DIR_MASK_S | DIR_MASK_W);
//	}else if(type==TYPE__){
//		return 0;
//	}else{
//		//fprintf(file_handle, "\n\n\nERROR IN SPOTS FROM TYPE; UNEXPECTED TYPE: %hhx\n\n\n", type);
//		//fflush(file_handle);
//	}
//}

bool DropletCustomEight::check_if_slave(uint16_t slave_q, uint16_t master_q){
	//note that, since NULL==TYPE__, nothing should ever be slave to TYPE__ (the null type).
	//if(master_q==NULL){ //then the function should return if slave_q type is slave to ANY type.
	//	if(slave_q&SLAVE_TYPES) return true;
	//}else{ //the function should return if slave_q type is slave to master_q type
	//	if((slave_q&SLAVE_TYPES)&&(master_q&MASTER_TYPES)) return true;
	//}
	return false;
}

void DropletCustomEight::get_neighbor_type(uint16_t type, int8_t value, uint8_t dir, uint16_t* neighbor_type, int8_t* neighbor_value){
	switch(type){
		case TYPE_SEED:
			switch(dir){
				case DIR_MASK_N:	*neighbor_type=TYPE_N;	*neighbor_value=value-1; break;
				case DIR_MASK_E:	*neighbor_type=TYPE_E;	*neighbor_value=value-1; break;
				case DIR_MASK_W:	*neighbor_type=TYPE_W;	*neighbor_value=value-1; break;
				case DIR_MASK_S:	*neighbor_type=TYPE_S;	*neighbor_value=value-1; break;
				case DIR_MASK_NE:	*neighbor_type=TYPE_NE;	*neighbor_value=value-1; break;
				case DIR_MASK_NW:	*neighbor_type=TYPE_NW;	*neighbor_value=value-1; break;
				case DIR_MASK_SE:	*neighbor_type=TYPE_SE;	*neighbor_value=value-1; break;
				case DIR_MASK_SW:	*neighbor_type=TYPE_SW;	*neighbor_value=value-1; break; 
			}
			break;
		case TYPE_N:
			switch(dir){
				case DIR_MASK_N: *neighbor_type=TYPE_N; *neighbor_value=value-1; break;
			}
			break;
		case TYPE_S:
			switch(dir){
				case DIR_MASK_S: *neighbor_type=TYPE_S; *neighbor_value=value-1; break;
			}
			break;
		case TYPE_E:
			switch(dir){
				case DIR_MASK_E: *neighbor_type=TYPE_E; *neighbor_value=value-1; break;
			}
			break;
		case TYPE_W:
			switch(dir){
				case DIR_MASK_W: *neighbor_type=TYPE_W; *neighbor_value=value-1; break;
			}
			break;
		case TYPE_NE:
			switch(dir){ 
				case DIR_MASK_N:  *neighbor_type=TYPE_N; *neighbor_value=value-1; break;
				case DIR_MASK_NE: *neighbor_type=TYPE_NE; *neighbor_value=value-1; break;
				case DIR_MASK_E:  *neighbor_type=TYPE_E; *neighbor_value=value-1; break;
			}
			break;
		case TYPE_NW:
			switch(dir){
				case DIR_MASK_N:  *neighbor_type=TYPE_N; *neighbor_value=value-1; break;
				case DIR_MASK_NW: *neighbor_type=TYPE_NW; *neighbor_value=value-1; break;
				case DIR_MASK_W:  *neighbor_type=TYPE_W; *neighbor_value=value-1; break;
			}
			break;
		case TYPE_SE:
			switch(dir){
				case DIR_MASK_S:  *neighbor_type=TYPE_S; *neighbor_value=value-1; break;
				case DIR_MASK_SE: *neighbor_type=TYPE_SE; *neighbor_value=value-1; break;
				case DIR_MASK_E:  *neighbor_type=TYPE_E; *neighbor_value=value-1; break;
			}
			break;
		case TYPE_SW:
			switch(dir){
				case DIR_MASK_S:  *neighbor_type=TYPE_S; *neighbor_value=value-1; break;
				case DIR_MASK_SW: *neighbor_type=TYPE_SW; *neighbor_value=value-1; break;
				case DIR_MASK_W:  *neighbor_type=TYPE_W; *neighbor_value=value-1; break;
			}
			break;
	}
	//fprintf(file_handle, "\n\n\nERROR UNEXPECTED TYPE/DIR COMBO T:%hhx D:%hhx\n\n\n", type, dir);
	//fflush(file_handle);
}

uint8_t DropletCustomEight::get_spots_from_type(uint16_t type){
	switch(type){
		case TYPE_N:	return DIR_MASK_N;
		case TYPE_E:	return DIR_MASK_E;
		case TYPE_W:	return DIR_MASK_W;
		case TYPE_S:	return DIR_MASK_S;
		case TYPE_NE:	return (DIR_MASK_N | DIR_MASK_NE | DIR_MASK_E);
		case TYPE_NW:	return (DIR_MASK_N | DIR_MASK_NW | DIR_MASK_W);
		case TYPE_SE:	return (DIR_MASK_S | DIR_MASK_SE | DIR_MASK_E);
		case TYPE_SW:	return (DIR_MASK_S | DIR_MASK_SW | DIR_MASK_W);
		case TYPE_SEED: return 0xFF; //all dirs
	}	
}

void DropletCustomEight::reset_before_waiting_for_msgs(){
	set_timer(RANDOM_WALK_DELAY_MS, RANDOM_WALK_TIMER);
	move_target = NULL;
	move_target_dir = 0;
	closestDir=0;
	closestID=0;
	closestDist=0;
	recruiting_robots.clear();
}

void DropletCustomEight::handle_rotate_to_straight(float theta){
	if((-30<=theta)&&(theta<30)){
		//do nothing
	}else if((-90<=theta)&&(theta<-30)){
		theta+=60.0;
	}else if((-150<=theta)&&(theta<-90)){
		theta+=120.0;
	}else if(((-180<=theta)&&(theta<-150))||((150<=theta)&&(theta<=180))){
		if(theta<0) theta+=180.0;
		else theta-=180.0;
	}else if((90<=theta)&&(theta<150)){
		theta-=120;
	}else if((30<=theta)&&(theta<90)){
		theta-=60;
	}
	if(theta<(ORIENT_THRESHOLD_DEG/2.)) rotate_steps(TURN_CLOCKWISE, ROTATE_STEPS_AMOUNT);
	else if(theta>(ORIENT_THRESHOLD_DEG/2.)) rotate_steps(TURN_COUNTERCLOCKWISE, ROTATE_STEPS_AMOUNT);
}


void DropletCustomEight::call_for_neighbors(){
	//fprintf(file_handle, "\tCalling for neighbors.\n");
	//fflush(file_handle);
	char msg[2];
	msg[0] = IN_ASSEMBLY_INDICATOR_BYTE;
	msg[1] = get_spots_from_type(my_type)-my_filled_spots;
	ir_broadcast(msg, 2);
	set_timer(NEIGHBOR_CALL_TIMEOUT_TIMER_DELAY_MS, NEIGHBOR_CALL_TIMEOUT_TIMER);
}

bool DropletCustomEight::check_if_stuck(float delta, float last_delta){
	if(last_delta<1){ //no bot moves that fast; this checks for the weird cases at the very start of motion.
		if(last_delta*0.5>delta){ //we slowed down by more than 50%
			return true;
		}else if(delta<STUCK_DIST_THRESHOLD){ //we're barely moving.
			return true;
		}
	}
	return false;
}

void DropletCustomEight::maintain_position(droplet_id_type bot, uint8_t dir){
	if(is_moving()){
		//already moving, great.
	}else{
		float bot_r, bot_theta, bot_phi;
		range_and_bearing(bot, &bot_r, &bot_theta, &bot_phi);
		float goal_r, goal_theta;
		get_relative_neighbor_position((1<<dir), bot_r, bot_theta, bot_phi, &goal_r, &goal_theta);
		if(goal_r<(PROXIMITY_THRESHOLD/5.0)){
			//We're close enough! Keep it up. Check our rotation.
			float delta_phi = bot_phi/* + getAngleFromDirMask((1<<dir))*/; //this is how much we need to change our phi by.
			delta_phi = quick_and_dirty_mod(delta_phi);
			if(fabs(delta_phi)<ORIENT_THRESHOLD_DEG){
				//Rotation good too! Nice.
			}else{
				if(delta_phi<0) rotate_steps(TURN_CLOCKWISE, ROTATE_STEPS_AMOUNT);
				else rotate_steps(TURN_COUNTERCLOCKWISE, ROTATE_STEPS_AMOUNT);
			}
		}else{
			move_steps(get_best_move_dir(goal_theta), 2);
		}
	}
}

void DropletCustomEight::broadcast_favorite_target(){
	favTgtMsg* msg = new favTgtMsg;
	msg->type = NOT_IN_ASSEMBLY_INDICATOR_BYTE;
	msg->id = closestID;
	msg->dir = closestDir;
	msg->dist = closestDist;
	ir_broadcast((char*)msg, sizeof(favTgtMsg));
}

void DropletCustomEight::broadcast_claim_msg(droplet_id_type parent, uint8_t dir){
	claimMsg* msg = new claimMsg;
	msg->type = CLAIM_MSG_TYPE;
	msg->parent_id = parent;
	msg->dir = dir;
	ir_broadcast((char*)msg, sizeof(claimMsg));
}

float DropletCustomEight::get_distance(float rA, float thetaA, float rB, float thetaB){
	float xA = rA*cos(deg2rad(thetaA));
	float yA = rA*sin(deg2rad(thetaA));
	float xB = rB*cos(deg2rad(thetaB));
	float yB = rB*sin(deg2rad(thetaB));
	float dist = sqrt((xA-xB)*(xA-xB)+(yA-yB)*(yA-yB));
	return dist;
}

move_direction DropletCustomEight::get_best_move_dir(float theta){
	theta = quick_and_dirty_mod(theta);
	if((-30<=theta)&&(theta<30)){
		return 1;
	}else if((-90<=theta)&&(theta<-30)){
		return 2;
	}else if((-150<=theta)&&(theta<-90)){
		return 3;
	}else if(((-180<=theta)&&(theta<-150))||((150<=theta)&&(theta<=180))){
		return 4;
	}else if((90<=theta)&&(theta<150)){
		return 5;
	}else if((30<=theta)&&(theta<90)){
		return 6;
	}
	//fprintf(file_handle,"\n\n\n\n\nERROR ERROR UNEXPECTED DIRECTION FOR THETA: %f ERROR ERROR \n\n\n\n\n",theta);
	//fflush(file_handle);
	return 7;
}

void DropletCustomEight::calculate_distance_to_target_positions(){
	closestDist = BIG_NUMBER;
	closestDir = 0;
	closestID = 0;
	for(std::map<droplet_id_type, recruitingRobot*>::iterator iter=recruiting_robots.begin() ; iter!=recruiting_robots.end(); iter++){
		for(uint8_t dir=0; dir<8; dir++){
			if(iter->second->desiredNeighbors & (0x1<<dir)){
				get_relative_neighbor_position(0x1<<dir, iter->second->range, iter->second->bearing, iter->second->heading, &(iter->second->toNeighborDist[dir]), &(iter->second->toNeighborTheta[dir]));
			}else{
				iter->second->toNeighborDist[dir] = BIG_NUMBER;
			}
			if(iter->second->toNeighborDist[dir]<closestDist){
				closestDist = iter->second->toNeighborDist[dir];
				closestID = iter->first;
				closestDir=dir;
			}
		}
	}
	////fprintf(file_handle, "Closest: {ID: %04hx, DIR: %02hhx, DIST: %f}\n",closestID, closestDir, closestDist);
}

float DropletCustomEight::getAngleFromDirMask(uint8_t dir_mask){
	float angle=0;
	switch(dir_mask){
		case DIR_MASK_N:	angle=	0.0;	break;
		case DIR_MASK_NE:	angle= -45.0;	break;
		case DIR_MASK_E:	angle= -90.0;	break;
		case DIR_MASK_SE:	angle= -135.0;	break;
		case DIR_MASK_S:	angle= -180.0;	break;
		case DIR_MASK_SW:	angle=	135.0;	break;
		case DIR_MASK_W:	angle=	90.0;	break;
		case DIR_MASK_NW:	angle=	45.0;	break;
	}
	return angle;
}


void DropletCustomEight::get_relative_neighbor_position(uint8_t dir_mask, float neighbor_r, float neighbor_theta, float neighbor_phi, float* target_r, float* target_theta){
	float delta_r=DROPLET_RADIUS*2+FORMATION_GAP;
	if(ANGLED_DIR&dir_mask) delta_r*=M_SQRT2; //the angled sides should be farther away.
	float delta_theta=neighbor_phi + getAngleFromDirMask(dir_mask);
	add_polar_vec(neighbor_r, neighbor_theta, delta_r, delta_theta, target_r, target_theta);
}

void DropletCustomEight::add_polar_vec(float r1, float th1, float r2, float th2, float* rs, float* ths){
	float x1=r1*cos(deg2rad(th1));
	float y1=r1*sin(deg2rad(th1));
	float x2=r2*cos(deg2rad(th2));
	float y2=r2*sin(deg2rad(th2));
	float xs=x1+x2;
	float ys=y1+y2;
	*rs = sqrt(xs*xs+ys*ys);
	*ths = rad2deg(atan2(ys,xs));
}

float DropletCustomEight::sub_polar_vec_mag(float r1, float th1, float r2, float th2){
	float x1=r1*cos(deg2rad(th1));
	float y1=r1*sin(deg2rad(th1));
	float x2=r2*cos(deg2rad(th2));
	float y2=r2*sin(deg2rad(th2));
	float xs=x1-x2;
	float ys=y1-y2;
	return sqrt(xs*xs+ys*ys);
}

void DropletCustomEight::remove_dir_from_spots_map(uint8_t dir, droplet_id_type id){
	////fprintf(file_handle, "want neighb.: %hhx; ", recruiting_robots[id]->desiredNeighbors);
	recruiting_robots[id]->desiredNeighbors &= ~((1<<dir));
	////fprintf(file_handle, " removed %hhu; now want neighb: %hhx\n", dir, recruiting_robots[id]->desiredNeighbors);
	if((recruiting_robots[id]->desiredNeighbors)==0){ //if no neighbors left, remove this robot from our list.
		////fprintf(file_handle, "\tNo neighbors left!\n");
		delete recruiting_robots[id];
		recruiting_robots.erase(recruiting_robots.find(id));
	}
	//fflush(file_handle);
}

float DropletCustomEight::deg2rad(float deg){
	return (float)((deg*((float)M_PI))/180.0);
}

float DropletCustomEight::rad2deg(float rad){
	return (float)((rad*180.0)/((float)M_PI));
}

float DropletCustomEight::quick_and_dirty_mod(float theta){
	while(theta>180.0){
		theta=theta-360.0;
	}
	while(theta<-180.0){
		theta=theta+360.0;
	}
	return theta;
}
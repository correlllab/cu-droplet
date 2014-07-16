#include "DropletCustomEight.h"

DropletCustomEight::DropletCustomEight(ObjectPhysicsData *objPhysics) 
	: DSimDroplet(objPhysics) 
{ return; }

DropletCustomEight::~DropletCustomEight() { return; }

void DropletCustomEight::DropletInit()
{
	init_all_systems();
	//char buffer[64];
	//sprintf(buffer,"C:\\Users\\Colab\\Desktop\\dropletSimDumps\\%04hxdat.txt",get_droplet_id());
	//file_handle = fopen (buffer,"w");
	set_rgb_led(0,0,0);
	srand(time(NULL));
	my_spots_to_fill=0;
	printf("Droplet Initialized.\r\n");
	state = STATE_START;
	set_timer(START_DELAY_MS,START_DELAY_TIMER);
}

void DropletCustomEight::DropletMainLoop()
{
	if(state&STATE_START){
		handle_start_broadcast();
	}else if(state&STATE_IN_ASSEMBLY){
		set_rgb_led(0,0,250);
		if(state&STATE_AWAITING_CONFIRMATION){
			set_rgb_led(0,250,250);
			awaiting_confirmation();
		}else if(state&STATE_ADJ_SPOTS_TO_BE_FILLED){
			set_rgb_led(250,250,250);
			adj_spots_to_fill();
		}else{
			//no spots to fill.
			set_rgb_led(0,0,0);
		}
	}else{ //not in assembly
		if(state&STATE_MOVING_TO_SPOT){
			set_rgb_led(250,0,250);
			move();
		}else if(state&STATE_DECIDING_SHOULD_MOVE){
			set_rgb_led(250,250,0);
			if(check_timer(MOVING_DELAY_TIMER)){
				check_ready_to_move();
			}else{
				decide_if_should_move();
			}
		}else{ //essentially just waiting around for a message?
			set_rgb_led(250,0,0);
			waiting_for_message();
		}
	}
}

void DropletCustomEight::handle_start_broadcast(){
	if(check_timer(START_DELAY_TIMER)){
		//I'm the seed, hurray!
		my_spots_to_fill = 0xFF; //want all spots filled.
		state=STATE_ADJ_SPOTS_TO_BE_FILLED;
		char msg[2];
		msg[0] = IN_ASSEMBLY_INDICATOR_BYTE;
		msg[1] = my_spots_to_fill;
		ir_broadcast(msg, 2);
		//fclose(file_handle);
	}else{
		if(rand_byte()<64){
			char id_msg=START_INDICATOR_BYTE;
			ir_broadcast(&id_msg, 1);
		}
		while(check_for_new_messages()){
			//fprintf(file_handle,"got_msg:\n");
			//for(uint8_t i=0;i<global_rx_buffer.data_len;i++){
			//	fprintf(file_handle,"%02hhx ", global_rx_buffer.buf[i]);
			//}
			//fprintf(file_handle,"\n");
			if(((global_rx_buffer.data_len-2 /*JOHN: data_len has a bug?*/)==1)&&(global_rx_buffer.buf[0]==START_INDICATOR_BYTE)){

				//fprintf(file_handle,"gotID: %04hx\n",global_rx_buffer.sender_ID);
				if(global_rx_buffer.sender_ID<get_droplet_id()){
					//I'm not the smallest, so I may as well stop broadcasting.
					state=0;
					set_timer(0,START_DELAY_TIMER);
				}
			}
			global_rx_buffer.read=1;
		}
	}
}

void DropletCustomEight::check_ready_to_move(){
	if(recruiting_robots.find(closestID) == recruiting_robots.end()){ //our closest isn't in the dictionary anymore, so we need to pick a closest, broadcast, and reset countdown
		calculate_distance_to_target_positions();
		set_timer(DELAY_BEFORE_MOVING_MS,0);
	}else{
		state = STATE_MOVING_TO_SPOT;
		move_target = closestID;
		move_target_dir = closestDir;
	}
}

void DropletCustomEight::awaiting_confirmation(){
	while(check_for_new_messages()){
		if((global_rx_buffer.buf[0]==IN_ASSEMBLY_INDICATOR_BYTE)&&((global_rx_buffer.data_len-2 /*JOHN: data_len has a bug?*/)==3)){
			droplet_id_type ack_tgt = *((droplet_id_type*)(global_rx_buffer.buf+1));
			if(ack_tgt==get_droplet_id()){
				//this message is for me, and the other droplet has confirmed that I am in the right spot. this droplet will no longer move.
				//need to add code for this droplet to start recruiting more droplets.
				my_spots_to_fill = get_spots_from_pos(1<<move_target_dir);
				state=STATE_ADJ_SPOTS_TO_BE_FILLED;
				char msg[2];
				msg[0] = IN_ASSEMBLY_INDICATOR_BYTE;
				msg[1] = my_spots_to_fill;
				ir_broadcast(msg, 2);
			}
		}
		global_rx_buffer.read=1;
	}
}

void DropletCustomEight::adj_spots_to_fill(){
	while(check_for_new_messages()){
		if((global_rx_buffer.buf[0]==NOT_IN_ASSEMBLY_INDICATOR_BYTE)&&((global_rx_buffer.data_len-2 /*JOHN: data_len has a bug?*/)==4)){
			droplet_id_type ack_tgt = *((droplet_id_type*)(global_rx_buffer.buf+1));
			uint8_t dir = global_rx_buffer.buf[3];
			if(ack_tgt==get_droplet_id()){
				float *rOther=NULL, *thetaOther=NULL, *phiOther=NULL;
				range_and_bearing(global_rx_buffer.sender_ID,rOther,thetaOther,phiOther);

				float desiredR=2.*DROPLET_RADIUS;
				float desiredTh=getAngleFromDirMask(1<<dir);
				float dist = get_distance(*rOther, *thetaOther, desiredR, desiredTh);

				if(dist<=FORMATION_GAP){
					//close enough! send confirmation message.
					char msg[3];
					msg[0] = IN_ASSEMBLY_INDICATOR_BYTE;
					*((droplet_id_type*)(msg+1)) = global_rx_buffer.sender_ID;
					ir_broadcast(msg, 3);
				}else{
					//Not close enough. Need to do something?
				}
			}
		}
		global_rx_buffer.read=1;
	}
}

void DropletCustomEight::move(){
	if(is_moving()){
		//already moving, great.
	}else{
		float *move_target_dist=NULL, *move_target_theta=NULL, *move_target_phi=NULL;
		range_and_bearing(move_target, move_target_dist, move_target_theta, move_target_phi);
		float *adj_move_target_dist=NULL, *adj_move_target_theta=NULL;
		get_relative_neighbor_position(1<<move_target_dir, *move_target_dist, *move_target_theta, *move_target_phi, adj_move_target_dist, adj_move_target_theta);

		if(*adj_move_target_dist<PROXIMITY_THRESHOLD){ //we're done moving!
			state = STATE_AWAITING_CONFIRMATION;
			char msg[4];
			msg[0] = NOT_IN_ASSEMBLY_INDICATOR_BYTE;
			*((droplet_id_type*)(msg+1)) = move_target;
			msg[3] = move_target_dir;
			ir_broadcast(msg, 4);
		}else{
			move_steps(get_best_move_dir(*adj_move_target_theta), 30);
		}
	}
}

void DropletCustomEight::decide_if_should_move(){
	if(rand_byte()<64){
		broadcast_favorite_target();
	}
	while(check_for_new_messages()){
		if(((global_rx_buffer.data_len-2 /*JOHN: data_len has a bug?*/)==8) & (global_rx_buffer.buf[0] == NOT_IN_ASSEMBLY_INDICATOR_BYTE)){
			droplet_id_type targetID = *(global_rx_buffer.buf+1);
			uint8_t dir = *(global_rx_buffer.buf+3);
			float dist = *(global_rx_buffer.buf+4);
			std::map<droplet_id_type, recruitingRobot>::iterator iter;
			iter = recruiting_robots.find(targetID);
			if(iter == recruiting_robots.end()){
				//the key isn't in our map, so we can ignore it.
			}else{
				if((targetID==closestID)&&(iter->second.toNeighborDist[dir]<=dist)){
					//check for a tie.
					if(iter->second.toNeighborDist[dir]==dist && get_droplet_id()<global_rx_buffer.sender_ID){
						//tie, but my id is lower, so i get it.
					}else{
						//tie, there id is lower, so we need to remove the dir from our list.
						recruiting_robots[targetID].desiredNeighbors &= ~((1<<dir));
						if(!recruiting_robots[targetID].desiredNeighbors){ //if no neighbors left, remove this robot from our list.
							recruiting_robots.erase(recruiting_robots.find(targetID));
						}
					}
					//if their target is my target and I'm closer, do nothing.
				}else{ //else, remove the dir from our list.
					recruiting_robots[targetID].desiredNeighbors &= ~((1<<dir));
					if(!recruiting_robots[targetID].desiredNeighbors){ //if no neighbors left, remove this robot from our list.
						recruiting_robots.erase(recruiting_robots.find(targetID));
					}
				}
			}
		}
		global_rx_buffer.read = 1;
	}
}

void DropletCustomEight::waiting_for_message(){
	if(!check_timer(DECIDING_DELAY_TIMER) || recruiting_robots.empty()){
		while(check_for_new_messages()){
			if(((global_rx_buffer.data_len-2 /*JOHN: data_len has a bug?*/)==2) & (global_rx_buffer.buf[0] == IN_ASSEMBLY_INDICATOR_BYTE)){ //the talking bot is in the assembly. We want to be in the assembly. Lets listen! 
				//We add that bot to our list of recruiting droplets and wait TO_DECIDING_COUNTDOWN passes for other potential recruiters to make themselves known.
				uint8_t desired_dirs = global_rx_buffer.buf[1];
				droplet_id_type target_droplet = global_rx_buffer.sender_ID;
				float *r=NULL, *theta=NULL, *phi=NULL;
				range_and_bearing(target_droplet, r, theta, phi);
				recruitingRobot target;
				target.desiredNeighbors = desired_dirs;
				target.range = *r;
				target.bearing = *theta;
				target.heading = *phi;
				if(recruiting_robots.empty()){
					set_timer(DELAY_BEFORE_DECIDING_MS,DECIDING_DELAY_TIMER);
				}
				recruiting_robots[target_droplet] = target;
			}else{ //The talking bot is not in the assembly. Who cares?
				//do nothing.
			}
			global_rx_buffer.read = 1;
		}
	}else{
		state=STATE_DECIDING_SHOULD_MOVE;
	}
}

void DropletCustomEight::broadcast_favorite_target(){
	char msg[8];
	msg[0] = NOT_IN_ASSEMBLY_INDICATOR_BYTE;
	*((droplet_id_type*)(msg+1)) = closestID;
	msg[3] = closestDir;
				
	*((float*)(msg+4)) = closestDist;
	ir_broadcast(msg, 8);
}

float DropletCustomEight::get_distance(float rA, float thetaA, float rB, float thetaB){
	float xA = rA*cos(thetaA);
	float yA = rA*sin(thetaA);
	float xB = rB*cos(thetaB);
	float yB = rB*sin(thetaB);
	float dist = sqrt((xA-xB)*(xA-xB)+(yA-yB)*(yA-yB));
	return dist;
}

uint8_t DropletCustomEight::get_spots_from_pos(uint8_t dir_pos){
	switch(dir_pos){
		case DIR_MASK_N:	return DIR_MASK_N;
		case DIR_MASK_NE:	return DIR_MASK_NE|DIR_MASK_N|DIR_MASK_E;
		case DIR_MASK_E:	return DIR_MASK_E;
		case DIR_MASK_SE:	return DIR_MASK_SE|DIR_MASK_S|DIR_MASK_E;
		case DIR_MASK_S:	return DIR_MASK_S;
		case DIR_MASK_SW:	return DIR_MASK_SW|DIR_MASK_S|DIR_MASK_W;
		case DIR_MASK_W:	return DIR_MASK_W;
		case DIR_MASK_NW:	return DIR_MASK_NW|DIR_MASK_N|DIR_MASK_W;
		default:			return 0xFF;
	}
}

move_direction DropletCustomEight::get_best_move_dir(float theta){
	if(((-M_PI/6.)<theta)&&(theta<(M_PI/6.))){
		return 0;
	}else if(((M_PI/6.)<theta)&&(theta<(M_PI/2.))){
		return 1;
	}else if(((M_PI/2.)<theta)&&(theta<(5.*M_PI/6.))){
		return 2;
	}else if(((5.*M_PI/6.)<theta)&&(theta<(7.*M_PI/6.))){
		return 3;
	}else if(((7.*M_PI/6.)<theta)&&(theta<(3.*M_PI/2.))){
		return 4;
	}else if(((3.*M_PI/2.)<theta)&&(theta<(11.*M_PI/6.))){
		return 5;
	}else{
		//error. probably need to modulus theta first.
		return 7;
	}
}

void DropletCustomEight::calculate_distance_to_target_positions(){
	closestDist = std::numeric_limits<float>::infinity();
	closestDir = 0;
	closestID = 0;
	for(std::map<droplet_id_type, recruitingRobot>::iterator iter=recruiting_robots.begin() ; iter!=recruiting_robots.end(); iter++){
		for(uint8_t dir=0; dir<8; dir++){
			if(iter->second.desiredNeighbors & (0x1<<dir)){
				get_relative_neighbor_position(0x1<<dir, iter->second.range, iter->second.bearing, iter->second.heading, &(iter->second.toNeighborDist[dir]), &(iter->second.toNeighborTheta[dir]));
			}else{
				iter->second.toNeighborDist[dir] = std::numeric_limits<float>::infinity();
			}
			if(iter->second.toNeighborDist[dir]<closestDist){
				closestDist = iter->second.toNeighborDist[dir];
				closestID = iter->first;
				closestDir=dir;
			}
		}
	}
}

float DropletCustomEight::getAngleFromDirMask(uint8_t dir_mask){
	float angle=0;
	switch(dir_mask){
		case DIR_MASK_N:	angle+=0;						break;
		case DIR_MASK_NE:	angle-=(float)(M_PI_4);			break;
		case DIR_MASK_E:	angle-=(float)(M_PI_2);			break;
		case DIR_MASK_SE:	angle-=(float)(M_PI+M_PI_4);	break;
		case DIR_MASK_S:	angle-=(float)(M_PI);			break;
		case DIR_MASK_SW:	angle+=(float)(M_PI-M_PI_4);	break;
		case DIR_MASK_W:	angle+=(float)(M_PI_2);			break;
		case DIR_MASK_NW:	angle+=(float)(M_PI_4);			break;
	}
	return angle;
}


void DropletCustomEight::get_relative_neighbor_position(uint8_t dir_mask, float neighbor_r, float neighbor_theta, float neighbor_phi, float* target_r, float* target_theta){
	float delta_r=DROPLET_RADIUS*2+FORMATION_GAP;
	float delta_theta=neighbor_phi + getAngleFromDirMask(dir_mask);
	add_polar_vec(neighbor_r, neighbor_theta, delta_r, delta_theta, target_r, target_theta);
}

void DropletCustomEight::add_polar_vec(float r1, float th1, float r2, float th2, float* rs, float* ths){
	float x1=r1*cos(th1);
	float y1=r1*sin(th1);
	float x2=r2*cos(th2);
	float y2=r2*sin(th2);
	float xs=x1+x2;
	float ys=y1+y2;
	*rs = sqrt(xs*xs+ys*ys);
	*ths = atan2(ys,xs);
}

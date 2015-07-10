//#include "droplet_programs/ball_bounce.h"
//
///*
 //* Any code in this function will be run once, when the robot starts.
 //*/
//void init()
//{
	//for(uint8_t i=0;i<6;i++) neighbors[i].ID=0;
	//heartbeat_msg	= "<3";
	//num_neighbors = 0;
	//last_loop_time = get_time();
	//last_heartbeat_time = get_time();
	//last_ball_time = 0;
	//outgoing_ball->msg_flag ='b';
	//set_all_ir_powers(LOW_PRIORITY_IR_POWER);
	//change_state(NOT_BALL);
//}
//
///*
 //* The code in this function will be called repeatedly, as fast as it can execute.
 //*/
//void loop()
//{
	//if(state==NOT_BALL)
	//{
		//uint8_t rVal = num_neighbors*20;
		//uint8_t bVal = (6-num_neighbors)*20;
		//set_red_led(rVal);
		//set_blue_led(bVal);
	//}
	//else if(state==BALL)
	//{
		//if(((get_time()-last_ball_time)>BALL_PERIOD)&&num_neighbors)
		//{
			//throw_ball();
			//change_state(NOT_BALL);
		//}
	//}
	//if((get_time()-last_heartbeat_time) > HEARTBEAT_PERIOD)
	//{
		//last_heartbeat_time = get_time();
		//ir_send(ALL_DIRS, heartbeat_msg, 2);
	//}
	//num_neighbors = update_group(get_time()-last_loop_time);
	//last_loop_time = get_time();
	//if(get_time()-last_ball_time>5000)
	//{
		//int8_t g_val = get_green_sensor();
		//if(g_val>75)
		//{
			//change_state(BALL);
			//ball_in_dir = rand_byte()%6;
		//}
	//}
	//delay_ms(50);
//}
//
//void throw_ball()
//{	
	//uint8_t tgt_dir = check_bounce(ball_in_dir);
	//outgoing_ball->soft_tgt_id = neighbors[tgt_dir].ID;
	//set_all_ir_powers(HIGH_PRIORITY_IR_POWER);
	//uint32_t before = get_time();
	//wait_for_ir(ALL_DIRS);
	//ir_send(ALL_DIRS, (char*)outgoing_ball, sizeof(throw_ball_msg));
	//delay_ms(50);
	//wait_for_ir(ALL_DIRS);
	//ir_send(ALL_DIRS, (char*)outgoing_ball, sizeof(throw_ball_msg));
	//delay_ms(50);
	//wait_for_ir(ALL_DIRS);
	//ir_send(ALL_DIRS, (char*)outgoing_ball, sizeof(throw_ball_msg));
	//printf("Elapsed: %u\r\n", get_time()-before);
	//set_all_ir_powers(LOW_PRIORITY_IR_POWER);
//}
//
///*
 //* After each pass through loop(), the robot checks for all messages it has 
 //* received, and calls this function once for each message.
 //*/
//void handle_msg(ir_msg* msg_s)
//{
	////printf(msg_s->msg);
	////printf("\r\n");
	//throw_ball_msg* in_ball;
	//if(strcmp(msg_s->msg, heartbeat_msg)==0) add_group_member(msg_s->sender_ID, msg_s->dir_received);
	////else if(msg_s->length==1 && state==BALL && msg_s->msg[0] == 'a')
	////{
		////change_state(NOT_BALL);
	////}
	//else if(msg_s->length==sizeof(throw_ball_msg) && (in_ball=(throw_ball_msg*)(msg_s->msg))->msg_flag=='b') 
	//{	
		//if(state==NOT_BALL)
		//{
			//if(in_ball->soft_tgt_id==get_droplet_id())
			//{
				//ball_in_dir = msg_s->dir_received;
				//change_state(BALL);
			//}
		//}
		//else if(state==BALL)
		//{
			////if(in_ball->soft_tgt_id==get_droplet_id())
			////{
				////ball_in_dir = msg_s->dir_received;
			////}
			////else
			////{
				////change_state(NOT_BALL);
			////}
		//}
	//}
	//else
	//{
		//printf("Got unexpected message:\r\n\t");
		//printf(msg_s->msg);
		//printf("\r\n");
	//}
//}
//
//
////TODO: Go over this function to make sure it doesn't assume in_dir is valid anymore.
//uint8_t check_bounce(uint8_t in_dir)
//{
	////note: directions below are relative to the direction from which the ball came in.
	//uint8_t opp_dir			= (in_dir+3)%6;
	//uint8_t left_dir		= (in_dir+1)%6;
	//uint8_t right_dir		= (in_dir+5)%6; //it's like -1
	//uint8_t far_left_dir	= (in_dir+2)%6;
	//uint8_t far_right_dir	= (in_dir+4)%6; //it's like -2
	//
	//uint8_t in			= neighbors[in_dir].ID;	
	//uint8_t opp			= neighbors[opp_dir].ID;
	//uint8_t left		= neighbors[left_dir].ID;
	//uint8_t right		= neighbors[right_dir].ID;
	//uint8_t far_left	= neighbors[far_left_dir].ID;
	//uint8_t far_right	= neighbors[far_right_dir].ID;
	//
	//if(opp != 0)				return opp_dir;								//go straight.
	//else if(right!=0 && left!=0)											//both close neighbors exist
	//{
		//if((far_right!=0 && far_left!=0)||(far_right==0 && far_left==0))	//far neighbors symmetrical
								//return in_dir;	
		//else if(far_right!=0)	return right_dir;							//more neighbors on right
		//else					return left_dir;							//more neighbors on left
	//}
	//else if(right==0 && left==0)											//neither close neighbor exists
	//{
		//if((far_right!=0 && far_left!=0)||(far_right==0 && far_left==0))	//far neighbors symmetrical
								//return in_dir;			
		//else if(far_right!=0)	return far_right_dir;	
		//else					return far_left_dir;	
	//}
	//else if(right!=0)
	//{
		//if(far_right!=0)		return far_right_dir;	
		//else if(far_left!=0)	return far_left_dir;	
		//else					return right_dir;		
	//}
	//else									
	//{
		//if(far_left!=0)			return far_left_dir;	
		//else if(far_right!=0)	return far_right_dir;	
		//else					return left_dir;
	//}
//}
//
//uint8_t update_group(uint32_t time_to_add)
//{
	//uint8_t neighb_count = 0;
	////uint8_t colls = check_collisions();	
	//for(uint8_t i=0;i<6;i++)
	//{
		//if(neighbors[i].ID!=0)
		//{
			//neighbors[i].ms_age+=time_to_add;
			//if(neighbors[i].ms_age > GROUP_TIMEOUT)
				//neighbors[i].ID=0;	
			//else
				//neighb_count++;
		//}
	//}
	//return neighb_count;
//}
//
//// If the senderID is already in our group, this function resets its age to 0.
//// Otherwise, this function adds it to the group list.
//void add_group_member ( uint16_t senderID, uint8_t dir)
//{
	//if(neighbors[dir].ID!=0)
	//{
		//if(neighbors[dir].ID == senderID)
		//{
			//neighbors[dir].ms_age = 0;
			//return;
		//}
	//} 
	//
	//neighbors[dir].ID = senderID;
	//neighbors[dir].ms_age = 0;
//}
//
//void change_state ( State new_state )
//{
	//state = new_state;
	//if(state==BALL)
	//{
		//set_rgb(250,250,250);
		//last_ball_time=get_time();
	//}
	//else
	//{
		//set_rgb(0,0,0);
	//}
//}
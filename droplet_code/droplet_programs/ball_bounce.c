#include "droplet_programs/ball_bounce.h"

void init(){
	for(uint8_t i=0;i<6;i++) neighbors[i].ID=0;	
	loopCount = 0;
	outwardDir = 0;
	lastGoodbye=0;	
	myRNBLoop = (uint16_t)((((float)rand_byte())/255.0)*(RNB_BC_PERIOD_MS/LOOP_PERIOD_MS));
	missedBroadcast = 0;
	myState=NOT_BALL;
	//enable_leg_status_interrupt();
}

void sendBallMsg(){
	BallBounceMsg msg;
	msg.flag = BALL_BOUNCE_FLAG;
	msg.id = outwardDirID;
	msg.seqPos = ballSeqPos;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(BallBounceMsg));
}

void sendBotDataMsg(){
	PosInfoMsg msg;
	msg.flag = BOT_DATA_FLAG;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearbyBotsData[i].id==0) continue;
		msg.bots[msg.numBots].id = nearbyBotsData[i].id;
		msg.bots[msg.numBots].rangeMM = nearbyBotsData[i].range/10;
		msg.bots[msg.numBots].bearingOver2 = nearbyBotsData[i].bearing/2;
		msg.bots[msg.numBots].headingOver2 = nearbyBotsData[i].heading/2;
		msg.numBots++;
		//TODO: Instead of just sending the first 6, send the 'best' 6. Though, need to define best.
		if(msg.numBots>=NUM_SENT_BOTS) break; 
	}
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(PosInfoMsg));
}

void loop(){
	if((get_time()%LOOP_PERIOD_MS)<(LOOP_PERIOD_MS/10)){
		if(myState==BALL){
			switch(ballSeqPos){
				case 0: set_rgb(50,50,50);		break;
				case 1: set_rgb(200,200,200);	break;
				case 2: set_rgb(50,50,50);		break;
				default: 
					printf("Error! Unexpected seqPos.\r\n");
			}
			sendBallMsg();
			ballSeqPos++;
			if(ballSeqPos>2){
				myState=NOT_BALL;
				ballSeqPos=0;
			}
			if(loopCount==myRNBLoop){
				missedBroadcast=1;
			}	
		}else{
			set_rgb(0,0,0);
			if((loopCount==myRNBLoop)||missedBroadcast){
				delay_ms(LOOP_PERIOD_MS/2);
				broadcast_rnb_data();
				printf("T: %lu\r\n", get_time());
				for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++)
				{
					if(neighbors[dir].ID!=0)
					printf("%hu: %X (%f)\r\n", dir, neighbors[dir].ID, neighbors[dir].posError);
					else
					printf("%hu: -\r\n",dir);
				}
				printf("\r\n");
				missedBroadcast=0;
			}
		}

		loopCount=((loopCount+1)%LOOPS_PER_RNB);
		delay_ms(LOOP_PERIOD_MS/10);
	}
	if(rnb_updated){
		use_new_rnb();
	}
}

void removeNeighbor(uint8_t dir){
	neighbors[dir].ID=0;
	neighbors[dir].lastSeen=0;
	neighbors[dir].posError=0;
	neighbors[dir].noCollCount=0;
}

void use_new_rnb(){
	rnb_updated=0;	
	
	
}

void calculateOutboundDir(uint16_t inID){
	outwardDirID=0;	
	for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
		if(neighbors[dir].ID==inID){
			outwardDir = check_bounce(dir);
			outwardDirID = neighbors[outwardDir].ID;
			break;
		}
	}
	if(outwardDirID==0){
		printf("Got ball from a non-neighbor? But we must continue!\r\n");
		uint8_t randDir = rand_byte()%NEIGHBORHOOD_SIZE;
		uint8_t i;
		for(i=0;i<NEIGHBORHOOD_SIZE;i++){
			if(neighbors[randDir+i].ID!=0) break;
		}
		outwardDir = check_bounce(randDir+i);
		outwardDirID = neighbors[outwardDir].ID;
	}
}

void handle_msg(ir_msg* msg_struct){
	if(((BallBounceMsg*)(msg_struct->msg))->flag==BALL_BOUNCE_FLAG){
		BallBounceMsg msg = *((BallBounceMsg*)(msg_struct->msg));	
		if(msg.id==get_droplet_id()&&myState!=BALL){
			//printf("Got the ball!\r\n");
			myState=BALL;
			calculateOutboundDir(msg_struct->sender_ID);
			ballSeqPos = msg.seqPos;
		}else{
			//printf("Someone else getting a ball. Could maybe use this for lost ball handling later.\r\n");	
		}
	}
	//else if(msg_struct->msg[0]==GOODBYE_FLAG){
		//for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
			//if(neighbors[dir].ID==msg_struct->sender_ID){
				//neighbors[dir].ID=0;
				//neighbors[dir].angleDiff=0;
				//neighbors[dir].lastSeen=0;
				//break;
			//}
		//}
	//}
}

#if (NEIGHBORHOOD_SIZE==6)
//Code below assumes neighborhood size of 6.
uint8_t check_bounce(uint8_t in_dir){
	//note: directions below are relative to the direction from which the ball came in.
	uint8_t opp_dir			= (in_dir+3)%6;
	uint8_t left_dir		= (in_dir+1)%6;
	uint8_t right_dir		= (in_dir+5)%6; //it's like -1
	uint8_t far_left_dir	= (in_dir+2)%6;
	uint8_t far_right_dir	= (in_dir+4)%6; //it's like -2
	
	uint16_t in			= neighbors[in_dir].ID;
	uint16_t opp		= neighbors[opp_dir].ID;
	uint16_t left		= neighbors[left_dir].ID;
	uint16_t right		= neighbors[right_dir].ID;
	uint16_t far_left	= neighbors[far_left_dir].ID;
	uint16_t far_right	= neighbors[far_right_dir].ID;
	
	if(opp != 0)				return opp_dir;								//go straight.
	else if(right!=0 && left!=0){											//both close neighbors exist
		if((far_right!=0 && far_left!=0)||(far_right==0 && far_left==0))	//far neighbors symmetrical
								return in_dir;	
		else if(far_right!=0)	return right_dir;							//more neighbors on right
		else					return left_dir;							//more neighbors on left
	}else if(right==0 && left==0){											//neither close neighbor exists
		if((far_right!=0 && far_left!=0)||(far_right==0 && far_left==0))	//far neighbors symmetrical
								return in_dir;			
		else if(far_right!=0)	return far_right_dir;	
		else					return far_left_dir;	
	
	}else if(right!=0){
		if(far_right!=0)		return far_right_dir;	
		else if(far_left!=0)	return far_left_dir;	
		else					return right_dir;		
	}else{
		if(far_left!=0)			return far_left_dir;	
		else if(far_right!=0)	return far_right_dir;	
		else					return left_dir;
	}
}
#else if (NEIGHBORHOOD_SIZE==8)
uint8_t check_bounce(uint8_t inDir){
	//note: directions below are relative to the direction from which the ball came in.
	uint8_t inLeftDir		= (inDir+1)%8;
	uint8_t leftDir			= (inDir+2)%8;
	uint8_t oppLeftDir		= (inDir+3)%8;
	uint8_t oppDir			= (inDir+4)%8;	
	uint8_t oppRightDir		= (inDir+5)%8;
	uint8_t rightDir		= (inDir+6)%8;
	uint8_t inRightDir		= (inDir+7)%8;
	
	uint16_t in			= neighbors[inDir].ID;
	uint16_t inLeft		= neighbors[inLeftDir].ID;
	uint16_t left		= neighbors[leftDir].ID;
	uint16_t oppLeft	= neighbors[oppLeftDir].ID;
	uint16_t opp		= neighbors[oppDir].ID;
	uint16_t oppRight	= neighbors[oppRightDir].ID;
	uint16_t right		= neighbors[rightDir].ID;
	uint16_t inRight	= neighbors[inRightDir].ID;
	
	if(opp != 0)				return oppDir;								//go straight.
	else if( (oppRight==0) && (right==0)){
		if(left!=0)				return leftDir;
		else if(oppLeft!=0)		return oppLeftDir;
		else if(inLeft!=0)		return inLeftDir;
		else if(inRight!=0)		return inRightDir;
	}else if( (oppLeft==0) && (left==0)){
		if(right!=0)				return rightDir;
		else if(oppRight!=0)		return oppRightDir;
		else if(inRight!=0)			return inRightDir;
		else if(inLeft!=0)			return inLeftDir;
	}
	return inDir;
}
#endif

/*
 *	The function below is optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection.
 */
uint8_t user_handle_command(char* command_word, char* command_args)
{
	if(strcmp_P(command_word,PSTR("ball"))==0){
		uint8_t inDir=rand_byte()%NEIGHBORHOOD_SIZE;
		for(uint8_t i=0;i<6;i++){
			if(neighbors[inDir+i].ID!=0){
				inDir+=i;
				break;
			}
		}
		outwardDir = check_bounce(inDir);
		outwardDirID = neighbors[outwardDir].ID;
		//printf("InDir: %hu, outDir: %hu, outID: %X\r\n", inDir, outwardDir, outwardDirID);
		myState = BALL;	
		ballSeqPos = 0;
		return 1;
	}
	return 0;
}

///*
 //*	The function below is optional - if it is commented in, and the leg interrupts have been turned on
 //*	with enable_leg_status_interrupt(), this function will get called when that interrupt triggers.
 //*/	
//void	user_leg_status_interrupt()
//{
	//
//}
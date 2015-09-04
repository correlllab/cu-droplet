#include "droplet_programs/ball_bounce.h"

#define F(x) ((6-x)%6)

void init(){	
	printf("%hd %hd %hd %hd %hd %hd\r\n", F(0), F(-1),F(-2),F(3),F(2),F(1));
	delay_ms(1000);
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

float calculatePosError(float thT, float rT, float thP, float rP){
	float xT, yT, xP, yP, xDiff, yDiff, thDiff, rDiff;
	xT = rT*cos(thT);
	yT = rT*sin(thT);
	xP = rP*cos(thP);
	yP = rP*sin(thP);
	
	xDiff = xP-xT;
	yDiff = yP-yT;
	
	thDiff = atan2(yDiff, xDiff);
	rDiff  = hypot(xDiff, yDiff); 
	
	return rDiff;
}

void use_new_rnb(){
	rnb_updated=0;	
	int8_t bear = (int8_t)ceilf((3.0*last_good_rnb.bearing)/M_PI);
	uint8_t newDir = ((6-bear)%6);
	
	uint16_t newID = last_good_rnb.id_number;
	float posError, angleError;
	posError = calculatePosError(basis_angle[newDir], 2.0*DROPLET_RADIUS, last_good_rnb.bearing, last_good_rnb.range);
	angleError = fabsf(basis_angle[newDir]-last_good_rnb.bearing);
	//printf("%X %hu | R:%f\tBearing: %f\tError: %f, %f\r\n", last_good_rnb.id_number, newDir, last_good_rnb.range, rad_to_deg(last_good_rnb.bearing), posError, rad_to_deg(angleError));	
	
	if((posError>2.0)||(angleError>(M_PI/18.0))){
		for(uint8_t dir=0;dir<6;dir++){
			if(neighbors[dir].ID==newID){
				removeNeighbor(dir);
				break;
			}
		}
	}else{
		for(uint8_t dir=0;dir<NEIGHBORHOOD_SIZE;dir++){
			if(neighbors[dir].ID==newID){ //If this ID is already in the neighbors array.
				if(dir!=newDir){ //We already have this bot as a neighbor. can't be in two spots.
					uint8_t myAngleWorse = posError>neighbors[dir].posError;
					uint8_t otherSpotNotTimedOut = (get_time()-neighbors[dir].lastSeen)<=GROUP_TIMEOUT_MS;
					if(myAngleWorse&&otherSpotNotTimedOut) return; //Sticking with the old dir. 
					//otherwise, we're going to store this ID at the newID and so wipe the old.
					removeNeighbor(dir);
				}
				break; //Since we insure that each ID is only in here once, we can stop after we find it once.
			}	
		}
		if((neighbors[newDir].ID!=0)&&(neighbors[newDir].ID!=newID)){ //If another bot at this dir and not this one.
			uint8_t myAngleWorse = posError>neighbors[newDir].posError;
			uint8_t otherBotNotTimedOut = (get_time()-neighbors[newDir].lastSeen)<=GROUP_TIMEOUT_MS;
			if(myAngleWorse&&otherBotNotTimedOut) return;
		}
		neighbors[newDir].posError = posError;
		neighbors[newDir].ID = newID;
		neighbors[newDir].lastSeen = get_time();
		neighbors[newDir].noCollCount = 0;	
	}
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
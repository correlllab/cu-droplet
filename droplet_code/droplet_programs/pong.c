#include "pong.h"

/*
 *
 * TODO, AMONG OTHER THINGS->
 *    Figure out when I should update my state!
 *
 */

 #define MIN_X 0
 #define MIN_Y 0
 #define MAX_X 250
 #define MAX_Y 250

void init(){
	if((LOCALIZATION_DUR)>=SLOT_LENGTH_MS){
		printf_P(PSTR("Error! Localization requires SLOT_LENGTH_MS to be greater than LOCALIZATION_DUR!\r\n"));
	}
	loopID = 0xFFFF;
	frameCount = 0;
	frameStart=get_time();
	mySlot = getSlot(get_droplet_id());
	for(uint8_t i=0;i<NUM_TRACKED_BOTS+1;i++){
		cleanOtherBot(&nearBots[i]);
	}
	myState = STATE_PIXEL;
	colorMode = GAME;
	gameMode = BOUNCE;
	lastBallID = 0;
	lastBallMsg = 0;
	lastPaddleMsg = 0;
	hardBotsList = NULL;
	theBall.lastUpdate = 0;
	theBall.xPos = UNDF;
	theBall.yPos = UNDF;
	theBall.xVel = 0;
	theBall.yVel = 0;
	theBall.id = 0;
	theBall.radius = 0;
	printf("mySlot: %u, frame_length: %lu\r\n\r\n", mySlot, FRAME_LENGTH_MS);
	set_all_ir_powers(200);
}

void loop(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
	}
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID==mySlot){
			broadcast_rnb_data();
			schedule_task(RNB_DUR+20, sendBotPosMsg, NULL);
		}else if(loopID==SLOTS_PER_FRAME-1){
			qsort(nearBots, NUM_TRACKED_BOTS+1, sizeof(OtherBot), nearBotsBearingCmp);
			printf_P(PSTR("\nID: %04X T: %lu [ "), get_droplet_id(), get_time());
			switch(myState){
				case STATE_PIXEL:					printf("Pixel");		break;
				case (STATE_PIXEL|STATE_NORTH):		printf("North Pixel");	break;
				case (STATE_PIXEL|STATE_SOUTH):		printf("South Pixel");	break;
				default:							printf("???");			break;
			}
			printf_P(PSTR(" ]"));
			if(POS_DEFINED(&myPos)){
				printf_P(PSTR("\tMy Pos: {%d, %d, %d}\r\n"), myPos.x, myPos.y, myPos.o);
				printPosCovar(&myPosCovar);
				printf("\r\n");
			}else{
				printf("\r\n\r\n");
			}
			updateHardBots();
			if(BALL_VALID()){
				printf_P(PSTR("\tBall ID: %hu; radius: %hu; Pos: (%d, %d) @ vel (%hd, %hd)\r\n"), theBall.id, theBall.radius, theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel);
			}
		}
		updateBall();
		if(BALL_VALID() && myDist!=UNDF && myDist<=DROPLET_RADIUS){
			sendBallMsg();
		}
		updateColor();
	}
	if(NS_PIXEL_Q(myState)){
		checkLightLevel();
	}
	if(rnb_updated){
		RNB_DEBUG_PRINT("\t(RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", id, range, bearing, heading);
		useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		rnb_updated=0;
	}
	delay_ms(LOOP_DELAY_MS);
}

void updateColor(){
	uint8_t newR = 0, newG = 0, newB = 0;
	if(colorMode==POS){
		getPosColor(&newR, &newG, &newB);
	}else if(colorMode==GAME){
		if(POS_DEFINED(&myPos)){
			float coverage = getBallCoverage() + getPaddleCoverage();
			coverage = (coverage > 1.0) ? 1.0 : coverage;
			uint8_t intensityIncrease = 0;
			if(coverage>0.01){
				intensityIncrease = (uint16_t)(5.0*pow(51.0,coverage));
			}
			uint16_t newRed		= newR + intensityIncrease;
			uint16_t newGreen	= newG + intensityIncrease;
			uint16_t newBlue	= newB + intensityIncrease;
			newR = newRed>255 ? 255 :  newRed;
			newG = newGreen>255 ? 255 : newGreen;
			newB = newBlue>255 ? 255 : newBlue;
		}else{
			newB = 50;
		}
	}
	set_rgb(newR, newG, newB);
}

float getBallCoverage(){
	float ballCoveredRatio = 0.0;
	if((myDist!=(uint16_t)UNDF) && (myDist<(DROPLET_RADIUS+theBall.radius)) && (theBall.id!=0x0F)){
		if(theBall.radius<DROPLET_RADIUS){
			if(myDist>=(DROPLET_RADIUS-theBall.radius)){
				ballCoveredRatio = getCoverageRatioA(theBall.radius, myDist);
				}else{
				ballCoveredRatio = 1.0;
			}
			}else{
			if(myDist>=(theBall.radius-DROPLET_RADIUS)){
				ballCoveredRatio = getCoverageRatioB(theBall.radius, myDist);
				}else{
				ballCoveredRatio = 1.0;
			}
		}
		printf("Ball Coverage:\t%f | me: (% 4d, % 4d) ball: (% 4d, % 4d)->%hu\r\n", ballCoveredRatio, myPos.x, myPos.y, theBall.xPos, theBall.yPos, theBall.radius);
	}	
	return ballCoveredRatio;
}

float getPaddleCoverage(){
	float paddleCoveredRatio = 0.0;
	if(gameMode==PONG && NS_PIXEL_Q(myState)){
		int16_t myStart = myPos.x-DROPLET_RADIUS;
		int16_t myEnd   = myPos.x+DROPLET_RADIUS;
		if(myEnd>paddleStart && paddleEnd>myStart){ //otherwise, no intersection
			if(myEnd>paddleEnd){
				if(myStart>paddleStart){
					paddleCoveredRatio = (1.0*(paddleEnd-myStart))/(1.0*(myEnd-myStart));
				}else{
					paddleCoveredRatio = (1.0*(paddleEnd-paddleStart))/(1.0*(myEnd-myStart));
				}
			}else{
				if(myStart>paddleStart){
					paddleCoveredRatio = 1.0; //my end-myStart
				}else{
					paddleCoveredRatio = (1.0*(myEnd-paddleStart))/(1.0*(myEnd-myStart));
				}
			}
		}
	}
	return paddleCoveredRatio;
	
}

void updateBall(){
	uint32_t now = get_time();
	int32_t timePassed = now-theBall.lastUpdate;
	if(POS_DEFINED(&myPos) && BALL_VALID()){
		theBall.xPos += (int16_t)((((int32_t)(theBall.xVel))*timePassed)/1000.0);
		theBall.yPos += (int16_t)((((int32_t)(theBall.yVel))*timePassed)/1000.0);
		theBall.lastUpdate = now;
		BALL_DEBUG_PRINT("B[%hu]: %d, %d\r\n", theBall.id, theBall.xPos, theBall.yPos);
		uint8_t bounced = 0;
		HardBot* tmp = hardBotsList;
		myDist = (uint16_t)hypotf(myPos.x-theBall.xPos, myPos.y-theBall.yPos);
		while(tmp!=NULL){
			OtherBot* bot = getOtherBot(tmp->id);
			otherDist = (uint16_t)hypotf(bot->pos.x - theBall.xPos, bot->pos.y - theBall.yPos);
			if(myDist < otherDist){
				BALL_DEBUG_PRINT("\t%04X | ", tmp->id);
				if(checkBounceHard(bot->pos.x, bot->pos.y, timePassed)){
					if(gameMode==PONG && ((SOUTH_PIXEL_Q(myState) && theBall.yVel<=0) || (NORTH_PIXEL_Q(myState) && theBall.yVel>=0))){
						if(!isCovered){
							//Other Side scores a point!
							killBall();
							set_rgb(255,0,0);
						}
					}
					calculateBounce(bot->pos.x, bot->pos.y);
					BALL_DEBUG_PRINT("Ball bounced off boundary between me and %04X!\r\n", tmp->id);
					bounced = 1;
					break;
				}
			}
			tmp = tmp->next;
		}
		if(theBall.xPos<MIN_X || theBall.xPos>MAX_X || theBall.yPos<MIN_Y || theBall.yPos>MAX_Y){
			BALL_DEBUG_PRINT("Ball hit boundary, so we must have lost track.\r\n");
			theBall.xPos = UNDF;
			theBall.yPos = UNDF;
			myDist = UNDF;
			otherDist = UNDF;
		}
	}else{
		myDist = UNDF;
		otherDist = UNDF;
	}
}

void checkLightLevel(){
	int16_t r, g, b;
	get_rgb(&r,&g,&b);
	int16_t sum = r+g+b;
	uint32_t now = get_time();
	if(sum<=25){
		paddleChange += ((now-lastLightCheck)*(3*PADDLE_VEL));
		//printf("Paddle ChangeX5: %f\r\n", paddleChange);
	}else if(sum<=40){
		paddleChange += ((now-lastLightCheck)*(2*PADDLE_VEL));
		//printf("Paddle ChangeX3: %f\r\n", paddleChange);
	}else if(sum<=60){
		paddleChange += ((now-lastLightCheck)*(PADDLE_VEL));
		//printf("Paddle ChangeX5: %f\r\n", paddleChange);
	}
	if(sum<=60){
		isCovered=1;
	}else{
		isCovered=0;
	}
	lastLightCheck = now;
	//printf("Light: %5d (%4d, %4d, %4d)\r\n",sum,r,g,b);
}

void updateHardBots(){
	//First, making a copy of nearBots so we can sort it by a weird metric.
	OtherBot nearBotsCopy[NUM_TRACKED_BOTS];
	memcpy(nearBotsCopy, nearBots, sizeof(BotPos)*NUM_TRACKED_BOTS);
	uint8_t numNearBots = 0;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id==0) continue;
		nearBotsCopy[i].id   = nearBots[i].id;
		nearBotsCopy[i].pos.x    = nearBots[i].pos.x;
		nearBotsCopy[i].pos.y    = nearBots[i].pos.y;
		nearBotsCopy[i].pos.o    = nearBots[i].pos.o;
		numNearBots++;
	}
	cleanHardBots(); //clean out the previous hardBots list -- we start fresh each farme.
	//sort nearBots according to their bearing.
	//TODO:
	//	nearBotsCmpFunc sorts by RANGE! Need to change to one that sorts by bearing!
	//
	qsort(nearBotsCopy, numNearBots, sizeof(BotMeas), nearBotsBearingCmp);
	//go through each near bot by bearing and add it to hardBots if the gap in bearings is above 120 degrees
	for(uint8_t i=0;i<numNearBots;i++){
		if(nearBotsCopy[i].id==0) continue;
		uint8_t nextI = (i+1)%numNearBots;
		int16_t iBearing =  rad_to_deg(atan2(nearBotsCopy[i].pos.y - myPos.y, nearBotsCopy[i].pos.x - myPos.x));
		int16_t iNextBearing = rad_to_deg(atan2(nearBotsCopy[nextI].pos.y - myPos.y, nearBotsCopy[nextI].pos.x - myPos.x));
		uint8_t difference = abs((iBearing-iNextBearing + 540)%360 - 180) ;
		if(difference>120){
			addHardBot(nearBotsCopy[i].id);
			addHardBot(nearBotsCopy[nextI].id);
		}
	}
	//print out hard bots list.
	if(hardBotsList!=NULL){
		BALL_DEBUG_PRINT("Edges:");
		HardBot* tmp = hardBotsList;
		while(tmp!=NULL){
			BALL_DEBUG_PRINT("\t%04X", tmp->id);
			tmp = tmp->next;
		}
		BALL_DEBUG_PRINT("\r\n");
	}
}

void sendBallMsg(){
	BallMsg msg;
	msg.flag = BALL_MSG_FLAG;
	int16_t tempX = theBall.xPos;
	int16_t tempY = theBall.yPos;
	msg.xPos		= tempX&0xFF;
	msg.extraBits	= ((tempX & 0x0700)>>3)&0xE0;
	msg.yPos		= tempY&0xFF;
	msg.extraBits |= ((tempY & 0x0700)>>6)&0x1C;
	msg.extraBits |= theBall.id&0x03;
	msg.xVel = theBall.xVel;
	msg.yVel = theBall.yVel;
	msg.radius = (theBall.radius&0xFC) | ((theBall.id&0x0C)>>2);
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(BallMsg));
	lastBallMsg=get_time();
}

void handleBallMsg(BallMsg* msg, uint32_t arrivalTime){
	BALL_DEBUG_PRINT("Got Ball! T: %lu\r\n\tPos: (%5.1f, %5.1f)   Vel: (%hd, %hd) | lastUpdate: %lu\r\n", get_time(), theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel, theBall.lastUpdate);
	int16_t highX = (int16_t)(((int8_t)(msg->extraBits))>>5);
	int16_t highY = (int16_t)((((int8_t)(msg->extraBits))<<3)>>5);
	int16_t tempX = (int16_t)((highX<<8) | ((uint16_t)(msg->xPos)));
	int16_t tempY = (int16_t)((highY<<8) | ((uint16_t)(msg->yPos)));
	uint8_t id = ((msg->extraBits)&0x03) | (((msg->radius)&0x03)<<2);
	if(id == 0x0F && theBall.id!=0x0F){
		lastBallID = theBall.id;
		set_rgb(255,0,0);
		}else if(id == lastBallID && theBall.id==0x0F){
		return; //this is from someone who hasn't realized the ball is dead, yet.
	}
	theBall.xPos = tempX;
	theBall.yPos = tempY;
	theBall.id = id;
	theBall.xVel = msg->xVel;
	theBall.yVel = msg->yVel;
	theBall.radius = ((msg->radius)&0xFC);
	theBall.lastUpdate = arrivalTime-4;
	BALL_DEBUG_PRINT("\tPos: (%5.1f, %5.1f)   Vel: (%hd, %hd) | lastUpdate: %lu\r\n", theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel, theBall.lastUpdate);
}

void sendBotPosMsg(){
	if(POS_DEFINED(&myPos)){
		BotPosMsg msg;
		copyBotPos(&myPos, &(msg.pos));
		msg.flag = BOT_POS_MSG_FLAG;
		ir_send(ALL_DIRS, (char*)(&msg), sizeof(BotPosMsg));
	}
}

void handleBotPosMsg(BotPosMsg* msg, id_t senderID){
	printf("%04X @ {%4d, %4d, % 4d}\r\n", senderID, (msg->pos).x, (msg->pos).y, (msg->pos).o);
}

void sendPaddleMsg(){
	if(!CTRL_Q(myState)){
		return;
	}
	PaddleMsg msg;
	if(NE_Q(myState)){
		msg.flag = 'P';
		msg.deltaPos = ((int16_t)paddleChange);
	}else if(NW_Q(myState)){
		msg.flag = 'P';
		msg.deltaPos = -1*((int16_t)paddleChange);
	}else if(SE_Q(myState)){
		msg.flag = 'S';
		msg.deltaPos = ((int16_t)paddleChange);
	}else if(SW_Q(myState)){
		msg.flag = 'S';
		msg.deltaPos = -1*((int16_t)paddleChange);
	}else{
		paddleChange = 0.0;
		return;
	}
	paddleChange = 0.0;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(PaddleMsg));
}

void handlePaddleMsg(char flag, int16_t delta){
	if((NORTH_PIXEL_Q(myState) && flag=='P') || (SOUTH_PIXEL_Q(myState) && flag=='S')){
		paddleStart += delta;
		paddleEnd += delta;
		//if(paddleStart<MIN_DIM){
			//paddleStart = MIN_DIM;
			//paddleEnd = MIN_DIM+ PADDLE_WIDTH;
		//}
		//if(paddleEnd>MAX_X_DIM){
			//paddleEnd = MAX_X_DIM;
			//paddleStart = MAX_X_DIM-PADDLE_WIDTH;
		//}
	}
}

void handle_msg(ir_msg* msg_struct){
	if(((BallMsg*)(msg_struct->msg))->flag==BALL_MSG_FLAG && msg_struct->length==sizeof(BallMsg)){
		handleBallMsg((BallMsg*)(msg_struct->msg), msg_struct->arrival_time);
	}else if(((BotMeasMsg*)(msg_struct->msg))->flag==BOT_MEAS_MSG_FLAG && msg_struct->length==sizeof(BotMeasMsg)){
		handleBotMeasMsg((BotMeasMsg*)(msg_struct->msg), msg_struct->sender_ID);
	}else if(((BotPosMsg*)(msg_struct->msg))->flag==BOT_POS_MSG_FLAG && msg_struct->length==sizeof(BotPosMsg)){
		handleBotPosMsg((BotPosMsg*)(msg_struct->msg), msg_struct->sender_ID);
	}else{
		printf_P(PSTR("%hu byte msg from %04X:\r\n\t"), msg_struct->length, msg_struct->sender_ID);
		for(uint8_t i=0;i<msg_struct->length;i++){
			printf("%02hX ", msg_struct->msg[i]);
		}
		printf("\r\n");
	}
}

OtherBot* getOtherBot(id_t id){
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id==id){
			return &(nearBots[i]);
		}
	}
	return NULL;
}

void findAndRemoveOtherBot(id_t id){
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id==id){
			cleanOtherBot(&nearBots[i]);
			break;
		}
	}
}

void removeOtherBot(uint8_t idx){
	cleanOtherBot(&nearBots[idx]);
}

OtherBot* addOtherBot(id_t id){
	uint8_t emptyIdx=0xFF;
	qsort(nearBots, NUM_TRACKED_BOTS+1, sizeof(OtherBot), nearBotsDistCmp);
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(nearBots[i].id==id){
			return &(nearBots[i]);
		}
		if(emptyIdx==0xFF && nearBots[i].id==0){
			emptyIdx=i;
		}
	}
	//The ID we're adding wasn't in our neighbors list.
	if(emptyIdx!=0xFF){ //An empty spot was available.
		return &(nearBots[emptyIdx]);
	}

	//BotPos* pos = &(nearBots[NUM_TRACKED_BOTS-1].pos);
	cleanOtherBot(&nearBots[NUM_TRACKED_BOTS]);
	return &(nearBots[NUM_TRACKED_BOTS]);
}

void cleanOtherBot(OtherBot* other){
	if(other==NULL) return;
	other->pos.x = UNDF;
	other->pos.y = UNDF;
	other->pos.o = UNDF;
	other->id = 0;
}

/*
 *	the function below is optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection.
 */
uint8_t user_handle_command(char* command_word, char* command_args){
	if(strcmp_P(command_word,PSTR("ball"))==0){
		if(POS_DEFINED(&myPos)){
			const char delim[2] = " ";
			char* token = strtok(command_args, delim);
			int8_t vel = (token!=NULL) ? (int8_t)atoi(token) : 20;
			token = strtok(NULL, delim);
			uint8_t size = (token!=NULL) ? (uint8_t)atoi(token) : (DROPLET_RADIUS+2);
			theBall.xPos = myPos.x;
			theBall.yPos = myPos.y;
			int16_t randomdir = rand_short()%360;
			theBall.xVel = vel*cos(deg_to_rad(randomdir));
			theBall.yVel = vel*sin(deg_to_rad(randomdir));
			theBall.id = 1;
			theBall.radius = size&0xfc;
			theBall.lastUpdate = get_time();
			printf("got ball command. velocity: %hd\r\n", vel);
		}else{
			uint8_t r = get_red_led();
			set_red_led(255);
			printf("got ball command, but i don't know where i am yet.\r\n");
			set_red_led(r);
		}
		return 1;
	}else if(strcmp_P(command_word,PSTR("mode"))==0){
		switch(command_args[0]){
			case 'p': colorMode = POS;		break;
			case 'g': colorMode = GAME;		break;
			default:  colorMode = GAME;		break;
		}
		switch(command_args[1]){
			case 'p': gameMode = PONG;		break;
			case 'b': gameMode = BOUNCE;	break;
			default:  gameMode = BOUNCE;	break;
		}
		return 1;
	}else if(strcmp_P(command_word, PSTR("ball_kill"))==0){
		killBall();
		return 1;
	}
	return 0;
}
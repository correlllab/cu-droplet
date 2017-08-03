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
	hardEdges = NULL;
	theBall.lastUpdate = 0;
	theBall.xPos = NAN;
	theBall.yPos = NAN;
	theBall.xVel = NAN;
	theBall.yVel = NAN;
	theBall.id = 0;
	theBall.radius = 0;
	msgHandleLock = 1;
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
	updateBall();
	updateColor();
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
			updateHardEdges();
			if(BALL_VALID()){
				printf_P(PSTR("\tBall ID: %hu; radius: %hu; Pos: (% 8.2f, % 8.2f) @ vel (% 6.2f, % 6.2f)\r\n"), theBall.id, theBall.radius, theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel);
			}
		}
		if(BALL_VALID() && myDist!=(uint16_t)UNDF){
			BALL_DEBUG_PRINT("B[%hu]: % 8.2f, % 8.2f (% 6.2f, % 6.2f)\r\n", theBall.id, theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel);
			if(myDist<=DROPLET_RADIUS){
				schedule_task(RNB_DUR+100, prepBallMsg, NULL);
			}
		}
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
		float portionOfSpeedRemaining = 1.0; 
		HardEdge* edge = hardEdges;
		//TODO: Some mechanism to keep all the Droplets in an area each doing redundant bounce calculations?
		float xIntersect, yIntersect;
		int32_t timeRemaining;
		while(edge!=NULL){
			 timeRemaining = timePassed*portionOfSpeedRemaining;
			if(checkBounce(edge, &xIntersect, &yIntersect, timeRemaining)){
				//if(gameMode==PONG && ((SOUTH_PIXEL_Q(myState) && theBall.yVel<=0) || (NORTH_PIXEL_Q(myState) && theBall.yVel>=0))){
					//if(!isCovered){
						////Other Side scores a point!
						//killBall();
						//set_rgb(255,0,0);
						//break;
					//}
				//}
				float distanceToEdge = hypotf(xIntersect-theBall.xPos, yIntersect-theBall.yPos);
				float overallSpeed = hypotf(theBall.xVel, theBall.yVel);
				portionOfSpeedRemaining -= distanceToEdge/overallSpeed;
				BALL_DEBUG_PRINT("\tSpeed: %f (% 6.2f, % 6.2f), distanceToEdge: %f, portionLeft: %f\r\n", overallSpeed, theBall.xVel, theBall.yVel, distanceToEdge, portionOfSpeedRemaining);
				theBall.xPos = xIntersect;
				theBall.yPos = yIntersect;

				int16_t edgeDeltaX = edge->xEnd - edge->xStart;
				int16_t edgeDeltaY = edge->yEnd - edge->yStart;
				calculateBounce(-edgeDeltaY, edgeDeltaX);
				BALL_DEBUG_PRINT("\tSpeed After: %f (% 6.2f, % 6.2f)\r\n", hypotf(theBall.xVel, theBall.yVel), theBall.xVel, theBall.yVel);
				edge = hardEdges; //We need to start searching through edges again to make sure there aren't two+ bounces within this timestep.
			}else{
				edge = edge->next;
			}
		}
		timeRemaining = portionOfSpeedRemaining*timePassed;
		theBall.xPos += ((theBall.xVel*timeRemaining)/1000.0);
		theBall.yPos += ((theBall.yVel*timeRemaining)/1000.0);
		myDist = (uint16_t)hypotf(myPos.x-theBall.xPos, myPos.y-theBall.yPos);
		theBall.lastUpdate = now;

		if(myDist > 250){
			BALL_DEBUG_PRINT("Ball moved far away, so we're going to stop tracking it.\r\n");
			theBall.xPos = NAN;
			theBall.yPos = NAN;
			theBall.id = 0x0F;
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

void updateHardEdges(){
	//First, making a copy of nearBots so we can sort it by a weird metric.
	cleanHardEdges(); //clean out the previous hardBots list -- we start fresh each farme.
	qsort(nearBots, NUM_TRACKED_BOTS+1, sizeof(OtherBot), nearBotsDistCmp);
	//sort nearBots according to their bearing.
	qsort(nearBots, NUM_TRACKED_BOTS, sizeof(OtherBot), nearBotsBearingCmp);
	//go through each near bot by bearing and add it to hardBots if the gap in bearings is above 120 degrees
	uint8_t nextI;
	
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		//TODO : Extend 'valid for hard edge' criteria to also have range limitations?
		if(nearBots[i].id==0 || !POS_DEFINED(&(nearBots[i].pos))) continue;
		for(nextI=((i+1)%NUM_TRACKED_BOTS);nextI!=i;nextI=((nextI+1)%NUM_TRACKED_BOTS)){
			if(nearBots[nextI].id==0 || !POS_DEFINED(&(nearBots[nextI].pos))){
				continue;
			}else{
				break;
			}
		}
		if(i==nextI){
			break; //This should only happen when there's only one trackedBot, in which case we can't say anything intelligent about hard edges.
		}
		int16_t iBearing =  rad_to_deg(atan2(nearBots[i].pos.y - myPos.y, nearBots[i].pos.x - myPos.x));
		int16_t iNextBearing = rad_to_deg(atan2(nearBots[nextI].pos.y - myPos.y, nearBots[nextI].pos.x - myPos.x));
		int16_t difference = abs((iBearing-iNextBearing + 540)%360 - 180);
		if(difference>120){
			addHardEdge(nearBots[i].pos.x, nearBots[i].pos.y, nearBots[nextI].pos.x, nearBots[nextI].pos.y);
		}
		if(nextI==0){
			break; //This will happen after we've gone through every tracked bot. We DO want to process it, though, and thereby close the circle.
		}
	}
	//print out hard bots list.
	if(hardEdges!=NULL){
		BALL_DEBUG_PRINT("Edges:");
		HardEdge* tmp = hardEdges;
		while(tmp!=NULL){
			BALL_DEBUG_PRINT("\t(%4d, %4d)<->(%4d, %4d)\r\n", tmp->xStart, tmp->yStart, tmp->xEnd, tmp->yEnd);
			tmp = tmp->next;
		}
		BALL_DEBUG_PRINT("\r\n");
	}
}

void prepBallMsg(){	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(msgHandleLock){
			msgHandleLock = 0;

			if(preppedMsgHandle==NULL){
				BallMsg* msg = (BallMsg*)myMalloc(sizeof(BallMsg));
				msg->xPos		= theBall.xPos;
				msg->yPos		= theBall.yPos;
				msg->id			= theBall.id;
				msg->xVel		= (int8_t)theBall.xVel;
				msg->yVel		= (int8_t)theBall.yVel;
				msg->radius		= theBall.radius;
				msg->flag		= 0; //Using this as number of send attempts, for now.
				preppedMsgHandle = msg;
				schedule_task(5, (arg_func_t)sendBallMsg, msg);
			}else{
				preppedMsgHandle->xPos		= theBall.xPos;
				preppedMsgHandle->yPos		= theBall.yPos;
				preppedMsgHandle->id			= theBall.id;
				preppedMsgHandle->xVel		= (int8_t)theBall.xVel;
				preppedMsgHandle->yVel		= (int8_t)theBall.yVel;
				preppedMsgHandle->radius		= theBall.radius;
			}

			msgHandleLock = 1;
		}else{
			printf("Message handle locked.\r\n");
		}
	}
}

void sendBallMsg(BallMsg* msg){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(!msgHandleLock){
			schedule_task(5, (arg_func_t)sendBallMsg, msg);
			return;
		}else{
			msgHandleLock = 0;
		}
	}
	if(ir_is_busy(ALL_DIRS)){
		if(msg->flag>5){
			printf("Giving up on sending this ball msg.\r\n");
			myFree(msg);
			preppedMsgHandle = NULL;
		}else{
			printf("Waiting to send ball msg.\r\n");
			uint32_t backOffMax = pow(2,msg->flag)-1;
			uint32_t backOffTime = rand_quad()%(backOffMax);
			backOffTime*=50;
			schedule_task(backOffTime, (arg_func_t)sendBallMsg, msg);
		}
		msg->flag += 1;
	}else{
		msg->flag = BALL_MSG_FLAG;
		ir_send(ALL_DIRS, (char*)msg, sizeof(BallMsg));
		lastBallMsg=get_time();
		myFree(msg);
		printf("Ball message sent.\r\n");
		preppedMsgHandle = NULL;
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		msgHandleLock = 1;
	}
}

void handleBallMsg(BallMsg* msg, uint32_t arrivalTime){
	BALL_DEBUG_PRINT("Got Ball! T: %lu\r\n", get_time());
	if(BALL_VALID()){
		BALL_DEBUG_PRINT("\tBEFORE | Pos: (% 8.2f, % 8.2f)   Vel: (% 6.2f, % 6.2f) | lastUpdate: %lu\r\n", theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel, theBall.lastUpdate);
	}
	if(msg->id == 0x0F && theBall.id!=0x0F){
		lastBallID = theBall.id;
		set_rgb(255,0,0);
	}else if(msg->id == lastBallID && theBall.id==0x0F){
		return; //this is from someone who hasn't realized the ball is dead, yet.
	}
	theBall.xPos = (float)msg->xPos;
	theBall.yPos = (float)msg->yPos;
	theBall.id = msg->id;
	theBall.xVel = (float)msg->xVel;
	theBall.yVel = (float)msg->yVel;
	theBall.radius = (msg->radius);
	theBall.lastUpdate = arrivalTime-4;
	BALL_DEBUG_PRINT("\t AFTER | Pos: (% 8.2f, % 8.2f)   Vel: (% 6.2f, % 6.2f) | lastUpdate: %lu\r\n", theBall.xPos, theBall.yPos, theBall.xVel, theBall.yVel, theBall.lastUpdate);
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
	OtherBot* otherBot = addOtherBot(senderID);
	otherBot->id = senderID;
	copyBotPos(&(msg->pos), &(otherBot->pos));
	//printf("%04X @ {%4d, %4d, % 4d}\r\n", senderID, (msg->pos).x, (msg->pos).y, (msg->pos).o);
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

void printNearBots(){
	printf_P(PSTR("\t                         Near Bots                        \r\n"));
	printf_P(PSTR("\t ID  |   x  |   y  |   o\r\n"));
	for(uint8_t i=0; i<NUM_TRACKED_BOTS+1; i++){
		printOtherBot(&nearBots[i]);
	}
	printf("\r\n");
}

void printOtherBot(OtherBot* bot){
	if(bot==NULL || !POS_DEFINED(&(bot->pos))) return;
	BotPos*  pos = &(bot->pos);
	printf("\t%04X | %4d | %4d | %4d\r\n", bot->id, pos->x, pos->y, pos->o);
}

void addHardEdge(int16_t fromX, int16_t fromY, int16_t toX, int16_t toY){
	if(hardEdges==NULL){
		hardEdges = (HardEdge*)myMalloc(sizeof(HardEdge));
		hardEdges->xStart = fromX;
		hardEdges->yStart = fromY;
		hardEdges->xEnd = toX;
		hardEdges->yEnd = toY;
		hardEdges->next = NULL;
	}else{
		HardEdge* temp = hardEdges;
		while(temp->next!=NULL){
			temp = temp->next;
		}
		temp->next = (HardEdge*)myMalloc(sizeof(HardEdge));
		temp->next->xStart = fromX;
		temp->next->yStart = fromY;
		temp->next->xEnd = toX;
		temp->next->yEnd = toY;
		temp->next->next = NULL;
	}
}

void cleanHardEdges(){
	HardEdge* temp;
	while(hardEdges!=NULL){
		temp = hardEdges->next;
		myFree(hardEdges);
		hardEdges = temp;
	}
}

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
			token = strtok(NULL, delim);
			int16_t randomdir = (token!=NULL) ? (int16_t)atoi(token) : ( ((int16_t)(rand_quad()%360)) - 180);
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
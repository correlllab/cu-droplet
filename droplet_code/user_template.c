#include "user_template.h"



void init(){
	if((LOCALIZATION_DUR)>=SLOT_LENGTH_MS){
		printf_P(PSTR("Error! Localization requires SLOT_LENGTH_MS to be greater than LOCALIZATION_DUR!\r\n"));
	}
	queueInit();
	myRole = UNKNOWN;
	myButton = BUTTON_UNKNOWN;
	loopID = 0xFFFF;
	isShifted = 0;
	lastKeypress = 0;
	wireSleepTask = NULL;
	isWired = 0;
	newPos.x = UNDF;
	newPos.y = UNDF;
	newPos.o = UNDF;
	periodicMouseBroadcast = 0;
	isBlinking = 0;
	leftMouseID = 0xFFFF;
	frameStart = getTime();
	mySlot = getSlot(getDropletID());
	//printf("mySlot: %u, frame_length: %lu\r\n\r\n", mySlot, FRAME_LENGTH_MS);
	if(!POS_DEFINED(&myPos)){
		//BotPos tmpPos;
		//tmpPos.x = (BUTTON_HALFWIDTH + BUTTON_WIDTH*(randShort()%10));
		//tmpPos.y = 10 + ((randShort()%3)+1)*BUTTON_HEIGHT;
		//setRoleAndButton(getButtonFromPosition(&tmpPos));
		//printf("%d, %d, %hu, %hu\r\n", tmpPos.x, tmpPos.y, myRole, myButton);
	}else{
		setRoleAndButton(getButtonFromPosition(&myPos));
		printf("%d, %d, %hu, %hu\r\n", myPos.x, myPos.y, myRole, myButton);
	}

	
	//if(POS_DEFINED(&myPos)){
	//	setRoleAndButton(getButtonFromPosition(&myPos));
	//}
	#ifdef AUDIO_DROPLET
		enableMicInterrupt();
	#endif	
	//setAllirPowers(225);
}

static void roleUnknownLoop(void){
	uint32_t frameTime = getTime()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
	}
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){ //This is a new slot.
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID==mySlot){ //This is my slot.
			blinkLED(1,30);
			broadcastRnbData();
		}else if(loopID==SLOTS_PER_FRAME-1){
			printf("%lu\r\n", getTime());
			if(POS_DEFINED(&myPos)){
				printf("\tMy Pos: {%d, %d, %d}\r\n", myPos.x, myPos.y, myPos.o);
				printPosCovar(&myPosCovar);
				printf("\r\n");
				checkPosition();
			}
		}
		if(!isBlinking){
			uint8_t newR = 0, newG = 0, newB = 0;
			getPosColor(&newR, &newG, &newB);
			setRGB(newR, newG, newB);
		}
	}
}

static void mouseLoop(void){
	uint32_t frameTime = getTime()-frameStart;
	if(frameTime>MK_FRAME_LENGTH){
		frameTime = frameTime-MK_FRAME_LENGTH;
		frameStart += MK_FRAME_LENGTH;
	}
	if(loopID!=(frameTime/MK_SLOT_LENGTH)){
		loopID = frameTime/MK_SLOT_LENGTH;
		if(loopID==0 && periodicMouseBroadcast){
			blinkLED(1,30);
			broadcastRnbData();
		}else if(loopID==MK_SLOTS_PER_FRAME-2){
			if(POS_DEFINED(&newPos)){
				MouseMoveEvent evt;
				evt.deltaX = newPos.x - myPos.x;
				evt.deltaY = newPos.y - myPos.y;
				evt.mouseEventMarker = MOUSE_EVENT_MARKER_FLAG;
				evt.time = getTime();
				printf("(% 5d, % 5d) -> (% 5d, % 5d)\r\n", myPos.x, myPos.y, newPos.x, newPos.y);								
				prepMouseMoveMsg(&evt);
				myPos = newPos;
				newPos.x = UNDF;
				newPos.y = UNDF;
				newPos.o = UNDF;
			}
		}else{
			ButtonPressEvent thisEvt;
			checkForEvent(frameStart+MK_SLOT_LENGTH*prevLoopID, frameStart+MK_SLOT_LENGTH*loopID, &thisEvt);
			if(thisEvt.button!=BUTTON_UNKNOWN && thisEvt.button!=MOUSE_EVENT_MARKER_FLAG){
				if(thisEvt.src == getDropletID()){
					if(isWired){
						wireTxButtonPress(thisEvt.button);
					}else{
						prepButtonPressMsg(&thisEvt);
					}
				}
			}		
			prevLoopID = loopID;
		}
		if(!isBlinking){
			setRGB(5,0, 15);
		}		
	}
}

static void keyboardLoop(void){
	uint32_t frameTime = getTime()-frameStart;
	if(frameTime>MK_FRAME_LENGTH){
		frameTime = frameTime-MK_FRAME_LENGTH;
		frameStart += MK_FRAME_LENGTH;
	}
	if(loopID!=(frameTime/MK_SLOT_LENGTH)){
		loopID = frameTime/MK_SLOT_LENGTH;
		ButtonPressEvent thisEvt;
		checkForEvent(frameStart+MK_SLOT_LENGTH*(loopID-1), frameStart+MK_SLOT_LENGTH*loopID, &thisEvt);
		if(thisEvt.button!=BUTTON_UNKNOWN && thisEvt.button!=MOUSE_EVENT_MARKER_FLAG){
			if(thisEvt.src == getDropletID()){
				if(myButton == BUTTON_SHIFT){
					isShifted = handleShiftPressed();
					thisEvt.button = isShifted ? BUTTON_CAPSLOCK_ON : BUTTON_CAPSLOCK_OFF;
				}
				if(isWired){
					wireTxButtonPress(thisEvt.button);
				}else{
					prepButtonPressMsg(&thisEvt);
				}
			}else{
				if(isWired){
					wireTxButtonPress(thisEvt.button);
				}else if(thisEvt.button==BUTTON_SHIFT){
					prepButtonPressMsg(&thisEvt);
				}
			}
		}
		if(!isBlinking){
			setRGB(0,0, 15);
		}
	}
}

void loop(){
	switch(myRole){
		case  UNKNOWN: roleUnknownLoop(); break;
		case    MOUSE: mouseLoop(); break;
		case KEYBOARD: keyboardLoop(); break;
	}
	delayMS(LOOP_DELAY_MS);
}

void handleMeas(Rnb* meas){
	RNB_DEBUG_PRINT("\t(RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
	if(myRole != MOUSE){
		useRNBmeas(meas);
	}
}

void handleMsg(irMsg* msgStruct){
	if(IS_BOT_MEAS_MSG(msgStruct)){
		if(myRole != MOUSE){ //mouse will be moving too much to participate in localization.
			handleBotMeasMsg((BotMeasMsg*)(msgStruct->msg), msgStruct->senderID);
		}else if(myRole == MOUSE && periodicMouseBroadcast){
			mouseHandleBotMeasMsg((BotMeasMsg*)(msgStruct->msg));
		}//The right mouse button isn't going to worry about its position at all.
	}else if(IS_BUTTON_PRESS_MSG(msgStruct)){
		handleButtonPressMsg((ButtonPressMsg*)(msgStruct->msg));
	}else if(IS_MOUSE_MOVE_MSG(msgStruct)){
		handleMouseMoveMsg((MouseMoveMsg*)(msgStruct->msg));
	}
}

static void mouseUpdatePos(BotPos* pos, Matrix* yourP){
	Vector xMe = {newPos.x, newPos.y, degToRad(newPos.o)};
	Vector xMeFromYou = {pos->x, pos->y, degToRad(pos->o)};
	Matrix myP;
	decompressP(&myP, &newPosCovar);
	Matrix myNewP;
	Vector myNewPos;

	covarIntersection(&myNewPos, &myNewP, &xMe, &myP, &xMeFromYou, yourP);
	if(!positiveDefiniteQ(&myNewP)){
		return;
	}
	float updateDist = updateDistance(&xMe, &myP, &xMeFromYou, yourP);
	if(updateDist>4.0){
		return;
	}else if(updateDist>1.0){
		covarUnion(&myNewPos, &myNewP, &xMe, &myP, &xMeFromYou, yourP);
		if(!positiveDefiniteQ(&myNewP)){
			return;
		}
	}

	newPos.x = myNewPos[0]>8191 ? 8191 : (myNewPos[0]<-8192 ? -8192 : myNewPos[0]);
	newPos.y = myNewPos[1]>8191 ? 8191 : (myNewPos[1]<-8192 ? -8192 : myNewPos[1]);
	newPos.o = (radToDeg(myNewPos[2]) + 0.5);
	compressP(&myNewP, &newPosCovar);
}

void mouseHandleBotMeasMsg(BotMeasMsg* msg){
	if(!POS_DEFINED(&(msg->pos))){
		return;
	}
	Matrix covar;
	decompressP(&covar, &(msg->covar));
	if(!POS_DEFINED(&newPos)){
		newPos = msg->pos;
		for(uint8_t i=0;i<6;i++){
			newPosCovar[i].u = msg->covar[i].u;
		}
	}else{
		mouseUpdatePos(&(msg->pos), &covar);
	}
}


void handleButtonPressMsg(ButtonPressMsg* msg){
	ButtonPressEvent* evt = &(msg->evt);
	evt->time = (getTime()/MIN_MULTIPRESS_DELAY)*MIN_MULTIPRESS_DELAY + ((evt->time)%MIN_MULTIPRESS_DELAY);
	if(evt->button == BUTTON_L_CLICK){
		leftMouseID = evt->src;
	}
	if(addEvent(evt)){
		if(evt->button==BUTTON_CAPSLOCK_ON){
			isShifted = 1;
			setGreenLED(5);
		}else if(evt->button==BUTTON_CAPSLOCK_OFF){
			isShifted = 0;
			setGreenLED(0);
		}
	}
}

void handleMouseMoveMsg(MouseMoveMsg* msg){
	MouseMoveEvent evt;
	evt.time = (getTime()/MIN_MULTIPRESS_DELAY)*MIN_MULTIPRESS_DELAY + ((msg->time)%MIN_MULTIPRESS_DELAY);
	evt.deltaX = msg->deltaX;
	evt.deltaY = msg->deltaY;
	evt.mouseEventMarker = MOUSE_EVENT_MARKER_FLAG;
	if(addEvent(&evt)){
		if(isWired){
			wireMouseMove(evt.deltaX, evt.deltaY);
		}else{
			//prepMouseMoveMsg(&evt);
		}
	}
}

//This seems to take ~600us per sample.
#define NUM_CHECK_SAMPLES 100
inline static uint8_t targetMaxCount(void){
	if(getTime()<120000) return 255; //Impossible for the first two minutes.
	if(getTime()<600000) return (uint8_t)(0.5*NUM_CHECK_SAMPLES); //Less high for 2-10 minutes
	return (uint8_t)(0.33*NUM_CHECK_SAMPLES);
}

#define NUM_MAXIMUMS_TRACKED 6
void checkPosition(){
	Matrix covar;
	decompressP(&covar, &myPosCovar);
	Vector eigValues;
	Matrix eigVectors;
	eigensystem(&eigValues, &eigVectors, &covar);
	
	Matrix diagSqrtEigValues = {{sqrt(eigValues[0]), 0, 0}, {0, sqrt(eigValues[1]), 0}, {0, 0, sqrt(eigValues[2])}};
	matrixInplaceMultiply(&diagSqrtEigValues, &eigVectors, &diagSqrtEigValues);
	Vector randNormSample;
	Vector result;
	BotPos resultPos;
	resultPos.o = 0;
	Button resultKeys[(LARGEST_KEYBOARD_KEY+1)] = {0}; //according to internet, this initializes everything to 0.
	for(uint8_t i=0;i<NUM_CHECK_SAMPLES;i++){
		randNormSample[0] = randNorm(0,1);
		randNormSample[1] = randNorm(0,1);
		randNormSample[2] = randNorm(0,1);
		matrixTimesVector(&result, &diagSqrtEigValues, &randNormSample);
		resultPos.x = myPos.x + result[0];
		resultPos.y = myPos.y + result[1];
		resultKeys[getButtonFromPosition(&resultPos)]++;
	}
	Button maxButtons[NUM_MAXIMUMS_TRACKED] = {BUTTON_UNKNOWN};
	uint8_t maxButtonCounts[NUM_MAXIMUMS_TRACKED] = {0};	
	for(uint8_t i=0;i<=LARGEST_KEYBOARD_KEY;i++){
		for(uint8_t j=0;j<NUM_MAXIMUMS_TRACKED;j++){
			if(resultKeys[i]>maxButtonCounts[j]){
				for(uint8_t k=(NUM_MAXIMUMS_TRACKED-1);k>j;k--){
					maxButtons[k] = maxButtons[k-1];
					maxButtonCounts[k] = maxButtonCounts[k-1];
				}
				maxButtons[j] = i;
				maxButtonCounts[j] = resultKeys[i];
				break;
			}
		}
	}
	printf("Max Keys:\r\n");
	printf(isprint(maxButtons[0]) ? "'%c' (%hu)" : "\\%2hu (%hu)", maxButtons[0], maxButtonCounts[0]);
	for(uint8_t i=1;i<NUM_MAXIMUMS_TRACKED;i++){
		printf(isprint(maxButtons[i]) ? ", '%c' (%hu)" : ", \\%2hu (%hu)", maxButtons[i], maxButtonCounts[i]);
	}
	printf("\r\n");
	if(maxButtonCounts[0]>targetMaxCount() && maxButtons[0]!=BUTTON_UNKNOWN){
		setRoleAndButton(maxButtons[0]);
	} 
}

void rnbBroadcastDebugWrapper(){
	printf("Pre Broadcast\r\n");
	broadcastRnbData();
	printf("Post Broadcast\r\n");
}

void userMicInterrupt(){
	if(myRole==UNKNOWN || getTime()<1500 || ( (getTime()-lastKeypress) < MIN_MULTIPRESS_DELAY ) ){
		return;
	}
	lastKeypress = getTime();
	ButtonPressEvent evt;
	buildButtonPressEvent(&evt);
	if(myButton == BUTTON_L_CLICK){
		periodicMouseBroadcast = 1;
	}
	if(addEvent(&evt)){ //This keeps us from repeating or otherwise responding to ourselves.
		blinkLED(2, 100);	
	}
}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection.
 //*/
uint8_t userHandleCommand(char* commandWord, char* commandArgs __attribute__ ((unused))){
	if(strcmp(commandWord,"WAKE")==0){
		if(wireSleepTask!=NULL){
			removeTask(wireSleepTask);
		}
		wireSleepTask = scheduleTask(10000, wireSleep, NULL);
		isWired = 1;
		return 1;
	}else if(strcmp(commandWord,"press")==0){
		char charPressed = commandArgs[0];
		if(charPressed == '\0'){
			return 0;
		}else if(isprint(charPressed)){
			ButtonPressEvent evt;
			if(isupper(charPressed) && !isShifted){
				evt.button = tolower(charPressed);
			}else if(islower(charPressed) && isShifted){
				evt.button = toupper(charPressed);
			}else{
				evt.button = charPressed;
			}
			evt.time = getTime();
			evt.src = getDropletID();			
			printf("PRESSED:    '%c'\r\n", evt.button);
			addEvent(&evt);
		}
		return 1;	
	}else if(strcmp(commandWord,"set_role")==0){
		Button button = atoi(commandArgs);
		setRoleAndButton(button);		
		return 1;
	}else{
		return 0;
	}
}

void prepMouseMoveMsg(MouseMoveEvent* evt){
	if(isWired){
		wireMouseMove(evt->deltaX, evt->deltaY);
	}else{
		MouseMoveMsg msg;
		msg.time   = evt->time;
		msg.deltaX = evt->deltaX;
		msg.deltaY = evt->deltaY;
		msg.flag = MOUSE_MOVE_MSG_FLAG;
		MouseMoveMsgNode* msgNode = (MouseMoveMsgNode*)myMalloc(sizeof(MouseMoveMsgNode));
		msgNode->numTries = 0;
		msgNode->msg = msg;
		sendMouseMoveMsg(msgNode);
	}
}

void sendMouseMoveMsg(MouseMoveMsgNode* msgNode){
	if(irIsBusy(ALL_DIRS)){
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			if( msgNode->numTries > 6 ){
				myFree(msgNode);
			}else{
				scheduleTask(getExponentialBackoff(msgNode->numTries), (arg_func_t)sendButtonPressMsg, (void*)msgNode);
			}
			msgNode->numTries++;
		}
	}else{
		irSend(ALL_DIRS, (char*)(&(msgNode->msg)), sizeof(MouseMoveMsg));
		myFree(msgNode);
	}
}

void prepButtonPressMsg(ButtonPressEvent* evt){
	ButtonPressMsg msg;
	msg.evt = *evt;
	msg.flag = KEYPRESS_MSG_FLAG;
	ButtonPressMsgNode* msgNode = (ButtonPressMsgNode*)myMalloc(sizeof(ButtonPressMsgNode));
	msgNode->numTries = 0;
	msgNode->msg = msg;
	sendButtonPressMsg(msgNode);
}

void sendButtonPressMsg(ButtonPressMsgNode* msgNode){
	if(irIsBusy(ALL_DIRS)){
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			if(msgNode->numTries>6){
				myFree(msgNode);
			}else{
				scheduleTask(getExponentialBackoff(msgNode->numTries), (arg_func_t)sendButtonPressMsg, (void*)msgNode);
			}
			msgNode->numTries++;
		}
	}else{
		irSend(ALL_DIRS, (char*)(&(msgNode->msg)), sizeof(ButtonPressMsg));
		myFree(msgNode);
		//if(rebroadcastButton(((msgNode->msg).evt).button)){
			//msgNode->numTries = 0;
			//scheduleTask(200, (arg_func_t)sendButtonPressMsg, (void*)msgNode);
		//}else{
			//myFree(msgNode);
		//}
	}
}

void wireSleep(){
	isWired = 0;
}

uint32_t getExponentialBackoff(uint8_t c){
	volatile uint32_t k;
	volatile uint32_t N;

	N= (((uint32_t)1)<<c);

	k = randQuad()%N;
	return ((k*IR_MSG_TIMEOUT));
}
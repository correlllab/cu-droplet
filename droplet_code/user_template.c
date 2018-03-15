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
	frameCount = 0;
	lastKeypress = 0;
	wireSleepTask = NULL;
	isWired = 0;
	periodicMouseBroadcast = 0;
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

void loop(){
	uint32_t frameTime = getTime()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
	}
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){ //This is a new slot.
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID==mySlot){ //This is my slot.
			if(myRole == UNKNOWN){
				blinkLED(1,30);
				broadcastRnbData();
			}
		}else if(loopID==SLOTS_PER_FRAME-1){ //This is the end-of-frame slot.
			if((myRole==UNKNOWN) && POS_DEFINED(&myPos)){
				printf_P(PSTR("\tMy Pos: {%d, %d, %d}\r\n"), myPos.x, myPos.y, myPos.o);
				printPosCovar(&myPosCovar);
				printf("\r\n");
				uint32_t before = getTime();
				checkPosition();
				printf("Check Position Took %lu ms for 100 runs.\r\n", getTime()-before);
			}
		}
		if(myRole == UNKNOWN){
			uint8_t newR = 0, newG = 0, newB = 0;
			getPosColor(&newR, &newG, &newB);
			setRGB(newR, newG, newB);
		}
	}
	if(periodicMouseBroadcast && myButton==BUTTON_L_CLICK && (getTime()-lastBroadcast > MOUSE_RNB_BROADCAST_PERIOD)){
		blinkLED(1,30);
		int16_t prevX = myPos.x;
		int16_t prevY = myPos.y;
		uint8_t posChanged = combineBotMeasEvents();
		
		int16_t deltaX = myPos.x-prevX;
		int16_t deltaY = myPos.y-prevY;
		
		broadcastRnbData();
		lastBroadcast=getTime();
		if(posChanged){
			waitForTransmission(ALL_DIRS);
			MouseMoveEvent evt;
			evt.deltaX = deltaX;
			evt.deltaY = deltaY;
			evt.mouseEventMarker = MOUSE_EVENT_MARKER_FLAG;
			evt.time = getTime();
			prepMouseMoveMsg(&evt);
		}
	}
	if(rnb_updated){
		RNB_DEBUG_PRINT("\t(RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		if(myRole != MOUSE){
			useRNBmeas(&last_good_rnb);
		}

		rnb_updated=0;
	}
	delayMS(LOOP_DELAY_MS);
}

uint8_t combineBotMeasEvents(){
	int16_t deltaX=0;
	int16_t deltaY=0;
	uint8_t numEncountered = 0;
	for(uint8_t i=0;i<NUM_LOGGED_EVENTS;i++){
		MouseMoveEvent* evt = &(eventLog[i]);
		if(evt->mouseEventMarker==MOUSE_EVENT_MARKER_FLAG && evt->time > lastBroadcast){
			numEncountered++;
			deltaX+=evt->deltaX;
			deltaY+=evt->deltaY;
		}
	}
	if(numEncountered>0){
		deltaX = deltaX/numEncountered;
		deltaY = deltaY/numEncountered;
		myPos.x += deltaX;
		myPos.y += deltaY;
		return 1;
	}else{
		return 0;
	}
}

void handleMsg(irMsg* msgStruct){
	if(IS_BOT_MEAS_MSG(msgStruct)){
		if(myRole != MOUSE){ //mouse will be moving too much to participate in localization.
			handleBotMeasMsg((BotMeasMsg*)(msgStruct->msg), msgStruct->senderID);
		}else if(myButton == BUTTON_L_CLICK && periodicMouseBroadcast){
			BotMeasMsg* botMeas = (BotMeasMsg*)(msgStruct->msg);
			BotPos* pos = (BotPos*)(&(botMeas->pos));
			if(POS_DEFINED(pos)){
				int16_t prevX = myPos.x;
				int16_t prevY = myPos.y;
				MouseMoveEvent evt;
				evt.deltaX = myPos.x - myPos.x;
				evt.deltaY = myPos.y - myPos.y;
				if(evt.deltaX!=0 || evt.deltaY!=0){
					evt.mouseEventMarker=0xFF;
					evt.time = getTime();
					addEvent(&evt);
				}
			}
		}//The right mouse button isn't going to worry about its position at all.
	}else if(IS_BUTTON_PRESS_MSG(msgStruct)){
		handleButtonPressMsg((ButtonPressMsg*)(msgStruct->msg));
	}else if(IS_MOUSE_MOVE_MSG(msgStruct)){
		handleMouseMoveMsg((MouseMoveMsg*)(msgStruct->msg));
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
			prepButtonPressMsg(&(msg->evt));
		}else if(evt->button==BUTTON_CAPSLOCK_OFF){
			isShifted = 0;
			setGreenLED(0);
			prepButtonPressMsg(&(msg->evt));
		}else if(isWired){
			wireTxButtonPress(evt->button);		
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
	uint8_t maxButtonCount = 0;
	Button maxButton = BUTTON_UNKNOWN;
	for(uint8_t i=0;i<=LARGEST_KEYBOARD_KEY;i++){
		if(resultKeys[i]>maxButtonCount){
			maxButtonCount = resultKeys[i];
			maxButton = i;
		}
	}
	printf("       Max Key: '");
	printf(isprint(maxButton) ? "%c" : "\\%hu", maxButton);
	printf("' (%hu)\r\n", maxButtonCount);
	if(maxButtonCount>targetMaxCount() && maxButton!=BUTTON_UNKNOWN){
		setRoleAndButton(maxButton);
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
	if(addEvent(&evt)){ //This keeps us from repeating or otherwise responding to ourselves.
		blinkLED(2, 100);	
		if(myButton==BUTTON_SHIFT){
			isShifted = handleShiftPressed();
			evt.button =  isShifted ? BUTTON_CAPSLOCK_ON : BUTTON_CAPSLOCK_OFF;
		}
		if(myButton == BUTTON_L_CLICK){
			periodicMouseBroadcast = 1;
			lastBroadcast=getTime();
		}
		if(isWired && myButton != BUTTON_SHIFT){
			wireTxButtonPress(evt.button);
		}else{
			prepButtonPressMsg(&evt);
		}
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
			if(addEvent(&evt)){
				prepButtonPressMsg(&evt);	
			}
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
			if( msgNode->numTries > 10 || (getTime() - msgNode->msg.time > MAX_EVENT_MSG_AGE) ){
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
			if(msgNode->numTries>10 || ((getTime() - (msgNode->msg).evt.time)>MAX_EVENT_MSG_AGE) ){
				myFree(msgNode);
			}else{
				scheduleTask(getExponentialBackoff(msgNode->numTries), (arg_func_t)sendButtonPressMsg, (void*)msgNode);
			}
			msgNode->numTries++;
		}
	}else{
		irSend(ALL_DIRS, (char*)(&(msgNode->msg)), sizeof(ButtonPressMsg));
		if(rebroadcastButton(((msgNode->msg).evt).button)){
			msgNode->numTries = 0;
			scheduleTask(200, (arg_func_t)sendButtonPressMsg, (void*)msgNode);
		}else{
			myFree(msgNode);
		}
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
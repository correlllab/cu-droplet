#include "user_template.h"

void init(){
	if((LOCALIZATION_DUR)>=SLOT_LENGTH_MS){
		printf_P(PSTR("Error! Localization requires SLOT_LENGTH_MS to be greater than LOCALIZATION_DUR!\r\n"));
	}
	myRole = UNKNOWN;
	myButton = BUTTON_UNKNOWN;
	loopID = 0xFFFF;
	isShifted = 0;
	frameCount = 0;
	lastKeypress = 0;
	wireSleepTask = NULL;
	isWired = 0;
	mouseBroadcastTask = NULL;
	leftMouseID = 0xFFFF;
	for(uint8_t i=0;i<3;i++){
		wiredBlinkLEDStore[i] = 0;
		buttonPressBlinkLEDStore[i] = 0;
	}
	frameStart = getTime();
	mySlot = getSlot(getDropletID());
	printf("mySlot: %u, frame_length: %lu\r\n\r\n", mySlot, FRAME_LENGTH_MS);
	if(POS_DEFINED(&myPos)){
		myRole = getRoleFromPosition(&myPos);
		setRGB(0,0,15);
	}
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
	if(rnb_updated){
		RNB_DEBUG_PRINT("\t(RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		if(myRole != MOUSE){
			if(last_good_rnb.id == leftMouseID){
				BotPos pos;
				DensePosCovar covar;
				
				uint8_t result = calcOtherBotPosFromMeas(&pos, &covar, &last_good_rnb);
				if(result){
					prepMouseMoveMsg(&pos, &covar);
				}
			}else{
				useRNBmeas(&last_good_rnb);
			}
		}

		rnb_updated=0;
	}
	delayMS(LOOP_DELAY_MS);
}

void handleMsg(irMsg* msgStruct){
	if(IS_BOT_MEAS_MSG(msgStruct)){
		if(myRole != MOUSE){ //mouse will be moving too much to participate in localization.
			handleBotMeasMsg((BotMeasMsg*)(msgStruct->msg), msgStruct->senderID);
		}
	}else if(IS_BUTTON_PRESS_MSG(msgStruct)){
		handleButtonPressMsg((ButtonPressMsg*)(msgStruct->msg));
	}else if(IS_MOUSE_MOVE_MSG(msgStruct)){
		handleMouseMoveMsg((MouseMoveMsg*)(msgStruct->msg));
	}
}

void handleButtonPressMsg(ButtonPressMsg* msg){
	ButtonPressEvent* evt = &(msg->evt);
	if(evt->key == BUTTON_L_CLICK){
		leftMouseID == evt->src;
	}
	if(addEvent(evt)){
		if(evt->key==BUTTON_SHIFT){
			isShifted = !isShifted;
			prepButtonPressMsg(&(msg->evt));
		}else if(isWired){
			wireTxButtonPress(evt->key);
		}
		prepButtonPressMsg(&(msg->evt));
	}
}

void handleMouseMoveMsg(MouseMoveMsg* msg){
	
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
	//uint8_t secondMaxKeyCount = 0;
	//KeyboardKey secondMaxKey = KEYBOARD_UNKNOWN;
	//for(uint8_t i=0;i<=LARGEST_KEYBOARD_KEY;i++){
		//if(i!=maxKey){
			//if(resultKeys[i]>secondMaxKeyCount){
				//secondMaxKeyCount = resultKeys[i];
				//secondMaxKey = i;
			//}
		//}
	//}
	//printf("Second Max Key: '");
	//printf(isprint(secondMaxKey) ? "%c" : "\\%hu", secondMaxKey);
	//printf("' (%hu)\r\n", secondMaxKeyCount);		
	if(maxButtonCount>targetMaxCount() && maxButton!=BUTTON_UNKNOWN){
		myButton = maxButton;
		if( (myButton!=BUTTON_L_CLICK) && (myButton!=BUTTON_R_CLICK) ){
			myRole = KEYBOARD;
			setRGB(0,0,15);
		}else{
			myRole = MOUSE;
			setRGB(10,0,15);
		}
	} 
}

void restoreLED(volatile LEDStore* vals){
	setRGB((*vals)[0], (*vals)[1], (*vals)[2]);
}

void userMicInterrupt(){
	if( (getTime()-lastKeypress) < MIN_MULTIPRESS_DELAY){
		return;
	}
	if(myRole==UNKNOWN){
		return;
	}
	lastKeypress = getTime();
	if(myButton!=BUTTON_SHIFT){
		if(storeAndSetLED(0, 80, 120, &buttonPressBlinkLEDStore)){
			scheduleTask(150, (arg_func_t)restoreLED, (void*)&buttonPressBlinkLEDStore);
		}
	}else{
		printf("KeyboardShift ");
		if(isShifted){
			printf("Off\r\n");
			isShifted = 0;
			restoreLED(&buttonPressBlinkLEDStore);
		}else{
			printf("On\r\n");
			isShifted = 1;
			storeAndSetLED(0, 80, 120, &buttonPressBlinkLEDStore);
		}
	}
	ButtonPressEvent evt;
	buildButtonPressEvent(&evt);
	if(addEvent(&evt)){ //This keeps us from repeating or otherwise responding to ourselves.
		if(myButton == BUTTON_L_CLICK){
			if(mouseBroadcastTask==NULL){
				mouseBroadcastTask = schedulePeriodicTask(MOUSE_RNB_BROADCAST_PERIOD, broadcastRnbData, NULL);
			}
		}
		if(isWired){
			wireTxButtonPress(evt.key);
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
			if( (uint8_t)(charPressed-97) < 26  ){
				charPressed -= 32;
			}
			evt.time = getTime();
			evt.key = charPressed;
			evt.src = getDropletID();			
			printf("PRESSED: ");
			printf(isprint(evt.key) ? "   '%c'\r\n" : "'\\%03hu'\r\n", evt.key);
			if(addEvent(&evt)){
				prepButtonPressMsg(&evt);	
			}
		}
		return 1;	
	}else{
		return 0;
	}
}

void prepMouseMoveMsg(BotPos* pos, DensePosCovar* covar){
	MouseMoveMsg msg;
	msg.pos = *pos;
	msg.covar = *covar;
	msg.time = getTime();
	msg.flag = MOUSE_MOVE_MSG_FLAG;
	MouseMoveMsgNode* msgNode = (MouseMoveMsgNode*)myMalloc(sizeof(MouseMoveMsgNode));
	msgNode->numTries = 0;
	msgNode->msg = msg;
	sendButtonPressMsg(msgNode);
}

void sendMouseMoveMsg(MouseMoveMsgNode* msgNode){
	if(irIsBusy(ALL_DIRS)){
		if( msgNode->numTries > 6 || (getTime() - msgNode->msg.time > 3000) ){
			myFree(msgNode);
		}else{
			scheduleTask(getExponentialBackoff(msgNode->numTries), (arg_func_t)sendButtonPressMsg, (void*)msgNode);
		}
		msgNode->numTries++;
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
		if(msgNode->numTries>6 || ((getTime() - (msgNode->msg).evt.time)>3000) ){
			myFree(msgNode);
		}else{
			uint32_t whenSend;
			if( (myButton!=BUTTON_L_CLICK) || (mouseBroadcastTask==NULL)){
				whenSend = getExponentialBackoff(msgNode->numTries);
			}else{
				whenSend = mouseBroadcastTask->scheduled_time;
				if(whenSend <= (getTime()+100)){
					whenSend += MOUSE_RNB_BROADCAST_PERIOD;
				}
				whenSend -= 100;		
			}
			scheduleTask(whenSend, (arg_func_t)sendButtonPressMsg, (void*)msgNode);
		}
		msgNode->numTries++;
	}else{
		irSend(ALL_DIRS, (char*)(&(msgNode->msg)), sizeof(ButtonPressMsg));
		myFree(msgNode);
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
	return ((k*16)+5);///20000000;
}
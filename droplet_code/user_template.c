#include "user_template.h"

void init(){
	if((LOCALIZATION_DUR)>=SLOT_LENGTH_MS){
		printf_P(PSTR("Error! Localization requires SLOT_LENGTH_MS to be greater than LOCALIZATION_DUR!\r\n"));
	}
	myRole = UNKNOWN;
	myKey = KEYBOARD_UNKNOWN;
	loopID = 0xFFFF;
	isShifted = 0;
	frameCount = 0;
	lastKeypress = 0;
	wireSleepTask = NULL;
	isWired = 0;
	for(uint8_t i=0;i<3;i++){
		wiredBlinkLEDStore[i] = 0;
		keypressBlinkLEDStore[i] = 0;
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
			useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
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
	}else if(IS_KEYPRESS_MSG(msgStruct)){
		handleKeypressMsg((KeypressMsg*)(msgStruct->msg));
	}
}

void handleKeypressMsg(KeypressMsg* msg){
	KeypressEvent* evt = &(msg->evt);
	if(addEvent(evt)){
		if(evt->key==KEYBOARD_SHIFT){
			isShifted = !isShifted;
			repeatKeypressMsg(msg);
		}else{
			if(isWired){
				wireTxKeypress(evt->key);
			}else{
				repeatKeypressMsg(msg);
			}
		}
	}
}



//This seems to take ~600us per sample.
#define NUM_CHECK_SAMPLES 100
inline static uint8_t targetMaxCount(void){
	if(getTime()<120000) return (uint8_t)(0.8*NUM_CHECK_SAMPLES); //Very high for the first two minutes.
	if(getTime()<240000) return (uint8_t)(0.6*NUM_CHECK_SAMPLES); //Less high for 2-4 minutes
	if(getTime()<480000) return (uint8_t)(0.5*NUM_CHECK_SAMPLES); //Down to 50 for 4-6 minutes.
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
	KeyboardKey resultKeys[(LARGEST_KEYBOARD_KEY+1)] = {0}; //according to internet, this initializes everything to 0.
	for(uint8_t i=0;i<NUM_CHECK_SAMPLES;i++){
		randNormSample[0] = randNorm(0,1);
		randNormSample[1] = randNorm(0,1);
		randNormSample[2] = randNorm(0,1);
		matrixTimesVector(&result, &diagSqrtEigValues, &randNormSample);
		resultPos.x = myPos.x + result[0];
		resultPos.y = myPos.y + result[1];
		resultKeys[getKeyFromPosition(&resultPos)]++;
	}
	uint8_t maxKeyCount = 0;
	KeyboardKey maxKey = KEYBOARD_UNKNOWN;
	for(uint8_t i=0;i<=LARGEST_KEYBOARD_KEY;i++){
		if(resultKeys[i]>maxKeyCount){
			maxKeyCount = resultKeys[i];
			maxKey = i;
		}
	}
	printf("       Max Key: '");
	printf(isprint(maxKey) ? "%c" : "\\%hu", maxKey);
	printf("' (%hu)\r\n", maxKeyCount);
	uint8_t secondMaxKeyCount = 0;
	KeyboardKey secondMaxKey = KEYBOARD_UNKNOWN;
	for(uint8_t i=0;i<=LARGEST_KEYBOARD_KEY;i++){
		if(i!=maxKey){
			if(resultKeys[i]>secondMaxKeyCount){
				secondMaxKeyCount = resultKeys[i];
				secondMaxKey = i;
			}
		}
	}
	printf("Second Max Key: '");
	printf(isprint(secondMaxKey) ? "%c" : "\\%hu", secondMaxKey);
	printf("' (%hu)\r\n", secondMaxKeyCount);		
	if(maxKeyCount>targetMaxCount() && maxKey!=KEYBOARD_UNKNOWN){
		myKey = maxKey;
		myRole = KEYBOARD;
		setRGB(0,0,15);
	} 
}



void restoreLED(volatile LEDStore* vals){
	setRGB((*vals)[0], (*vals)[1], (*vals)[2]);
}

void userMicInterrupt(){
	if( (getTime()-lastKeypress) < MIN_MULTIPRESS_DELAY){
		return;
	}
	lastKeypress = getTime();
	if(myKey!=KEYBOARD_SHIFT){
		storeLED(&keypressBlinkLEDStore);
		setRGB(0,80,120);
		scheduleTask(150, (arg_func_t)restoreLED, (void*)&keypressBlinkLEDStore);
		if(myRole == KEYBOARD){
			KeypressEvent evt;
			buildKeypressEvent(&evt);;
			if(addEvent(&evt)){ //This keeps us from repeating or otherwise responding to ourselves.
				if(isWired){
					wireTxKeypress(evt.key);
				}else{
					prepKeypressMsg(&evt);
				}
			}
		}else if(myRole == MOUSE){
			printf("Mouse\r\n");
		}else{
			printf("Role Unknown\r\n");
		}
	}else{
		printf("KeyboardShift ");
		if(isShifted){
			printf("Off\r\n");			
			isShifted = 0;
			restoreLED(&keypressBlinkLEDStore);
		}else{
			printf("On\r\n");
			isShifted = 1;
			storeLED(&keypressBlinkLEDStore);
			setRGB(0,80,120);
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
			KeypressEvent evt;
			if( (uint8_t)(charPressed-97) < 26  ){
				charPressed -= 32;
			}
			evt.time = getTime();
			evt.key = charPressed;
			evt.src = getDropletID();			
			printf("PRESSED: ");
			printf(isprint(evt.key) ? "   '%c'\r\n" : "'\\%03hu'\r\n", evt.key);
			if(addEvent(&evt)){
				prepKeypressMsg(&evt);	
			}
		}
		return 1;	
	}else{
		return 0;
	}
}

void prepKeypressMsg(KeypressEvent* evt){
	KeypressMsg msg;
	msg.evt = *evt;
	msg.flag = KEYPRESS_MSG_FLAG;
	KeypressMsgNode* msgNode = (KeypressMsgNode*)myMalloc(sizeof(KeypressMsgNode));
	msgNode->numTries = 0;
	msgNode->msg = msg;
	sendKeypressMsg(msgNode);
}

void sendKeypressMsg(KeypressMsgNode* msgNode){
	if(irIsBusy(ALL_DIRS)){
		if(msgNode->numTries>6 || ((getTime() - (msgNode->msg).evt.time)>5000) ){
			myFree(msgNode);
		}else{
			scheduleTask(getExponentialBackoff(msgNode->numTries), (arg_func_t)sendKeypressMsg, (void*)msgNode);
		}
		msgNode->numTries++;
	}else{
		irSend(ALL_DIRS, (char*)(&(msgNode->msg)), sizeof(KeypressMsg));
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
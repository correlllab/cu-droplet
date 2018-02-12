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
	frameStart = getTime();
	mySlot = getSlot(getDropletID());
	printf("mySlot: %u, frame_length: %lu\r\n\r\n", mySlot, FRAME_LENGTH_MS);
	if(POS_DEFINED(&myPos)){
		myRole = getRoleFromPosition(&myPos);
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
		}else if(myRole == KEYBOARD || myRole == MOUSE){
			setRGB(0,0,25);
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

//This seems to take 650us per sample.
#define NUM_CHECK_SAMPLES 100
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
			//Really, resultPos.o should be myPos.o, converted to radians and added to result[2].
			//But, getKeyFromPosition doesn't care about the bot's orientation, so that's skipped.
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
		if(maxKeyCount>=60 && maxKey!=KEYBOARD_UNKNOWN){
			myKey = maxKey;
			myRole = KEYBOARD;
		}
}

void userMicInterrupt(){
	if( (getTime()-lastKeypress) < MIN_MULTIPRESS_DELAY){
		return;
	}
	lastKeypress = getTime();
	if(myKey!=KEYBOARD_SHIFT){
		setRGB(0,80,120);
		scheduleTask(150, ledOff, NULL);
		if(myRole == KEYBOARD){
			KeypressEvent evt;
			buildKeypressEvent(&evt);;
			if(addEvent(&evt)){ //This keeps us from repeating or otherwise responding to ourselves.
				if(isWired){
					wireTxKeypress(evt.key);
				}else{
					sendKeypressMsg(&evt);
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
			setRGB(0,0,0);
		}else{
			printf("On\r\n");
			isShifted = 1;
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
	}else{
		return 0;
	}
}

void wireSleep(){
	isWired = 0;
}
#include "user_template.h"

void init(){
	enableMicInterrupt();
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
			if(POS_DEFINED(&myPos)){
				printf_P(PSTR("\tMy Pos: {%d, %d, %d}\r\n"), myPos.x, myPos.y, myPos.o);
				printPosCovar(&myPosCovar);
				printf("\r\n");
				checkPosition();
			}
		}
		if(myRole == UNKNOWN){
			uint8_t newR = 0, newG = 0, newB = 0;
			getPosColor(&newR, &newG, &newB);
			setRGB(newR, newG, newB);
		}else if(myRole == KEYBOARD){
			setRGB(0,0,0);
		}else if(myRole == MOUSE){
			setRGB(0,0,10);
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
	int8_t result = addEvent(msg);
	if(result==0){
		//We've already seen this event, so nothing else to do.
		return;
	}else if(result == -1){
		printf("Event queue error!\r\n");
		return;
	}
	if(msg->key==KEYBOARD_SHIFT){
		isShifted = !isShifted;
		repeatKeypressMsg(msg);
	}else{
		if(isWired){
			setRGB(50,0,0);
			scheduleTask(150, ledOff, NULL);
			printf("KeyboardKey ");	
			if(isprint(myKey)){
				printf("'%c'\r\n", (char)myKey);
			}else{
				printf("'\\%hu'\r\n", (uint8_t)myKey);
			}
		}else{
			repeatKeypressMsg(msg);
		}
	}
}

void repeatKeypressMsg(KeypressMsg* msg){
	irSend(ALL_DIRS, (char*)msg, sizeof(KeypressMsg));
}

#define NUM_CHECK_SAMPLES 10
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
		KeyboardKey resultKeys[NUM_CHECK_SAMPLES];
		uint8_t resultCounts[NUM_CHECK_SAMPLES];
		for(uint8_t i=0;i<NUM_CHECK_SAMPLES;i++){
			resultKeys[i] = KEYBOARD_UNKNOWN;
			resultCounts[i] = 0;
		}
		KeyboardKey resultKey;
		uint8_t resultFound;
		for(uint8_t i=0;i<NUM_CHECK_SAMPLES;i++){
			randNormSample[0] = randNorm(0,1);
			randNormSample[1] = randNorm(0,1);
			randNormSample[2] = randNorm(0,1);
			matrixTimesVector(&result, &diagSqrtEigValues, &randNormSample);
			//Really, resultPos.o should be myPos.o, converted to radians and added to result[2].
			//But, getKeyFromPosition doesn't care about the bot's orientation, so that's skipped.
			resultPos.x = myPos.x + result[0];
			resultPos.y = myPos.y + result[1];
			resultKey = getKeyFromPosition(&resultPos);
			
			//This next bit tabulates the distributes of resulting keys.
			resultFound = 0;
			for(uint8_t j=0;j<=i;j++){
				if(resultKeys[j]==resultKey){
					resultFound=1;
					resultCounts[j]++;
					break;
				}
			}
			if(!resultFound){
				resultKeys[i] = resultKey;
				resultCounts[i]++;
			}
			
		}
		uint8_t maxCount = 0;
		resultKey = KEYBOARD_UNKNOWN;
		printf("check_position_results= {");
		for(uint8_t i=0;i<NUM_CHECK_SAMPLES;i++){
			if(resultCounts[i]>0){
				if(resultCounts[i]>maxCount){
					resultKey = resultKeys[i];
					maxCount = resultCounts[i];
				}else if(resultCounts[i]==maxCount){
					resultKey = KEYBOARD_UNKNOWN;
				}
				printf( isprint(resultKeys[i]) ? "{%c, %hu}" : "{%hu, %hu}", resultKeys[i], resultCounts[i]);
				if(i!=(NUM_CHECK_SAMPLES-1)){
					printf(",");
				}
			}
		}
		printf("};\r\n");
		if(maxCount>3 && resultKey!=KEYBOARD_UNKNOWN){
			myKey = resultKey;
			myRole = KEYBOARD;
		}
}

void sendKeypressMsg(){
	KeypressMsg msg;
	msg.time = getTime();
	msg.src	 = getDropletID();
	msg.key  = myKey;
	msg.key = ( isShifted && isalpha(myKey) ) ? (myKey-32) : myKey; //convert to uppercase if appropriate.
	msg.flag = KEYPRESS_MSG_FLAG;
	addEvent(&msg);//This keeps us from repeating or otherwise responding to ourselves.
	irSend(ALL_DIRS, (char*)(&msg), sizeof(KeypressMsg));
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
			printf("PRESSED: ");
			if(isprint(myKey)){
				printf("'%c'\r\n", (char)myKey);
			}else{
				printf("'\\%hu'\r\n", (uint8_t)myKey);
			}
			sendKeypressMsg();
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

void wireSleep(){
	isWired = 0;
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
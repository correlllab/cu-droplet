#include "user_template.h"

uint16_t micData[2100];

void init(){
	enableMicInterrupt();
	if((LOCALIZATION_DUR)>=SLOT_LENGTH_MS){
		printf_P(PSTR("Error! Localization requires SLOT_LENGTH_MS to be greater than LOCALIZATION_DUR!\r\n"));
	}
	myRole = UNKNOWN;
	myKey = KEYBOARD_UNKNOWN;
	loopID = 0xFFFF;
	frameCount = 0;
	frameStart = getTime();
	mySlot = getSlot(getDropletID());
	printf("mySlot: %u, frame_length: %lu\r\n\r\n", mySlot, FRAME_LENGTH_MS);
	if(POS_DEFINED(&myPos)){
		myRole = getRoleFromPosition(&myPos);
	}
	//delayMS(1000);
	//setRGB(200,0,0);
	//delayMS(1000);
	//setRGB(200,200,0);
	//delayMS(1000);
	//setRGB(0,200,0);
	//delayMS(1000);
	//setRGB(15,15,15);
//
	//micRecording(micData, 2100, 2800);
	//setRGB(0,0,200);
	//printRecording(micData, 2100);
	//setRGB(0,0,0);
	//delayMS(20000);
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
				//if position error is low enough, call getRoleFromPosition.
			}
		}
		//if(myRole == UNKNOWN){
			//uint8_t newR = 0, newG = 0, newB = 0;
			//getPosColor(&newR, &newG, &newB);
			//setRGB(newR, newG, newB);
		//}else if(myRole == KEYBOARD){
			//setRGB(0,0,0);
		//}else if(myRole == MOUSE){
			//setRGB(0,0,10);
		//}
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

void handleMsg(irMsg* msg_struct){
	if(((BotMeasMsg*)(msg_struct->msg))->flag==BOT_MEAS_MSG_FLAG && msg_struct->length==sizeof(BotMeasMsg)){
		printf("Got BotMeasMsg.\r\n");
		if(myRole == MOUSE){
			//TODO: Write a version of handleBotMeasMsg as if no prior information about my position.	
		}else{
			handleBotMeasMsg((BotMeasMsg*)(msg_struct->msg), msg_struct->senderID);
		}
	}
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
		KeyboardKey resultKey = KEYBOARD_UNKNOWN;
		for(uint8_t i=0;i<NUM_CHECK_SAMPLES;i++){
			randNormSample[0] = randNorm(0,1);
			randNormSample[1] = randNorm(0,1);
			randNormSample[2] = randNorm(0,1);
			matrixTimesVector(&result, &diagSqrtEigValues, &randNormSample);
			//Really, resultPos.o should be myPos.o, converted to radians and added to result[2].
			//But, getKeyFromPosition doesn't care about the bot's orientation, so that's skipped.
			resultPos.x = myPos.x + result[0];
			resultPos.y = myPos.y + result[1];
			resultKey = resultKey ^ getKeyFromPosition(&resultPos);
		}
		printf("Result Key: %hu\r\n", resultKey);
}

void userMicInterrupt(){
	if(myKey!=KEYBOARD_SHIFT){
		setRGB(0,80,120);
		scheduleTask(150, ledOff, NULL);
		if(myRole == KEYBOARD){
			printf("Keyboard Key ");
			if(isprint(myKey)){
				printf("'%c'\r\n", (char)myKey);
			}else{
				printf("'\\%hu'\r\n", (uint8_t)myKey);
			}
		}else if(myRole == MOUSE){
			printf("Mouse\r\n");
		}else{
			printf("Role Unknown\r\n");
		}
	}else{
		printf("Keyboard Shift ");
		if(getRedLED() || getGreenLED() || getBlueLED()){
			printf("Off\r\n");
			setRGB(0,0,0);
		}else{
			printf("On\r\n");
			setRGB(0,80,120);
		}
	}
}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){	
	//return 0;
//}
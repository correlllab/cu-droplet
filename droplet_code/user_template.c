#include "user_template.h"



void init(){
	if((LOCALIZATION_DUR)>=SLOT_LENGTH_MS){
		printf_P(PSTR("Error! Localization requires SLOT_LENGTH_MS to be greater than LOCALIZATION_DUR!\r\n"));
	}
	loopID = 0xFFFF;
	frameCount = 0;
	frameStart=getTime();
	isBlinking = 0;
	
	largestFrameGap = 2;
	for(uint8_t i=0;i<93;i++){
		lastFrameSeen[i] = 0;
	}
	
	mySlot = getSlot(getDropletID());
	printf("mySlot: %u, frame_length: %lu\r\n\r\n", mySlot, FRAME_LENGTH_MS);
}

void loop(){
	uint32_t frameTime = getTime()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
	}
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID==mySlot){
			
			//if(frameCount%2){
				//blinkLED(64, 0, 0, 250);
				//if(!seedFlag && POS_DEFINED(&myPos)){
					//prepPosMsg(getDropletID(), myPos);
				//}
			//}else{
				blinkLED(0, 64, 0, 250);
				broadcastRnbData();
			//}
		}else if(loopID==SLOTS_PER_FRAME-1){
			blinkLED(40,40,40,500);
			//if(frameCount%2){
				//printf("Comm frame ended. (%lu)\r\n", largestFrameGap);
				//if(largestFrameGap>2){
					//largestFrameGap--;
				//}
			//}else{			
				printf_P(PSTR("\nID: %04X T: %lu\r\n"), getDropletID(), getTime());
				printf("Update frame ended.\r\n");
				//if(POS_DEFINED(&myPos)){
					//printf_P(PSTR("{\r\n\t%lu,\r\n\t{%d, %d, %d},\r\n"), getTime(), myPos.x, myPos.y, myPos.o);
					//printPosCovar(&myPosCovar);
					//printf("},\r\n");
				//}
			//}
		}
	}
	delayMS(LOOP_DELAY_MS);
}

void prepPosMsg(id_t id, BotPos pos){
	PosMsgNode* msgNode = (PosMsgNode*)myMalloc(sizeof(PosMsgNode));
	msgNode->numTries = 0;
	(msgNode->msg).flag = POS_MSG_FLAG;
	(msgNode->msg).pos = pos;
	(msgNode->msg).id = id;
	sendPosMsg(msgNode);
}

void sendPosMsg(PosMsgNode* msgNode){
	if(irIsBusy(ALL_DIRS)){
		if(msgNode->numTries>6){
			myFree(msgNode);
		}else{
			scheduleTask(getExponentialBackoff(msgNode->numTries), (arg_func_t)sendPosMsg, msgNode);
		}
		msgNode->numTries++;
	}else{
		blinkLED(16,0,0,150);
		irSend(ALL_DIRS, (char*)(&(msgNode->msg)), sizeof(PosMsg));
		myFree(msgNode);
	}
}

void handlePosMsg(PosMsg* msg){
	if(msg->id == getDropletID() || !(frameCount%2)){
		return;
	}
	uint8_t idOrd = getDropletOrd(msg->id);
	if(!lastFrameSeen[idOrd]){
		printf("(*POS*) {\"%04X\", {% 6d, % 6d, % 6d}},\r\n", msg->id, (msg->pos).x, (msg->pos).y, (msg->pos).o);
		prepPosMsg(msg->id, msg->pos);
	}else if(lastFrameSeen[idOrd]!=frameCount){
		printf("(*POS*) {\"%04X\", {% 6d, % 6d, % 6d}},\r\n", msg->id, (msg->pos).x, (msg->pos).y, (msg->pos).o);		
		uint32_t frameGap = frameCount - lastFrameSeen[idOrd];
		if(frameGap>=largestFrameGap){
			largestFrameGap = frameGap+1;
		}
		uint32_t threshold = (0xFFFFFFFF/largestFrameGap)*frameGap;
		uint32_t val = randQuad();
		//printf("Val: %f, Threshold: %f, frameGap: %lu, largestFrameGap: %lu\r\n",(val*1.0)/(0xFFFFFFFF*1.0),(threshold*1.0)/(0xFFFFFFFF*1.0), frameGap, largestFrameGap);
		if(val<threshold){
			prepPosMsg(msg->id, msg->pos);
		}
	}
	lastFrameSeen[idOrd] = frameCount;
	
}

void handleMeas(Rnb* meas){
	blinkLED(0,24,0,150);
	RNB_DEBUG_PRINT("\t(RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", meas->id, meas->range, meas->bearing, meas->heading);
	useRNBmeas(meas);
}

void handleMsg(irMsg* msgStruct){
	blinkLED(0,0,40,150);
	if(IS_BOT_MEAS_MSG(msgStruct)){
		handleBotMeasMsg((BotMeasMsg*)(msgStruct->msg), msgStruct->senderID);
	}else if(IS_POS_MSG(msgStruct)){
		handlePosMsg((PosMsg*)(msgStruct->msg));
	}
}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){	
	//return 0;
//}
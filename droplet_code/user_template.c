#include "user_template.h"

void init(){
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
			broadcastRnbData();
		}else if(loopID==SLOTS_PER_FRAME-1){ //This is the end-of-frame slot.
			//if(POS_DEFINED(&myPos)){
				//printf_P(PSTR("{\r\n\t%lu,\r\n\t{%d, %d, %d},\r\n"), getTime(), myPos.x, myPos.y, myPos.o);
				//printPosCovar(&myPosCovar);
				//printf("},\r\n");
			//}
		}
		if(myRole == UNKNOWN){
			uint8_t newR = 0, newG = 0, newB = 0;
			getPosColor(&newR, &newG, &newB);
			setRGB(newR, newG, newB);
		}else if(myRole == KEYBOARD){
			setRGB(0,0,0);
		}else if(myRole == MOUSE){
			setRGB(100,100,100);
		}
	}
	if(rnb_updated){
		RNB_DEBUG_PRINT("\t(RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", id, range, bearing, heading);
		if(myRole != MOUSE){
			useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		}
		rnb_updated=0;
	}
	delayMS(LOOP_DELAY_MS);
}

void handleMsg(irMsg* msg_struct){
	if(((BotMeasMsg*)(msg_struct->msg))->flag==BOT_MEAS_MSG_FLAG && msg_struct->length==sizeof(BotMeasMsg)){
		if(myRole == MOUSE){
			//TODO: Write a version of handleBotMeasMsg as if no prior information about my position.	
		}else{
			handleBotMeasMsg((BotMeasMsg*)(msg_struct->msg), msg_struct->senderID);
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
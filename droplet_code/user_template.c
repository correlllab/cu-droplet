#include "user_template.h"

void init(){
	if((LOCALIZATION_DUR)>=SLOT_LENGTH_MS){
		printf_P(PSTR("Error! Localization requires SLOT_LENGTH_MS to be greater than LOCALIZATION_DUR!\r\n"));
	}
	loopID = 0xFFFF;
	frameCount = 0;
	frameStart=getTime();
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
			broadcastRnbData();
		}else if(loopID==SLOTS_PER_FRAME-1){
			//printf_P(PSTR("\nID: %04X T: %lu "), get_droplet_id(), get_time());
			if(POS_DEFINED(&myPos)){
				printf_P(PSTR("{\r\n\t%lu,\r\n\t{%d, %d, %d},\r\n"), getTime(), myPos.x, myPos.y, myPos.o);
				printPosCovar(&myPosCovar);
				printf("},\r\n");
			}
		}
		uint8_t newR = 0, newG = 0, newB = 0;
		getPosColor(&newR, &newG, &newB);
		setRGB(newR, newG, newB);
	}
	delayMS(LOOP_DELAY_MS);
}

void handleMeas(Rnb* meas){
	RNB_DEBUG_PRINT("\t(RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", meas->id, meas->range, meas->bearing, meas->heading);
	useRNBmeas(meas);
}

void handleMsg(irMsg* msgStruct){
	if(((BotMeasMsg*)(msgStruct->msg))->flag==BOT_MEAS_MSG_FLAG && msgStruct->length==sizeof(BotMeasMsg)){
		handleBotMeasMsg((BotMeasMsg*)(msgStruct->msg), msgStruct->senderID);
	}
}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){	
	//return 0;
//}
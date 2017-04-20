#include "swarm_calibrate.h"
#define STEPS_PER_TEN 50


void init(){
	if((LOCALIZATION_DUR)>=SLOT_LENGTH_MS){
		printf_P(PSTR("Error! Localization requires SLOT_LENGTH_MS to be greater than LOCALIZATION_DUR!\r\n"));
	}
	loopID = 0xFFFF;
	frameCount = 0;
	frameStart=get_time();
	mySlot = getSlot(get_droplet_id());
	printf("mySlot: %u, frame_length: %lu\r\n\r\n", mySlot, FRAME_LENGTH_MS);
	//set_all_ir_powers(200);
}

void loop(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
	}
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID==mySlot){
			broadcast_rnb_data();
		}else if(loopID==SLOTS_PER_FRAME-1){
			printf_P(PSTR("\nID: %04X T: %lu "), get_droplet_id(), get_time());
			if(POS_DEFINED(&myPos)){
				printf_P(PSTR("\tMy Pos: {%d, %d, %d}\r\n"), myPos.x, myPos.y, myPos.o);
				printPosCovar(&myPosCovar);
				printf("\r\n");
			}else{
				printf("\r\n\r\n");
			}
		}
		uint8_t newR = 0, newG = 0, newB = 0;
		getPosColor(&newR, &newG, &newB);
		set_rgb(newR, newG, newB);
	}
	if(rnb_updated){
		RNB_DEBUG_PRINT("\t(RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		Vector estPos; //relPos will have orientation in radians.
		Vector me = {myPos.x, myPos.y, deg_to_rad(myPos.o)};
			
		// use other pos for relative pos, getOtherPos 
		BotPos* otherPos = getOtherPos(last_good_rnb.id);
		
		// store these intermittently
		relativePosition(last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading, otherPos, &estPos);
		
		useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		rnb_updated=0;
	}
	delay_ms(LOOP_DELAY_MS);
}

void sendBotPosMsg(){
	ir_send(ALL_DIRS, &myPos, sizeof(BotPos));
}

void handle_msg(ir_msg* msg_struct){
	if(((BotMeasMsg*)(msg_struct->msg))->flag==BOT_MEAS_MSG_FLAG && msg_struct->length==sizeof(BotMeasMsg)){
		handleBotMeasMsg((BotMeasMsg*)(msg_struct->msg), msg_struct->sender_ID);
	}else if(msg_struct->length==sizeof(BotPos)){
		BotPos* pos = (BotPos*)(msg_struct->msg);
		id_t id = msg_struct->sender_ID;
		//store this in some array of bot positions.
		//how many bot positions should I keep around?
		//if less than total possible, how handle conflicts?
	}
}

// https://github.com/correlllab/cu-droplet/blob/9e98d62e3bcffc1b98e1fb7daf6f1c3f27f572d5/droplet_code/droplet_programs/positions.c
void getOtherPos(){
	//Need other bot positions for reference when doing  my relative position
}










///*
 //*	the function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. it should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){
	//return 0;
//}

//uint8_t chooseDir()
//{
	//// this function will be used to decide the next direction to go for calibration
	//return 7;
//}


// adjusting motors will occur at the end of our sampling period.
// sample period consists of
// 1: change state to the movement mode
// 2: pick a direction to move in
// 3: for a period of time/a number of steps, move steps, intermittently collect a point from rnb data (average, nearest, or most recent?)
// 4: 
//void adjustMotors(uint16_t stepsTaken, uint8_t direction, struct sampleArr){
	//// from the last element in the struct we can get our resting position
	//// 
//}

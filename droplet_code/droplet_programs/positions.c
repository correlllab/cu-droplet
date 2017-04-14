//#include "droplet_programs/positions.h"
//
//void init(){
	//if((LOCALIZATION_DUR)>=SLOT_LENGTH_MS){
		//printf_P(PSTR("Error! Localization requires SLOT_LENGTH_MS to be greater than LOCALIZATION_DUR!\r\n"));
	//}
	//loopID = 0xFFFF;
	//frameCount = 0;
	//frameStart=get_time();
	//mySlot = getSlot(get_droplet_id());
	//printf("mySlot: %u, frame_length: %lu\r\n\r\n", mySlot, FRAME_LENGTH_MS);
	//set_all_ir_powers(200);
//}
//
//void loop(){
	//uint32_t frameTime = get_time()-frameStart;
	//if(frameTime>FRAME_LENGTH_MS){
		//frameTime = frameTime - FRAME_LENGTH_MS;
		//frameStart += FRAME_LENGTH_MS;
		//frameCount++;
	//}
	//if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		//loopID = frameTime/SLOT_LENGTH_MS;
		//if(loopID==mySlot){
			//broadcast_rnb_data();
		//}else if(loopID==SLOTS_PER_FRAME-1){
			//printf_P(PSTR("\nID: %04X T: %lu "), get_droplet_id(), get_time());
			//if(POS_DEFINED(&myPos)){
				//printf_P(PSTR("\tMy Pos: {%d, %d, %d}\r\n"), myPos.x, myPos.y, myPos.o);
				//printPosCovar(&myPosCovar);
				//printf("\r\n");
			//}else{
				//printf("\r\n\r\n");
			//}
		//}
		//uint8_t newR = 0, newG = 0, newB = 0;
		//getPosColor(&newR, &newG, &newB);
		//set_rgb(newR, newG, newB);
	//}
	//if(rnb_updated){
		//RNB_DEBUG_PRINT("\t(RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", id, range, bearing, heading);
		//useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		//rnb_updated=0;
	//}
	//delay_ms(LOOP_DELAY_MS);
//}
//
//void handle_msg(ir_msg* msg_struct){
	//if(((BotMeasMsg*)(msg_struct->msg))->flag==BOT_MEAS_MSG_FLAG && msg_struct->length==sizeof(BotMeasMsg)){
		//handleBotMeasMsg((BotMeasMsg*)(msg_struct->msg), msg_struct->sender_ID);
	//}
//}
//
/////*
 ////*	The function below is optional - commenting it in can be useful for debugging if you want to query
 ////*	user variables over a serial connection.
 ////*/
////uint8_t user_handle_command(char* command_word, char* command_args){	
	////return 0;
////}

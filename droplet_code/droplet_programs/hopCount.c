//#include "droplet_programs/hopCount.h"
//#include "stdio.h"
//#include "stdarg.h"
//
//#define DEBUG_MODE
//
//#ifdef DEBUG_MODE
//#define DEBUG_PRINT(format, ...) printf(format, ##__VA_ARGS__)
//#else
//#define DEBUG_PRINT(format, ...)
//#endif
//
///*
 //*  TODO
 //*   --Read papers: 
 //*			-*- http://dl.acm.org/citation.cfm?id=938995
 //*			-*- 
 //*   --Infer location?
 //*/ 
//
//
//void init(){
	////enable_sync_blink(0);
	//lastLoop = 0;
	//frameCount = 0;
	//randomThresh = 0;
	//clearTrackedHops();
	//initializeSeeds();
	//msgPower = 128;
	//myMsgLoop = (get_droplet_id()%(SLOTS_PER_FRAME-1));;
	//printf("MsgLoop: %d\r\n", myMsgLoop);
	//frameStart = get_time_wrapper();
	//prevColorSlot = SLOTS_PER_FRAME+1;
//}
//
 //uint32_t __attribute__ ((noinline)) get_time_wrapper(){
	//return get_time();
//}
//
//void clearTrackedHops(){
	//myX=NAN;
	//myY=NAN;
	//posColorMode = 0;
	//for(uint8_t i=0;i<NUM_SEEDS;i++){
		//trackedHops[i].time = 0;
		//trackedHops[i].id= 0;
		//trackedHops[i].x = 0;
		//trackedHops[i].y = 0;
		//trackedHops[i].flag = 0;
		//trackedHops[i].marker = 255;
		//trackedHops[i].hopCount=255;
	//}
//}
//
//void initializeSeeds(){
	//seed = 0;
	//for(uint8_t i=0; i<NUM_SEEDS ; i++){
		//if(get_droplet_id()==SEED_IDS[i]){
			//seed = 1;
			//trackedHops[i].time = get_time();			
			//trackedHops[i].id = get_droplet_id();
			//trackedHops[i].x = SEED_X[i];
			//trackedHops[i].y = SEED_Y[i];
			//trackedHops[i].flag = 1;
			//trackedHops[i].marker = 0;
			//trackedHops[i].hopCount = 0;
			//myX = SEED_X[i];
			//myY = SEED_Y[i];
			////trackedHops[i].flag += 2;
			//break;
		//}
	//}
//}
//
//
//void loop(){
	////printf("%lu\t%u\r\n",get_time(),TCE0.CNT);
	//uint32_t frameTime = get_time()-frameStart;
	//if(frameTime>FRAME_LENGTH_MS){
		//frameTime -= FRAME_LENGTH_MS;
		//frameStart = get_time()-frameTime;
		//frameCount++;
	//}
	//uint8_t loopID = (SLOTS_PER_FRAME*frameTime)/FRAME_LENGTH_MS;
	//if(loopID!=lastLoop){
		//if(loopID==myMsgLoop){
			//propagateAsNecessary();
			//if(seed && (rand_byte()<randomThresh)){ //chance for seeds to start a new echo.
				//for(uint8_t i=0;i<NUM_SEEDS;i++){
					//if(trackedHops[i].id==get_droplet_id()){
						//trackedHops[i].flag = 1;
						//break;
					//}
				//}
			//}
		//}
		//if(loopID==SLOTS_PER_FRAME-1){
			//DEBUG_PRINT("%lu [%lu] (%-6.1f, %-6.1f)\r\n", frameCount, get_time(), myX, myY);
			//for(uint8_t i=0;i<NUM_SEEDS;i++){
				//DEBUG_PRINT("\t%04X: %hu", trackedHops[i].id, trackedHops[i].hopCount);
				//if(trackedHops[i].flag){
					//DEBUG_PRINT(" [%lu]\r\n", trackedHops[i].time);
				//}else{
					//DEBUG_PRINT("\r\n");
				//}
			//}
			//
		//}
		//setColor(loopID);
	//}
//
	//
	//lastLoop = loopID;
	//delay_ms(SLOT_LENGTH_MS/10);
//}
//
//void setColor(uint8_t loopID){
	//if(posColorMode){
		//setPosColor();
	//}else{
		//setSeedDataColor(loopID);
	//}
//}
//
//void setPosColor(){
	//if(isnan(myX)||isnan(myY)){
		//set_rgb(50,50,50);
	//}else{
		//uint8_t rColor = (uint8_t)((255*myX)/X_MAX_DIM);
		//uint8_t gColor = (uint8_t)((255*myY)/Y_MAX_DIM);
		//set_rgb(rColor,gColor,0);
	//}
//}
//
//void setSeedDataColor(uint8_t loopID){
	//uint8_t colorSlot = (loopID*COLOR_SLOTS)/SLOTS_PER_FRAME;
	//if(prevColorSlot==colorSlot){
		//if(colorSlot==0){ //go dark to indicate start of frame.
			//set_rgb(0,0,0);
		//}else{ //set color for each seed based on hops.
			//Hop* slottedHop = &(trackedHops[colorSlot-1]);
			//uint16_t hopCount = slottedHop->hopCount;
			//if(slottedHop->id && hopCount!=255){ //255 is used to indicate an unseen ID after a clear.
				//uint16_t hue = (360*(hopCount%NUM_HUES))/NUM_HUES;
				//set_hsv(hue, 255, 255);
			//}else{ //no seed for this slot, so go dim.
				//set_rgb(50,50,50);
			//}
		//}
	//}else{ //at the start of each slot, flash white.
		//set_rgb(255,255,255);
	//}
	//prevColorSlot = colorSlot;
//}
//
///* 
 //* Returns number of empty seed slots, or '0' if we're sending a mesmsage. 
 //* Either way, '0' means we definitely don't want to become a seed.
 //*/
//uint8_t propagateAsNecessary(){
	//uint32_t oldest = get_time();
	//uint8_t oldestIdx = 0xFF;
	//uint8_t numEmpty = 0;
	//for(uint8_t i=0;i<NUM_SEEDS;i++){
		//if(!trackedHops[i].id){
			//numEmpty++;
		//}else{
			//if(trackedHops[i].flag==1){
				//if(trackedHops[i].time<oldest){
					//oldest = trackedHops[i].time;
					//oldestIdx = i;
				//}
			//}else{
				//trackedHops[i].flag--;
			//}
		//}
	//}
	//if(oldestIdx==0xFF){
		//return numEmpty;
	//}else{
		//propagateHop(oldestIdx);
		//return 0;
	//}
//}
//
//void sendHopMsg(uint16_t id, int16_t x, int16_t y, uint8_t idx, uint8_t marker, uint8_t hC){
	//HopMsg msg;
	//msg.flag = HOP_MSG_FLAG;
	//msg.id = id;
	//msg.x = x;
	//msg.y = y;
	//msg.marker = marker;
	//msg.hopCount = (hC&0x3F)|(idx<<6);
	//uint16_t prevPower = get_all_ir_powers();
	//set_all_ir_powers(msgPower);
	//ir_send(ALL_DIRS, (char*)(&msg), sizeof(HopMsg));
	//set_all_ir_powers(prevPower);
//}
//
//void addHop(uint16_t id, int16_t x, int16_t y, uint8_t idx, uint8_t marker, uint8_t hopCount){
	//if(trackedHops[idx].id==0){
		//trackedHops[idx].id = id;
		//trackedHops[idx].hopCount = hopCount;
		//trackedHops[idx].x = x;
		//trackedHops[idx].y = y;
		//trackedHops[idx].time = get_time();
		//trackedHops[idx].marker = marker;
		//trackedHops[idx].flag = 1;
	//}else if(trackedHops[idx].id==id){
		//if(hopCount < trackedHops[idx].hopCount){
			//if(marker <= trackedHops[idx].marker){
				//trackedHops[idx].hopCount = hopCount;
			//}
			//trackedHops[idx].time = get_time();
			//trackedHops[idx].flag = 1;
			//if(trackedHops[idx].x!=x || trackedHops[idx].y!=y){
				//printf("ERROR\tID found, but coords don't match? %d!=%d || %d!=%d\r\n",trackedHops[idx].x, x, trackedHops[idx].y, y);
			//}
		//}
	//}else{
		//printf("ERROR! trackedHops collision!\r\n");
	//}
//}
//
/////*
 ////* Returns: 
 ////*		0 if the addition was unsuccessful,
 ////*		1 if successful and ID was already present.
 ////*		2 if successful and ID was not already present.
 ////*/
////uint8_t addHop(uint16_t id, int16_t x, int16_t y, uint8_t marker, uint8_t idx, uint8_t hopCount){
	//////printf("Adding hop.\r\n");	
	////uint8_t idIdx = 0xFF;
	////uint8_t emptyIdx = 0xFF;
	////for(uint8_t i=0 ; i<NUM_SEEDS ; i++){
		////if(emptyIdx==0xFF && trackedHops[i].id == 0){
			////emptyIdx = i;
		////}
		////if(trackedHops[i].id == id){
			////idIdx = i;
			////break;
		////}
	////}
	////
	////if(idIdx!=0xFF){ //ID found.
		////if(hopCount < trackedHops[idIdx].hopCount){
			////if(marker <= trackedHops[idIdx].marker){
				////trackedHops[idIdx].hopCount = hopCount;
			////}
			////trackedHops[idIdx].time = get_time();
			////trackedHops[idIdx].flag = 1;
			////if(trackedHops[idIdx].x!=x || trackedHops[idIdx].y!=y){
				////printf("ERROR\tID found, but coords don't match? %d!=%d || %d!=%d\r\n",trackedHops[idIdx].x, x, trackedHops[idIdx].y, y);
			////}
		////}
		////return 1;
	////}else{ //ID not found
		////if(emptyIdx==0xFF){
			////return 0;
		////}else{
			////trackedHops[emptyIdx].id = id;
			////trackedHops[emptyIdx].hopCount = hopCount;
			////trackedHops[emptyIdx].x = x;
			////trackedHops[emptyIdx].y = y;
			////trackedHops[emptyIdx].time = get_time();
			////trackedHops[emptyIdx].marker = marker;
			////trackedHops[emptyIdx].flag = 1;
			////sortTrackedHops();					
			////return 2;
		////}
	////}	
////}
//
////void sortTrackedHops(){
	////int8_t n = NUM_SEEDS;
	////int8_t newN;
	////do{
		////newN = 0;
		////for(int8_t i=1;i<n;i++){
			////if(trackedHops[i-1].id > trackedHops[i].id){
				////swapTwoTrackedHops(i-1,i);
				////newN=i;
			////}
		////}
		////n = newN;
	////}while(n);
////}
//
//void handle_msg(ir_msg* msg_struct){
	//HopMsg* msg=((HopMsg*)(msg_struct->msg));
	////printf("msg!\r\n\tid:\t%04X\r\n\tx:\t%d\r\n\ty:\t%d\r\n\tflag:\t%c\r\n\thC:\t%hu\r\n",msg->id, msg->x, msg->y, msg->flag, msg->hopCount);
	//if(msg->flag==HOP_MSG_FLAG){
		//if(msg->id==0){
			//return;
		//}	
		//uint8_t hopCount = (msg->hopCount)&0x3F;
		//uint8_t idx		 = (msg->hopCount)>>6;
		//addHop(msg->id, msg->x, msg->y, idx, msg->marker, hopCount);
		//if(!seed && countSeeds()==NUM_SEEDS){
			//updatePos();
		//}
	//}
//}
//
//
//void updatePos(){
	//#if NUM_SEEDS!=4
		//asdfs //I want this to break loudly if NUM_SEEDS isn't 4. 
	//#endif
	//float xMin=1000.0, xMax=-1000.0, yMin=1000.0, yMax=-1000.0;
	//float sD[4];
	//for(uint8_t i=0;i<NUM_SEEDS;i++){
		//sD[i]= ((float)(trackedHops[i].hopCount*MM_PER_HOP))/M_SQRT2;
		//if(trackedHops[i].x<xMin){
			//xMin = trackedHops[i].x;
		//}
		//if(trackedHops[i].x>xMax){
			//xMax = trackedHops[i].x;
		//}
		//if(trackedHops[i].y<yMin){
			//yMin = trackedHops[i].y;
		//}
		//if(trackedHops[i].y>yMax){
			//yMax = trackedHops[i].y;
		//}
	//}
	//myX = (sD[0]+sD[1]-sD[2]-sD[3]+xMax-xMin)/2+xMin;
	//myY = (sD[0]-sD[1]+sD[2]-sD[3]+yMax-yMin)/2+yMin;
//}
//
///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args)
//{
	//if(strcmp_P(command_word,PSTR("ir_p"))==0){
		//uint16_t power = atoi(command_args);
		//if(power>256){
			//printf("IR Power must be 0 to 256 inclusive. Yes, 256 is max. Not 255.\r\n");
		//}else{
			//printf("Setting IR Power to %u\r\n", power);
			//msgPower = power;
		//}
		//return 1;
	//}else if(strcmp_P(command_word,PSTR("clear"))==0){
		//for(uint8_t i=0;i<NUM_SEEDS;i++){
			//if(trackedHops[i].id==get_droplet_id()){
				//trackedHops[i].hopCount = 0;
				//trackedHops[i].flag = 1;
				//trackedHops[i].marker = 0;
			//}else{
				//trackedHops[i].marker = 255;
				//trackedHops[i].hopCount = 255;
				//trackedHops[i].flag = 0;
			//}
		//}
		//return 1;
	//}else if(strcmp_P(command_word,PSTR("pcm"))==0){
		//if(command_args[0]=='e'){
			//posColorMode = 1;
		//}else if(command_args[0]=='d'){
			//posColorMode = 0;
		//}else{
			//posColorMode = !posColorMode;
		//}
		//if(posColorMode){
			//printf("posColorMode enabled.\r\n");
		//}else{
			//printf("posColorMode disabled.\r\n");
		//}
		//return 1;
	//}else if(strcmp_P(command_word,PSTR("thresh"))==0){
		//uint8_t thresh = atoi(command_args);
		//printf("Setting randomThresh to %hu.\r\n",thresh);
		//randomThresh = thresh;
		//return 1;
	//}
	//return 0;
//}
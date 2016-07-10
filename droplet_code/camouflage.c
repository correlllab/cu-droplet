/*
* camouflage.c
* For Camouflage Project
* Created: 5/25/2016, re-factored on 6/30/2016
* Author : Yang Li
* Description:
1. https://bitbucket.org/dominicverity/turing-pattern-generator
2. A local activator-inhibitor model of vertebrate skin patterns

Algorithmic Description:

INITIALIZATION:
Project one of the three patterns on Droplets

READ COLOR AND GRAPH PHASE:
Each frame, droplet senses the color on it, and store it
If its slot: Broadcasting to find out the four neighbors and store the information
If its slot: Share the neighbor information to have a bigger neighbor information 
	for Pattern formation
Last idle slot: figure out the big picture

GRADIENT PHASE:
Each frame, if its slot: broadcast to its neighbors where it is (distance and bearing) "my position"
Each frame, if not its slot: try to receive messages (color) from other Droplet, and store 
	them in four_Direction_Array
Each frame, during the last idle slot: Compute the gradients in each direction and update 
	the guess of pattern

CONSENSUS PHASE:
Each frame, if its slot: broadcast to its neighbors "my pattern"
Each frame, if not its slot: try to receive messages (guessed pattern and degree) from other 
	Droplet, and store them
Each frame, during the last idle slot: Try to make consensus on which the most possible 
	pattern to apply

TURING PATTERN PHASE:
__Generate a pattern based on Young's model__
Each frame, if its slot: broadcast it current "color"
Each frame, it not its slot: try to receive messages ("color") from droplets in the 
	two circles, and store them
Each frame, during the last idle slot: try to sum up the DCs and change the "color" accordingly


STRUCT:
Four neighbor struct and message  // FOR gradient
Eight neighbor struct // FOR consensus
Twelve neighbor struct // FOR formation

Neighbor Index document:
//////////////////////////////
		8
	7	0	4
11	3	X	1	9
	6	2	5
		10
//////////////////////////////

Patterns:
0: horizontal
1: vertical

*/

#include "camouflage.h"

/*
* Any code in this function will be run once, when the robot starts.
*/
void init()
{
	// me - initialization
	me.dropletId = get_droplet_id();
	me.mySlot = me.dropletId%(SLOTS_PER_FRAME-1);
	//me.mySlot = get_droplet_ord(me.dropletId)-100; // For AUDIO_DROPLET
	//me.mySlot = get_droplet_order_camouflage(me.dropletId);
	me.myDegree = 1;
	me.turing_color = rand_byte()%2;
	for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++)
	{
		me.neighborIds[i] = 0;
	}
	
	me.rgb[0] = 0;
	me.rgb[1] = 0;
	me.rgb[2] = 0;
	/********************* other variables ************************/ 
	// fourDr: use as a message to send out
	myFourDr.dropletId = me.dropletId;
	myFourDr.flag = NEIGHBOR_MSG_FLAG;
	for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++)
	{
		myFourDr.Ids[i] = 0;
		myFourDr.gotMsg_flags[i] = 0;
		
		fourNeiInfo[i].dropletId = 0;
		fourNeiRGB[i].dropletId = 0;
	}
	
	for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++){
		eightNeiPattern[i].dropletId = 0;
	}
	// global
	frameCount = 1;
	loopID = 0xFFFF;
	phase = 0;
	
	frameStart = get_time();
	
	printf("Initializing Camouflage Project. mySlot is %003d\r\n", me.mySlot);		
}

/*
* The code in this function will be called repeatedly, as fast as it can execute.
*/
void loop()
{
	switch (phase){
		case 0: preparePhase(); break;
		case 1: gradientPhase(); break;
		case 2: consensusPhase(); break;
		case 3: turingPhase(); break;
		default: 
		break;
	}
}

// different phases send different kind of message
void sendNeiMsg(){
	neighborMsg msg = myFourDr;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(neighborMsg));
}

void sendRGBMsg(){
	if((50<me.rgb[0]&& me.rgb[0]<400) &&
	(50<me.rgb[1]&& me.rgb[1]<400) &&
	(50<me.rgb[2]&& me.rgb[2]<400) ){
		rgbMsg msg;
		msg.flag = RGB_MSG_FLAG;
		msg.dropletId = me.dropletId;
		for (uint8_t i=0; i<3; i++){
			msg.rgb[i] = me.rgb[i];
		}
		ir_send(ALL_DIRS, (char*)(&msg), sizeof(rgbMsg));
	}
}

void sendPatternMsg(){
	patternMsg msg;
	msg.flag = PATTERN_MSG_FLAG;
	msg.dropletId = me.dropletId;
	msg.degree = me.myDegree;
	msg.pattern = (uint16_t)(me.myPattern_f*65535.0f);
	
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(patternMsg));
}

void sendTuringMsg(){
	turingMsg msg;
	msg.flag = TURING_MSG_FLAG;
	msg.dropletId = me.dropletId;
	msg.color = me.turing_color;
	
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(turingMsg));
}

/*
* After each pass through loop(), the robot checks for all messages it has
* received, and calls this function once for each message.
*/
void handle_msg(ir_msg* msg_struct)
{
	switch (phase){
		case 0: handle_neighbor_msg(msg_struct); break;
		case 1: handle_rgb_msg(msg_struct); break;
		case 2: handle_pattern_msg(msg_struct); break;
		case 3: handle_turing_msg(msg_struct); break;
		default: break;
	}
}

void handle_neighbor_msg(ir_msg* msg_struct){
	neighborMsg* msg = (neighborMsg**)(msg_struct->msg);
	if(msg->flag == NEIGHBOR_MSG_FLAG){
		for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++)
		{
			if (msg->dropletId == myFourDr.Ids[i])
			{
				myFourDr.gotMsg_flags[i] = 1;
				fourNeiInfo[i] = (*msg);
				break;
			}
		}
	}	
}

void handle_rgb_msg(ir_msg* msg_struct){
	rgbMsg* msg = (rgbMsg**)(msg_struct->msg);
	if(msg->flag == RGB_MSG_FLAG){
		for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++)
		{
			if (msg->dropletId == myFourDr.Ids[i])
			{
				fourNeiRGB[i] = (*msg);
				break;
			}
		}
	}
}

void handle_pattern_msg(ir_msg* msg_struct){
	patternMsg* msg = (patternMsg**)(msg_struct->msg);
	if(msg->flag == PATTERN_MSG_FLAG){
		for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++)
		{
			if (msg->dropletId == me.neighborIds[i])
			{
				eightNeiPattern[i] = (*msg);
				break;
			}
		}
	}
}

void handle_turing_msg(ir_msg* msg_struct){
	turingMsg* msg = (turingMsg**)(msg_struct->msg);
	if(msg->flag == TURING_MSG_FLAG){
		for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++)
		{
			if (msg->dropletId == me.neighborIds[i])
			{
				twelveNeiTuring[i] = (*msg);
				break;
			}
		}
	}
}

void preparePhase(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime > FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		printf("\r\n[Prepare Phase] Current frame No. is %u\r\n", frameCount);
	}
	
	/*****************  code here executes once per slot.   ******************/
	// The first condition is to ensure this
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID == me.mySlot){
			/* Do stuff. send messages. do rnb broadcast. */
			//set_rgb(255, 0, 0);
			broadcast_rnb_data();
			sendNeiMsg();
			
			// read colors
			int16_t red_led;
			int16_t green_led;
			int16_t blue_led;
			
			get_rgb(&red_led,&green_led,&blue_led);

			// store to print
			allRGB[frameCount-1].rgb[0] = red_led;
			allRGB[frameCount-1].rgb[1] = green_led;
			allRGB[frameCount-1].rgb[2] = blue_led;
			red_array[frameCount-1] = red_led;
			green_array[frameCount-1] = green_led;
			blue_array[frameCount-1] = blue_led;
			//printf("X[%04X] R: %d G: %d B: %d (ori)\r\n",
			//me.dropletId,red_led, green_led, blue_led);

			//set_rgb(me.rgb[0], me.rgb[1], me.rgb[2]);
		}
		else if(loopID == SLOTS_PER_FRAME-1){
			/* End of frame. Do some final processing here */
			set_rgb(0, 255, 0);
			extendNeighbors();
							
			if (frameCount<NUM_PREPARE) {
				if(TEST_PREPARE){
					printf("X[%04X] R: %d G: %d B: %d\r\n",
					me.dropletId, allRGB[frameCount-1].rgb[0], 
					allRGB[frameCount-1].rgb[1], allRGB[frameCount-1].rgb[2]);
				}
				frameCount++; 
			}
			else {
				phase++; frameCount = 1;
				me.rgb[0] = meas_find_median(&red_array, NUM_PREPARE);
				me.rgb[1] = meas_find_median(&green_array, NUM_PREPARE);
				me.rgb[2] = meas_find_median(&blue_array, NUM_PREPARE);
				
				if(TEST_PREPARE){
					for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++){
						if (me.neighborIds[i] != 0)
						{
							printf("%u-[%04X]\r\n", i, me.neighborIds[i]);
						}
					}
					printf("\r\n");
				}
			}
		}
		else{
			led_off();
		}
	}
	
	/*****************  code here executes once per loop.   ******************/
	if(rnb_updated){
		if(last_good_rnb.conf > 1.0){
			float bearing = last_good_rnb.bearing;
			float range = last_good_rnb.range;
			uint16_t id_number = last_good_rnb.id_number;
			if(!TEST_PREPARE){
				printf("ID: %04X Rang: %0.4f Bearing: %0.4f \r\n",
				id_number, range, bearing*180.0/M_PI);
			}
			
			if(fabs(bearing-M_PI_2) < PI_12 && range < 8.0f) {// left
				myFourDr.Ids[3] = id_number;
				if(TEST_PREPARE){
					printf("L - ID: %04X Rang: %0.4f Bearing: %0.4f \r\n",
					id_number, range, bearing*180.0/M_PI);
				}
			}
			else if(fabs(bearing+M_PI_2) < PI_12 && range < 8.0f) {// right
				myFourDr.Ids[1] = id_number;
				if(TEST_PREPARE){
					printf("R - ID: %04X Rang: %0.4f Bearing: %0.4f \r\n",
					id_number, range, bearing*180.0/M_PI);
				}
			}
			else if(fabs(bearing-0.0f) < PI_12 && range < 8.0f) {// top
				myFourDr.Ids[0] = id_number;
				if(TEST_PREPARE){
					printf("T - ID: %04X Rang: %0.4f Bearing: %0.4f \r\n",
					id_number, range, bearing*180.0/M_PI);
				}
			}
			else if( (fabs(bearing-M_PI) < PI_12 || fabs(bearing+M_PI) < PI_12 )  && range < 8.0f ) {// bottom
				myFourDr.Ids[2] = id_number;
				if(TEST_PREPARE){
					printf("B - ID: %04X Rang: %0.4f Bearing: %0.4f \r\n",
					id_number, range, bearing*180.0/M_PI);
				}
			}
		}

		rnb_updated = 0;
	}

	/* Define the duration of loop */
	delay_ms(LOOP_DELAY_MS);
}


// Inside of __preparePhase__
// get information about neighbor's neighbor
// extend the neighbor graph based on them
void extendNeighbors(){
	// top part - Done!
	if (myFourDr.gotMsg_flags[0] != 0){
		me.neighborIds[0] = myFourDr.Ids[0];
		if (fourNeiInfo[0].Ids[0] != 0){
			me.neighborIds[8] = fourNeiInfo[0].Ids[0];
		}
		if (fourNeiInfo[0].Ids[1] != 0){
			me.neighborIds[4] = fourNeiInfo[0].Ids[1];
		}
		if (fourNeiInfo[0].Ids[3] != 0){
			me.neighborIds[7] = fourNeiInfo[0].Ids[3];
		}				
	}
	
	// left part - Done!
	if (myFourDr.gotMsg_flags[1] != 0){
		me.neighborIds[1] = myFourDr.Ids[1];
		if (fourNeiInfo[1].Ids[0] != 0){
			me.neighborIds[4] = fourNeiInfo[1].Ids[0];
		}
		if (fourNeiInfo[1].Ids[1] != 0){
			me.neighborIds[9] = fourNeiInfo[1].Ids[1];
		}
		if (fourNeiInfo[1].Ids[2] != 0){
			me.neighborIds[5] = fourNeiInfo[1].Ids[2];
		}
	}	

	// bottom part - Done!
	if (myFourDr.gotMsg_flags[2] != 0){
		me.neighborIds[2] = myFourDr.Ids[2];
		if (fourNeiInfo[2].Ids[1] != 0){
			me.neighborIds[5] = fourNeiInfo[2].Ids[1];
		}
		if (fourNeiInfo[2].Ids[2] != 0){
			me.neighborIds[10] = fourNeiInfo[2].Ids[2];
		}
		if (fourNeiInfo[2].Ids[3] != 0){
			me.neighborIds[6] = fourNeiInfo[2].Ids[3];
		}
	}
	
	// right part - Done!
	if (myFourDr.gotMsg_flags[3] != 0){
		me.neighborIds[3] = myFourDr.Ids[3];
		if (fourNeiInfo[3].Ids[0] != 0){
			me.neighborIds[7] = fourNeiInfo[3].Ids[0];
		}
		if (fourNeiInfo[3].Ids[2] != 0){
			me.neighborIds[6] = fourNeiInfo[3].Ids[2];
		}
		if (fourNeiInfo[3].Ids[3] != 0){
			me.neighborIds[11] = fourNeiInfo[3].Ids[3];
		}
	}
	
	// recalculate Degree for consensus phase
	me.myDegree = 1;
	for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++)
	{
		if (me.neighborIds[i] != 0) me.myDegree++;
	}
}

void gradientPhase(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime > FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		printf("\r\n[Gradient Phase] Current frame No. is %u\r\n", frameCount);
	}
	
	/*****************  code here executes once per slot.   ******************/
	// The first condition is to ensure this
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID == me.mySlot){
			/* Do stuff. send messages. do rnb broadcast. */
			set_rgb(255, 0, 0);
			sendRGBMsg();
		}
		else if(loopID == SLOTS_PER_FRAME-1){
			/* End of frame. Do some final processing here */
			set_rgb(0, 0, 255);
			
			if (TEST_GRADIENT){
				printf("X[%04X] R: %d G: %d B: %d\r\n",
				me.dropletId, me.rgb[0], me.rgb[1], me.rgb[2]);
				for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
					printf("%u[%04X] R: %d G: %d B: %d\r\n",
					i, fourNeiRGB[i].dropletId, fourNeiRGB[i].rgb[0], 
					fourNeiRGB[i].rgb[1], fourNeiRGB[i].rgb[2]);
				}
			}
			// At the end of this phase, decide the pattern
			// based on the information received
			// check if go to another phase		
			if (frameCount<NUM_GRADIENT) {frameCount++; }
			else {
				phase++; frameCount = 1;
				decidePattern();
				allPattern[0] = me.myPattern_f;
			}
		}
		else{
			led_off();
		}
	}
	
	/*****************  code here executes once per loop.   ******************/
	
	/* Define the duration of loop */
	delay_ms(LOOP_DELAY_MS);		
}

// Inside of __gradientPhase__
// Apply filters on both directions: horizontal and vertical
// Note: ONLY ONCE
void decidePattern(){
	// Compute two gradients and decide which pattern
	// At this point, only use Red channel
	uint8_t channel = 1;
	if((50<me.rgb[0]&& me.rgb[0]<400) &&
	(50<me.rgb[1]&& me.rgb[1]<400) &&
	(50<me.rgb[2]&& me.rgb[2]<400) ){  // ignore and set to 0.5f
		for(uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
			if (fourNeiRGB[i].dropletId == 0){
				for (uint8_t j=0; j<3; j++){
					fourNeiRGB[i].rgb[j] = me.rgb[j];
				}
			}
		}
		uint16_t diff_row = 0;
		uint16_t diff_col = 0;
		for (uint8_t channel = 1; channel <=2; channel++){
			diff_row += abs(2*me.rgb[channel] - fourNeiRGB[1].rgb[channel] - fourNeiRGB[3].rgb[channel]);
			diff_col += abs(2*me.rgb[channel] - fourNeiRGB[0].rgb[channel] - fourNeiRGB[2].rgb[channel]);
		}
		// Decide which pattern to be
		if(diff_row < diff_col){ // row less than col: horizontal
			me.myPattern_f = 0.0f;
		}
		else if (diff_col < diff_row){ // col less than row: vertical
			me.myPattern_f = 1.0f;
		}
		else{ // for the corner ones
			me.myPattern_f = 0.5f;
		}
	}
	else{
		me.myPattern_f = 0.5f;
	}
	if (TEST_GRADIENT){
		printf("X[%04X] RGB: %03u\r\n", me.dropletId, me.rgb[channel]);
		for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
			if (fourNeiRGB[i].dropletId != 0){
				printf("%u[%04X] RGB: %03u\r\n", i, fourNeiRGB[i].dropletId,
				fourNeiRGB[i].rgb[channel]);
			}
		}
	}
}

void consensusPhase(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime > FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		printf("\r\n[Consensus Phase] Current frame No. is %u\r\n", frameCount);
	}
	
	/*****************  code here executes once per slot.   ******************/
	// The first condition is to ensure this
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID == me.mySlot){
			/* Do stuff. send messages. do rnb broadcast. */
			set_rgb(255, 0, 0);
			sendPatternMsg();
		}
		else if(loopID == SLOTS_PER_FRAME-1){
			/* End of frame. Do some final processing here */
			set_rgb(255, 255, 255);			
			weightedAverage();
			
			if (frameCount<NUM_CONSENSUS) {
				allPattern[frameCount] = me.myPattern_f;
				frameCount++; }
			else {
				phase++; frameCount = 1;
				if (me.turing_color == 0){
					set_rgb(255, 255, 255);
				}
				else{
					set_rgb(255, 0, 0);
				}
			}
		}
		else{
			led_off();
		}
	}
	
	/*****************  code here executes once per loop.   ******************/
		
	/* Define the duration of loop */
	delay_ms(LOOP_DELAY_MS);
}

// Why small degree has larger weight?
void weightedAverage(){
	float wi;
	float wc;
	float pattern = 0.0f;
	uint8_t maxDegree;
	wc = 1.0f;
	for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++){
		if (eightNeiPattern[i].dropletId != 0){
			maxDegree = me.myDegree;
			if (maxDegree < eightNeiPattern[i].degree){
				maxDegree = eightNeiPattern[i].degree;
			}
			wi = 1.0f/(1.0f+(float)maxDegree);
			wc -= wi;
			pattern += wi*(float)eightNeiPattern[i].pattern/65535.0f;		
		}
	}
	pattern += wc*me.myPattern_f;
	
	if (TEST_CONSENSUS){
		for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++){
			if (eightNeiPattern[i].dropletId != 0){
				printf("%u[%04X] Degree: %u Pattern: %.4f\r\n", i, eightNeiPattern[i].dropletId,
				eightNeiPattern[i].degree, (float)eightNeiPattern[i].pattern/65535.0f);
			}
		}
		printf("\r\nPre-pattern: %0.4f Cur-pattern: %0.4f\r\n", me.myPattern_f, pattern);
	}
	me.myPattern_f = pattern;
}

void turingPhase(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime > FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		printf("\r\n[Turing Phase] Current frame No. is %u\r\n", frameCount);
	}
	
	/*****************  code here executes once per slot.   ******************/
	// The first condition is to ensure this
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID == me.mySlot){
			/* Do stuff. send messages. do rnb broadcast. */
			sendTuringMsg();
		}
		else if(loopID == SLOTS_PER_FRAME-1){
			/* End of frame. Do some final processing here */
			if (fabs(me.myPattern_f-0.5f) > TURING_RANDOM){
				changeColor();
			}
			else{
				if (me.turing_color == 0){
					set_rgb(255, 255, 255);
				}
				else{
					set_rgb(255, 0, 0);
				}
			}
			
			
			if (frameCount<NUM_TURING) {frameCount ++; }
			else {
				phase ++; // set it back to 0 to restart !!!
				frameCount = 1; 
				printf("\r\nAll Done!\r\n"); 
				displayMenu();
			}
		}
		else{
			//led_off();
		}
	}
	
	/*****************  code here executes once per loop.   ******************/
		
	/* Define the duration of loop */
	delay_ms(LOOP_DELAY_MS);
}

// 
void changeColor(){
	uint8_t na = 0;
	uint8_t ni = 0;
	float ss = 0.0f;
	if (me.turing_color == 1){
		na += 1;
		ni += 1;
	}
	if (me.myPattern_f < 0.5f-TURING_RANDOM) {	// pattern = 0: horizontal
		for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
			if (twelveNeiTuring[hIndex[i]].color == 1){
				na += 1;
			}
		}
		for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++){
			if (twelveNeiTuring[i].color == 1){
				ni += 1;
			}
		}
	}
	else if (me.myPattern_f > 0.5f+TURING_RANDOM){	// pattern = 1: vertical
		for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
			if (twelveNeiTuring[vIndex[i]].color == 1){
				na += 1;
			}
		}
		for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++){
			if (twelveNeiTuring[i].color == 1){
				ni += 1;
			}
		}		
	}
	
	ss += (float)na - (float)ni*TURING_F;
	if (ss > 0){
		me.turing_color = 1;
	} 
	else if (ss < 0){
		me.turing_color = 0;
	}

	if (me.turing_color == 0){
		set_rgb(255, 255, 255);
	}
	else{
		set_rgb(255, 0, 0);
	}	
	
	if (TEST_TURING) {
		for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++) {
			if (twelveNeiTuring[i].dropletId != 0) {
				printf("%u[%04X] Color: %u\r\n", i, 
				twelveNeiTuring[i].dropletId, twelveNeiTuring[i].color);
			}
		}
	}
	
}

uint8_t user_handle_command(char* command_word, char* command_args){
	if(strcmp(command_word, "pn")==0){ 
		printns();
	}
	if(strcmp(command_word, "pp")==0){
		printprob();
	}
	if(strcmp(command_word, "pt")==0){
		printturing();
	}	
	if(strcmp(command_word, "pa")==0){
		printprob();
		printturing();
		printns();
		//printrgbs();
		//printrgbs_ordered();
		printfrgb();
	}
			
	return 0;	
}

void displayMenu(){
	printf("pn: print neighbors' ID\r\n"); // 
	printf("pp: print all pattern probs\r\n"); 
	printf("pt: print neighbors' turing colors\r\n"); // 
	printf("pa: print all above info\r\n"); 
}

void printns(){
	printf("\r\nPrint neighbors' ID\r\n");
	printf("X[%04X]\r\n", me.dropletId);
	for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++)
	{
		printf("%d [%04X]\r\n", i, me.neighborIds[i]);
	}
}

void printrgbs(){
	printf("\r\nPrint all rgbs read\r\n");
	for (uint8_t i=0; i<NUM_PREPARE; i++)
	{
		printf("%u: %d %d %d\r\n", i, allRGB[i].rgb[0], allRGB[i].rgb[1], allRGB[i].rgb[2]);
	}
}

void printrgbs_ordered(){
	printf("\r\nPrint all rgbs read (ordered)\r\n");
	for (uint8_t i=0; i<NUM_PREPARE; i++)
	{
		printf("%u: %d %d %d\r\n", i, red_array[i], green_array[i], blue_array[i]);
	}
}

void printfrgb(){
	printf("\r\nPrint final rgb and neighbors\r\n"); 
	printf("X: %d %d %d\r\n", me.rgb[0],
	me.rgb[1], me.rgb[2]);
	for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++)
	{
		printf("%u: %d %d %d\r\n", i, fourNeiRGB[i].rgb[0], 
		fourNeiRGB[i].rgb[1], fourNeiRGB[i].rgb[2]);
	}	
}

void printprob(){
	printf("\r\nPrint all pattern probs\r\n"); 
	for (uint8_t i=0; i<NUM_CONSENSUS; i++)
	{
		printf("%0.6f\r\n", allPattern[i]);
	}
}

void printturing(){
	printf("\r\nPrint final turing colors\r\n"); 
	printf("X[%04X] Color: %u\r\n", me.dropletId, me.turing_color);
	for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++) {
		if (twelveNeiTuring[i].dropletId != 0) {
			printf("%u[%04X] Color: %u\r\n", i,
			twelveNeiTuring[i].dropletId, twelveNeiTuring[i].color);
		}
		else{
			printf("%u[    ] Color: #\r\n", i);
		}
	}	
}
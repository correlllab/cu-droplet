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
void init(){
	me.dropletId = get_droplet_id();
	me.mySlot = me.dropletId%(SLOTS_PER_FRAME-1);
	//me.mySlot = get_droplet_ord(me.dropletId)-100; // For AUDIO_DROPLET
	me.mySlot = (get_droplet_id()%(SLOTS_PER_FRAME-1));
	me.myDegree = 1;
	me.turing_color = rand_byte()%2;
	for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++){
		me.neighborIds[i] = 0;
		twelveNeiTuring[i].color = 0;
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
	
	for (uint8_t i=0; i<NUM_PATTERNS; i++){
		me.myPattern_f[i] = 0.0f;
	}
	measRoot = NULL;
	lastMeasAdded = measRoot;

	// global
	frameCount = 0;
	loopID = 0xFFFF;
	phase = 0;
	printf("Initializing Camouflage Project. mySlot is %03hu\r\n", me.mySlot);		
	frameStart = get_time();
}

/*
* The code in this function will be called repeatedly, as fast as it can execute.
*/
void loop(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime > FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
	}
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID == me.mySlot){
			delay_ms(10);
			switch (phase){
				case Prepare:	prepareSlot();		break;
				case Gradient:	sendRGBMsg();		break;
				case Consensus:	sendPatternMsg();	break;
				case Turing:	sendTuringMsg();	break;
			}
			delay_ms(10);
		}else if(loopID == SLOTS_PER_FRAME-1){
			switch (phase){
				case Prepare:	prepareEOP();		break;
				case Gradient:	gradientEOP();		break;
				case Consensus: consensusEOP();		break;
				case Turing:	turingEOP();		break;
			}
			printf("End of frame %lu.\r\n", frameCount);
		}
		setColor();
	}
	
	/*****************  code here executes once per loop.   ******************/
	if(rnb_updated){
		if(phase==Prepare){
			handleRNB();
		}
		rnb_updated = 0;
	}

	/* Define the duration of loop */
	delay_ms(LOOP_DELAY_MS);
}

void setColor(){
	if(loopID == me.mySlot){
		switch (phase){
			case Prepare: led_off(); break;
			case Gradient: set_rgb(255, 0, 0); break;
			case Consensus: set_rgb(255, 0, 0); break;
			case Turing: /*Do nothing.*/ break;
		}
	}else if(loopID == SLOTS_PER_FRAME-1){
		switch (phase){
			case Prepare: set_rgb(0, 255, 0); break;
			case Gradient: set_rgb(0, 0, 255); break;
			case Consensus:
				if(me.turing_color){
					set_rgb(255, 0, 0);
				}else{
					set_rgb(255,255,255);
				}
				break;
			case Turing: changeColor(); break;
		}
	}else{
		if(phase!=Turing){
			led_off();
		}
	}
}

void handleRNB(){
	uint8_t conf = (uint8_t)(sqrt(last_good_rnb.conf+1.0)+0.5);
	if(conf>2 && last_good_rnb.range < 8.0){
		RnbNode* tmp = (RnbNode*)myMalloc(sizeof(RnbNode));
		tmp->bearing = rad_to_deg(last_good_rnb.bearing);
		tmp->range = 10*last_good_rnb.range;
		tmp->id = last_good_rnb.id_number;
		tmp->conf = conf;
		tmp->next = NULL;
		lastMeasAdded->next = tmp;
		lastMeasAdded = tmp;

		printf("ID: %04X Range: %4d Bearing: % 4d\r\n", tmp->id, tmp->range, tmp->bearing);
	}
}

// different phases send different kind of message
void sendNeiMsg(){
	ir_send(ALL_DIRS, (char*)(&myFourDr), sizeof(neighborMsg));
}

void sendRGBMsg(){
	if((50<me.rgb[0] && me.rgb[0]<400) && (50<me.rgb[1] && me.rgb[1]<400) && (50<me.rgb[2] && me.rgb[2]<400)){
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
	for (uint8_t i=0; i<NUM_PATTERNS; i++){
		msg.pattern_f[i] = me.myPattern_f[i];
	}
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
		case 0: handle_neighbor_msg((neighborMsg*)msg_struct->msg); break;
		case 1: handle_rgb_msg((rgbMsg*)msg_struct->msg); break;
		case 2: handle_pattern_msg((patternMsg*)msg_struct->msg); break;
		case 3: handle_turing_msg((turingMsg*)msg_struct->msg); break;
		default: break;
	}
}

void handle_neighbor_msg(neighborMsg* msg){
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

void handle_rgb_msg(rgbMsg* msg){
	if(msg->flag == RGB_MSG_FLAG){
		for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
			if (msg->dropletId == myFourDr.Ids[i]){
				fourNeiRGB[i] = (*msg);
				break;
			}
		}
	}
}

void handle_pattern_msg(patternMsg* msg){
	if(msg->flag == PATTERN_MSG_FLAG){
		for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++){
			if (msg->dropletId == me.neighborIds[i]){
				eightNeiPattern[i] = (*msg);
				break;
			}
		}
	}
}

void handle_turing_msg(turingMsg* msg){
	if(msg->flag == TURING_MSG_FLAG){
		for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++){
			if (msg->dropletId == me.neighborIds[i]){
				twelveNeiTuring[i] = (*msg);
				break;
			}
		}
	}
}

void prepareSlot(){
	/* Do stuff. send messages. do rnb broadcast. */
	//set_rgb(255, 0, 0);
	broadcast_rnb_data();
	delay_ms(10);
	sendNeiMsg();

	// read colors
	int16_t red, green, blue;

	get_rgb(&red, &green, &blue);

	// store to print
	allRGB[frameCount].rgb[0] = red;
	allRGB[frameCount].rgb[1] = green;
	allRGB[frameCount].rgb[2] = blue;
	red_array[frameCount] = red;
	green_array[frameCount] = green;
	blue_array[frameCount] = blue;
	//printf("X[%04X] R: %d G: %d B: %d (ori)\r\n",	//me.dropletId,red_led, green_led, blue_led);
}

void prepareEOP(){
	/* End of frame. Do some final processing here */
	extendNeighbors();
	if (frameCount<NUM_PREPARE) {
		if(TEST_PREPARE){
			printf("X[%04X] R: %d G: %d B: %d\r\n", me.dropletId, allRGB[frameCount].rgb[0], allRGB[frameCount].rgb[1], allRGB[frameCount].rgb[2]);
		}
	}else{
		id_t maxIDs[NUM_NEIGHBOR_4] = {0,0,0,0};
		uint8_t maxConfs[NUM_NEIGHBOR_4] = {0,0,0,0};
		RnbNode* tmp;
		while(measRoot!=NULL){
			tmp	= measRoot;
			if(abs(tmp->bearing-90) <= NEIGHBOR_ANGLE_THRESH){// left
				if(tmp->conf>maxConfs[3]){
					maxIDs[3] = tmp->id;
					maxConfs[3] = tmp->conf;
				}
			}else if(abs(tmp->bearing+90) <= NEIGHBOR_ANGLE_THRESH) {// right
				if(tmp->conf>maxConfs[1]){
					maxIDs[1] = tmp->id;
					maxConfs[1] = tmp->conf;
				}
			}else if( abs(tmp->bearing) <= NEIGHBOR_ANGLE_THRESH) {// top
				if(tmp->conf>maxConfs[0]){
					maxIDs[0] = tmp->id;
					maxConfs[0] = tmp->conf;
				}
			}else if( ( abs(tmp->bearing-180) <= NEIGHBOR_ANGLE_THRESH || abs(tmp->bearing+180) <= NEIGHBOR_ANGLE_THRESH )) {// bottom
				if(tmp->conf>maxConfs[2]){
					maxIDs[2] = tmp->id;
					maxConfs[2] = tmp->conf;
				}
			}
			measRoot = tmp->next;
			myFree(tmp);
		}
		printf("Neighbors: ");
		for(uint8_t i=0;i<NUM_NEIGHBOR_4;i++){
			myFourDr.Ids[i] = maxIDs[i];
			if(myFourDr.Ids[i]!=0){
				printf("%04X ", myFourDr.Ids[i]);
			}else{
				printf("---- ");
			}
		}
		printf("\r\n");


		me.rgb[0] = meas_find_median(red_array, NUM_PREPARE);
		me.rgb[1] = meas_find_median(green_array, NUM_PREPARE);
		me.rgb[2] = meas_find_median(blue_array, NUM_PREPARE);
	
		if(TEST_PREPARE){
			for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++){
				if (me.neighborIds[i] != 0){
					printf("%hu-[%04X]\r\n", i, me.neighborIds[i]);
				}
			}
			printf("\r\n");
		}
		phase++;
		frameCount = 0;
	}
}

void gradientEOP(){
	/* End of frame. Do some final processing here */
	if (TEST_GRADIENT){
		printf("X[%04X] R: %d G: %d B: %d\r\n", me.dropletId, me.rgb[0], me.rgb[1], me.rgb[2]);
		for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
			printf("%hu[%04X] R: %d G: %d B: %d\r\n", i, fourNeiRGB[i].dropletId, fourNeiRGB[i].rgb[0], fourNeiRGB[i].rgb[1], fourNeiRGB[i].rgb[2]);
		}
	}
	// At the end of this phase, decide the pattern
	// based on the information received
	// check if go to another phase
	if(frameCount>=NUM_GRADIENT){
		decidePattern();
		for (uint8_t i=0; i<NUM_PATTERNS; i++){
			allPattern[0].pattern_f[i] = me.myPattern_f[i];
		}
		phase++;
		frameCount = 0;
	}
}

void consensusEOP(){
	weightedAverage();
	if (frameCount<NUM_CONSENSUS) {
		for (uint8_t i=0; i<NUM_PATTERNS; i++){
			allPattern[frameCount].pattern_f[i] = me.myPattern_f[i];
		}
	}else{
		phase++;
		frameCount = 0;
	}
}

void turingEOP(){
	/* End of frame. Do some final processing here */
	changeColor();
	if (frameCount>=NUM_TURING){
		printf("\r\nAll Done!\r\n");
		displayMenu();
		frameCount = 0;
	}
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
	for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++){
		if (me.neighborIds[i] != 0) me.myDegree++;
	}
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
		
		//uint16_t diff_row = 0;
		//uint16_t diff_col = 0;
		//for (uint8_t channel = 0; channel <3; channel++){
			//diff_row += abs(me.rgb[channel] - fourNeiRGB[1].rgb[channel]) + abs(me.rgb[channel]- fourNeiRGB[3].rgb[channel]);
			//diff_col += abs(me.rgb[channel] - fourNeiRGB[0].rgb[channel]) + abs(me.rgb[channel]- fourNeiRGB[2].rgb[channel]);
		//}
		
		float diff_row = 0;
		float diff_col = 0;
		float grays[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
		
		for (uint8_t j=0; j<NUM_NEIGHBOR_4; j++){
			for (uint8_t i=0; i<3; i++){
				grays[j] += (float)fourNeiRGB[j].rgb[i]*rgb_weights[i];
			}			
		}

		for (uint8_t i=0; i<3; i++){
			grays[4] += (float)me.rgb[i]*rgb_weights[i];
		}
					
		diff_row = fabs(grays[4]-grays[1] + grays[4]-grays[3]);
			
		diff_col = fabs(grays[4]-grays[0] + grays[4]-grays[2]);
		
		printf("Px: %0.4f Py: %0.4f\r\n", diff_row, diff_col);
		
		// Decide which pattern to be
		if(diff_col - diff_row > (float)threshold_mottled){ // row less than col: horizontal
			me.myPattern_f[0] = 1.0f;
		}
		else if (diff_row - diff_col > (float)threshold_mottled){ // col less than row: vertical
			me.myPattern_f[1] = 1.0f;
		}
		else{ // for the corner ones
			me.myPattern_f[2] = 1.0f;
		}
	}
	else{
		for (uint8_t i=0; i<NUM_PATTERNS; i++){
			me.myPattern_f[i] = 0.333333333f;
		}	
	}
	if (TEST_GRADIENT){
		printf("X[%04X] RGB: %03d\r\n", me.dropletId, me.rgb[channel]);
		for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
			if (fourNeiRGB[i].dropletId != 0){
				printf("%hu[%04X] RGB: %03d\r\n", i, fourNeiRGB[i].dropletId, fourNeiRGB[i].rgb[channel]);
			}
		}
	}
}

// Why small degree has larger weight?
void weightedAverage(){
	float wi;
	float wc;
	float pattern[3] = {0.0f, 0.0f, 0.0f};
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
			for(uint8_t j=0; j<NUM_PATTERNS; j++){
				pattern[j] += wi*(float)eightNeiPattern[i].pattern_f[j];
			}
		}
	}
	for (uint8_t i=0; i<NUM_PATTERNS; i++){
		pattern[i] += wc*me.myPattern_f[i];
	}
	
	if (TEST_CONSENSUS){
		for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++){
			if (eightNeiPattern[i].dropletId != 0){
				printf("%hu[%04X] Degree: %hu Pattern: %0.4f %0.4f %0.4f\r\n", i, eightNeiPattern[i].dropletId,
				eightNeiPattern[i].degree, eightNeiPattern[i].pattern_f[0], eightNeiPattern[i].pattern_f[1],
				eightNeiPattern[i].pattern_f[2]);
			}
		}
		printf("\r\nPre-pattern: [%0.4f %0.4f %0.4f] Cur-pattern: [%0.4f %0.4f %0.4f]\r\n", 
		me.myPattern_f[0], me.myPattern_f[1], me.myPattern_f[2], pattern[0], pattern[1], pattern[2]);
	}
	for (uint8_t i=0; i<NUM_PATTERNS; i++)
	{
		me.myPattern_f[i] = pattern[i];
	}
	
}

void changeColor(){
	uint8_t na = 0;
	uint8_t ni = 0;
	float ss = 0.0f;
	if (me.turing_color == 1){
		na += 1;
	}
	
	for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++){
		if (twelveNeiTuring[i].dropletId == 0){
			switch (i){
				case 0: twelveNeiTuring[i].color = twelveNeiTuring[2].color; break;
				case 1: twelveNeiTuring[i].color = twelveNeiTuring[3].color; break;
				case 2: twelveNeiTuring[i].color = twelveNeiTuring[0].color; break;
				case 3: twelveNeiTuring[i].color = twelveNeiTuring[1].color; break;
				case 4: twelveNeiTuring[i].color = twelveNeiTuring[7].color; break;
				case 5: twelveNeiTuring[i].color = twelveNeiTuring[6].color; break;
				case 6: twelveNeiTuring[i].color = twelveNeiTuring[5].color; break;
				case 7: twelveNeiTuring[i].color = twelveNeiTuring[4].color; break;
				case 8: twelveNeiTuring[i].color = twelveNeiTuring[10].color; break;
				case 9: twelveNeiTuring[i].color = twelveNeiTuring[11].color; break;
				case 10: twelveNeiTuring[i].color = twelveNeiTuring[8].color; break;
				case 11: twelveNeiTuring[i].color = twelveNeiTuring[9].color; break;
			}
		}
	}
	
	if ( (me.myPattern_f[0] > me.myPattern_f[1]) && (me.myPattern_f[0] > me.myPattern_f[2]) ) {	// pattern = 0: horizontal
		// exclude activator from inhibitor
		printf("Horizontal stripe!\r\n");
		for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
			if (twelveNeiTuring[hIndex[i]].color == 1){
				na += 1;
			}
		}
		for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++){
			if (i==1 || i==3) continue;
			else if (twelveNeiTuring[i].color == 1){
				ni += 1;
			}
		}
	}else if ((me.myPattern_f[1] > me.myPattern_f[0]) && (me.myPattern_f[1] > me.myPattern_f[2])){	// pattern = 1: vertical
		printf("Vertical stripe!\r\n");
		for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
			if (twelveNeiTuring[vIndex[i]].color == 1){
				na += 1;
			}
		}
		for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++){
			if (i==0 || i==2) continue;
			else if (twelveNeiTuring[i].color == 1){
				ni += 1;
			}
		}		
	}else{
		printf("Mottled Pattern!\r\n");		
		for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
			if (twelveNeiTuring[vIndex[i]].color == 1){
				na += 1;
			}
		}
		for (uint8_t i=4; i<NUM_NEIGHBOR_12; i++){
			if (twelveNeiTuring[i].color == 1){
				ni += 1;
			}
		}		
	}
	
	ss += (float)na - (float)ni*TURING_F;
	if (ss > 0){
		me.turing_color = 1;
	}else if (ss < 0){
		me.turing_color = 0;
	}

	if (me.turing_color == 0){
		set_rgb(255, 255, 255);
	}else{
		set_rgb(0, 0, 0);
	}	
	
	if (TEST_TURING) {
		for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++) {
			if (twelveNeiTuring[i].dropletId != 0) {
				printf("%hu[%04X] Color: %hu\r\n", i, twelveNeiTuring[i].dropletId, twelveNeiTuring[i].color);
			}
		}
	}
	
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
		printf("\t%hu [%04X]\r\n", i, me.neighborIds[i]);
	}
}

void printrgbs(){
	printf("\r\nPrint all rgbs read\r\n");
	for (uint8_t i=0; i<NUM_PREPARE; i++){
		printf("\t%hu: %d %d %d\r\n", i, allRGB[i].rgb[0], allRGB[i].rgb[1], allRGB[i].rgb[2]);
	}
}

void printrgbs_ordered(){
	printf("\r\nPrint all rgbs read (ordered)\r\n");
	for (uint8_t i=0; i<NUM_PREPARE; i++){
		printf("\t%hu: %d %d %d\r\n", i, red_array[i], green_array[i], blue_array[i]);
	}
}

void printfrgb(){
	printf("\r\nPrint final rgb and neighbors\r\n"); 
	printf("X: %d %d %d\r\n", me.rgb[0], me.rgb[1], me.rgb[2]);
	for (uint8_t i=0; i<NUM_NEIGHBOR_4; i++){
		printf("\t%hu: %d %d %d\r\n", i, fourNeiRGB[i].rgb[0], fourNeiRGB[i].rgb[1], fourNeiRGB[i].rgb[2]);
	}	
}

void printprob(){
	printf("\r\nPrint all pattern probs\r\n"); 
	for (uint8_t i=0; i<NUM_CONSENSUS; i++){
		printf("%0.6f %0.6f %0.6f\r\n", allPattern[i].pattern_f[0], allPattern[i].pattern_f[1], allPattern[i].pattern_f[2]);
	}
}

void printturing(){
	printf("\r\nPrint final turing colors\r\n"); 
	printf("X[%04X] Color: %u\r\n", me.dropletId, me.turing_color);
	for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++) {
		if (twelveNeiTuring[i].dropletId != 0) {
			printf("%hu[%04X] Color: %u\r\n", i, twelveNeiTuring[i].dropletId, twelveNeiTuring[i].color);
		}else{
			printf("%hu[    ] Color: #\r\n", i);
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

	if(strcmp(command_word, "set_thresh")==0){
		threshold_mottled = atoi(command_args);
	}

	return 0;
}
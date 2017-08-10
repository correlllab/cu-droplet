/*
* camouflage.c
* For Camouflage Project
* Created: 5/25/2016, re-factored on 6/30/2016
* Updated: 8/2/2017
* Author : Yang Li, John Klingner
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
2: mottle

*/

#include "camouflage.h"

/*
* Any code in this function will be run once, when the robot starts.
*/
void init(){
	me.dropletId = get_droplet_id();
	me.mySlot = me.dropletId%(SLOTS_PER_FRAME-1);
	//me.mySlot = get_droplet_ord(me.dropletId)-100; // For AUDIO_DROPLET
	//me.mySlot = (get_droplet_id()%(SLOTS_PER_FRAME-1));
	me.myDegree = 1;
	
	me.rgb[0] = 0;
	me.rgb[1] = 0;
	me.rgb[2] = 0;
	/********************* other variables ************************/ 
	// fourDr: use as a message to send out
	for (uint8_t i=0; i<NUM_PATTERNS; i++){
		me.myPattern_f[i] = 0.0f;
	}
	measRoot = NULL;
	lastMeasAdded = NULL;

	// global
	frameCount = 0;
	loopID = 0xFFFF;
	phase = Localize;
	printf("Initializing Camouflage Project. mySlot is %03hu\r\n", me.mySlot);
	frameStart = get_time();
	
	for(uint8_t i=0;i<NUM_PHASES;i++){
		frameLength[i] = ((uint32_t)slotLength[i])*((uint32_t)SLOTS_PER_FRAME);
	}

	printf("\r\n*************  Start LOCALIZE Phase   ***************\r\n");
}

/*
* The code in this function will be called repeatedly, as fast as it can execute.
*/
void loop(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime > frameLength[phase]){
		frameTime = frameTime - frameLength[phase];
		frameStart += frameLength[phase];
		frameCount++;
	}
	if(loopID!=(frameTime/slotLength[phase])){
		loopID = frameTime/slotLength[phase];
		if(loopID == me.mySlot){
			printf("Start of my slot, frame %lu.\r\n", frameCount);
			switch (phase){
				case Localize:	localizeSlot();		break;
				case Prepare:	sendBotPosMsg();	break;
				case Consensus:	sendPatternMsg();	break;
				case Turing:	sendTuringMsg();	break;
				case Waiting: /*Do nothing.*/		break;
			}
			printf("End of my slot, frame %lu.\r\n", frameCount);
		}else if(loopID == SLOTS_PER_FRAME-1){
			switch (phase){
				case Localize:	localizeEOP();		break;
				case Prepare:	prepareEOP();		break;
				case Consensus: consensusEOP();		break;
				case Turing:	turingEOP();		break;
				case Waiting:   /*waitingEOP();*/ 	break;
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
			case Localize: led_off(); break;
			case Prepare: set_rgb(80, 160, 160); break;
			case Consensus: set_rgb(200, 0, 0); break;
			default: /*Do nothing.*/ break;
		}
	}else if(loopID == SLOTS_PER_FRAME-1){
		switch (phase){
			case Localize: set_rgb(0, 200, 200); break;
			case Prepare: set_rgb(0, 200, 0); break;
			case Consensus:
				if(me.turing_color){
					set_rgb(200, 0, 0);
				}else{
					set_rgb(200,200,200);
				}
				break;
			case Turing: changeColor(); break;
			default: /*Do nothing.*/ break;
		}
	}else{
		if(phase==Waiting || phase==Turing){
			/*Do nothing.*/
		}else{
			led_off();
		}
	}
}

void handleRNB(){
	/*
		* Comment in/out the line below to toggle the Localization system.
		* If you do, consult BotPos myPos for your position estimate, and
		* DensePosCovar myPosCovar for the error in your position estimate.
		* call decompressP with a pointer to myPosCovar and a pointer to a Matrix
		*     to unpack the covar in to something useful.
		* Example:
		*     Matrix covar;
		*     decompressP(&covar, &myPosCovar);
		*/
	useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
}



void sendBotPosMsg(){
	BotPosMsg msg;
	msg.bots[0] = myPosColor;
	
	uint8_t indices[NUM_CHOSEN_BOTS];
	chooseTransmittedBots(&indices);
	for(uint8_t i=1;i<(NUM_CHOSEN_BOTS+1);i++){
		msg.bots[i+1] = otherBots[indices[i]];
	}

	msg.flag = BOT_POS_MSG_FLAG;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(BotPosMsg));
}

void handleBotPosMsg(BotPosMsg* msg, id_t sender){
	for(uint8_t i=0;i<NUM_TRANSMITTED_BOTS;i++){
		addBot( (msg->bots)[i]);
	}
}

void sendPatternMsg(){
	patternMsg msg;
	msg.flag = PATTERN_MSG_FLAG;
	msg.dropletId = me.dropletId;
	msg.degree = me.myDegree;
	for (uint8_t i=0; i<NUM_PATTERNS; i++){
		float tmp = me.myPattern_f[i];
		msg.pattern_f[i] = (tmp>1.0) ? 1.0 : ((tmp<0.0) ? 0.0 : tmp);
		me.myPattern_f[i] = msg.pattern_f[i];
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
	if(((BotMeasMsg*)(msg_struct->msg))->flag==BOT_MEAS_MSG_FLAG && msg_struct->length==sizeof(BotMeasMsg)){
		handleBotMeasMsg((BotMeasMsg*)(msg_struct->msg), msg_struct->sender_ID);
	}else{	
		switch (phase){
			case Prepare: handleBotPosMsg((((BotPosMsg*)(msg_struct->msg))), msg_struct->sender_ID); break;
			case Consensus: handle_pattern_msg((patternMsg*)msg_struct->msg); break;
			case Turing: handle_turing_msg((turingMsg*)msg_struct->msg); break;
			default: break;
		}
	}
}

void handle_pattern_msg(patternMsg* msg){
	if(msg->flag == PATTERN_MSG_FLAG){

	}
}

void handle_turing_msg(turingMsg* msg){
	if(msg->flag == TURING_MSG_FLAG){

	}
}

void localizeSlot(){
	broadcast_rnb_data();
	// read colors
	int16_t red, green, blue;

	get_rgb(&red, &green, &blue);

	// store to print
	allRGB[frameCount][0] = red;
	allRGB[frameCount][1] = green;
	allRGB[frameCount][2] = blue;
	red_array[frameCount] = red;
	green_array[frameCount] = green;
	blue_array[frameCount] = blue;
	printf("X[%04X] R: %d G: %d B: %d (ori)\r\n", me.dropletId, red, green, blue);
}

void localizeEOP(){
	if(frameCount<(NUM_PREPARE-1)){
		printf("My Pos: % 4d, % 4d, %4d\r\n", myPos.x, myPos.y, myPos.o);
	}else{
		if(!POS_DEFINED(&myPos)){
			printf("Localize ended without my figuring out my position.\r\n");
			warning_light_sequence();
		}
		me.rgb[0] = meas_find_median(red_array, NUM_PREPARE);
		me.rgb[1] = meas_find_median(green_array, NUM_PREPARE);
		me.rgb[2] = meas_find_median(blue_array, NUM_PREPARE);

		myPosColor.x = myPos.x;
		myPosColor.y = myPos.y;
		myPosColor.col = packColor(me.rgb[0], me.rgb[1], me.rgb[2]);
		
		me.turing_color = (me.rgb[0]+me.rgb[1])>130;

		phase=Prepare;
		frameCount = 0;
		printf("\r\n*************  Start PREPARE Phase   ***************\r\n");
	}

}

void prepareEOP(){
	/* End of frame. Do some final processing here */
	//updateNeighbors();
	//extendNeighbors();
	if (frameCount<(NUM_PREPARE-1)) {
		if(TEST_PREPARE){
			printf("X[%04X] R: %d G: %d B: %d\r\n", me.dropletId, allRGB[frameCount][0], allRGB[frameCount][1], allRGB[frameCount][2]);
		}
	}else{
		printf("Once only loop in Prepare\r\n");
		decidePattern();
		phase=Consensus;
		frameCount = 0;
		printf("\r\n*************  Start CONSENSUS Phase   ***************\r\n");
	}
}

void consensusEOP(){
	weightedAverage();
	if (frameCount<(NUM_CONSENSUS-1)) {
		for (uint8_t i=0; i<NUM_PATTERNS; i++){
			allPattern[frameCount].pattern_f[i] = me.myPattern_f[i];
		}
	}else{
		phase=Turing;
		frameCount = 0;
		printf("\r\n*************  Start TURING Phase   ***************\r\n");
	}
}

void turingEOP(){
	/* End of frame. Do some final processing here */
	//changeColor();
	if (frameCount>=(NUM_TURING-1)){
		printf("\r\nAll Done!\r\n");
		displayMenu();
		frameCount = 0;
		phase=Waiting;
		printf("\r\n*************  Start WAITING Phase   ***************\r\n");
	}
}


void waitingEOP(){
	if (frameCount>=(NUM_WAITING-1)){
		printf("\r\nRestarting!\r\n");
		frameCount = 0;
		phase = Prepare;
	}
}

/*
 * Occurs at end of Prepare phase.
 * Note: ONLY ONCE
 */
void decidePattern(){
	float xGradient = 0.0;
	float yGradient = 0.0;
	float LofGx, LofGy;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(POS_C_DEFINED(&(otherBots[i]))){
			break;
		}
		//Maybe add a cut off here, so we ignore contributions from robots farther than ______ away?
		LofGxy(otherBots[i].x - myPosColor.x, otherBots[i].y - myPosColor.y, &LofGx, &LofGy);
		xGradient += fabs(unpackColorToGray(otherBots[i].col)) * LofGx;
		yGradient += fabs(unpackColorToGray(otherBots[i].col)) * LofGy;
	}
	float gradientDirRad = atan2(yGradient, xGradient);
	float gradientMagnitude = sqrtf(xGradient*xGradient + yGradient*yGradient);

}

// Why small degree has larger weight?
void weightedAverage(){
	float wi;
	float wc;
	float pattern[3] ={0.0, 0.0, 0.0};
	uint8_t maxDegree;
	wc = 1.0;
	for (uint8_t i=0; i<NUM_NEIGHBOR_8; i++){
		if (eightNeiPattern[i].dropletId){
			maxDegree = me.myDegree;
			if (maxDegree < eightNeiPattern[i].degree){
				maxDegree = eightNeiPattern[i].degree;
			}
			wi = 1.0/(1.0+maxDegree);
			wc -= wi;
			for(uint8_t j=0; j<NUM_PATTERNS; j++){
				pattern[j] += wi*eightNeiPattern[i].pattern_f[j];
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

// Change me.turing_color according to Young's model
// the neighbors' colors are also added to turingHistory array for record
void changeColor(){
	uint8_t na = 0;
	uint8_t ni = 0;
	float ss = 0.0f;
	if (me.turing_color == 1){
		na += 1;
	}
	
	// save the original colors to turingHistory
	for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++)
	{
		if (twelveNeiTuring[i].dropletId != 0)
		{
			turingHistory[frameCount][i] = twelveNeiTuring[i].color;
		} 
		else
		{
			turingHistory[frameCount][i] = 2;
		}
	}
	
	
	for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++){
		if (twelveNeiTuring[i].dropletId == 0){
			switch (i){
				case 0: twelveNeiTuring[i].color = twelveNeiTuring[2].color; break;
				case 1: twelveNeiTuring[i].color = twelveNeiTuring[3].color; break;
				case 2: twelveNeiTuring[i].color = twelveNeiTuring[0].color; break;
				case 3: twelveNeiTuring[i].color = twelveNeiTuring[1].color; break;
				case 4:
					if((twelveNeiTuring[7].dropletId == 0) && (twelveNeiTuring[5].dropletId == 0)){
						twelveNeiTuring[i].color = twelveNeiTuring[6].color;
					}else if((twelveNeiTuring[5].dropletId != 0)){
						twelveNeiTuring[i].color = twelveNeiTuring[5].color;
					}else if(twelveNeiTuring[7].dropletId !=0){
						twelveNeiTuring[i].color = twelveNeiTuring[7].color;
					}else{
						twelveNeiTuring[i].color = 0;
					}
					break;				
				case 5: 
					if((twelveNeiTuring[4].dropletId == 0) && (twelveNeiTuring[6].dropletId == 0)){
						twelveNeiTuring[i].color = twelveNeiTuring[7].color;
						}else if((twelveNeiTuring[4].dropletId != 0)){
						twelveNeiTuring[i].color = twelveNeiTuring[4].color;
						}else if(twelveNeiTuring[6].dropletId !=0){
						twelveNeiTuring[i].color = twelveNeiTuring[6].color;
						}else{
						twelveNeiTuring[i].color = 0;
					}				
					break;
				case 6: 
					if((twelveNeiTuring[7].dropletId == 0) && (twelveNeiTuring[5].dropletId == 0)){
						twelveNeiTuring[i].color = twelveNeiTuring[4].color;
						}else if((twelveNeiTuring[5].dropletId != 0)){
						twelveNeiTuring[i].color = twelveNeiTuring[5].color;
						}else if(twelveNeiTuring[7].dropletId !=0){
						twelveNeiTuring[i].color = twelveNeiTuring[7].color;
						}else{
						twelveNeiTuring[i].color = 0;
					}				
					break;
				case 7: 
					if((twelveNeiTuring[4].dropletId == 0) && (twelveNeiTuring[6].dropletId == 0)){
						twelveNeiTuring[i].color = twelveNeiTuring[5].color;
						}else if((twelveNeiTuring[4].dropletId != 0)){
						twelveNeiTuring[i].color = twelveNeiTuring[4].color;
						}else if(twelveNeiTuring[6].dropletId !=0){
						twelveNeiTuring[i].color = twelveNeiTuring[6].color;
						}else{
						twelveNeiTuring[i].color = 0;
					}
					break;
				case 8: twelveNeiTuring[i].color = twelveNeiTuring[10].color; break;
				case 9: twelveNeiTuring[i].color = twelveNeiTuring[11].color; break;
				case 10: twelveNeiTuring[i].color = twelveNeiTuring[8].color; break;
				case 11: twelveNeiTuring[i].color = twelveNeiTuring[9].color; break;
			}
		}
	}

	// save the corrected colors to turingHistoryCorrected
	for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++)
	{
		turingHistoryCorrected[frameCount][i] = twelveNeiTuring[i].color;
	}	
	
	if ( (me.myPattern_f[0] > me.myPattern_f[1]) && (me.myPattern_f[0] > me.myPattern_f[2]) ) {	// pattern = 0: horizontal
		// exclude activator from inhibitor
		printf("Horizontal stripe!\r\n");
		na += (twelveNeiTuring[1].color == 1);
		na += (twelveNeiTuring[3].color == 1);
		na += (twelveNeiTuring[9].color == 1);
		na += (twelveNeiTuring[11].color == 1);

		ni += (twelveNeiTuring[0].color == 1);
		ni += (twelveNeiTuring[2].color == 1);
		ni += (twelveNeiTuring[4].color == 1);
		ni += (twelveNeiTuring[5].color == 1);
		ni += (twelveNeiTuring[6].color == 1);
		ni += (twelveNeiTuring[7].color == 1);
	}else if ((me.myPattern_f[1] > me.myPattern_f[0]) && (me.myPattern_f[1] > me.myPattern_f[2])){	// pattern = 1: vertical
		printf("Vertical stripe!\r\n");
		na += (twelveNeiTuring[0].color == 1);
		na += (twelveNeiTuring[2].color == 1);
		na += (twelveNeiTuring[8].color == 1);
		na += (twelveNeiTuring[10].color == 1);

		ni += (twelveNeiTuring[1].color == 1);
		ni += (twelveNeiTuring[3].color == 1);
		ni += (twelveNeiTuring[4].color == 1);
		ni += (twelveNeiTuring[5].color == 1);
		ni += (twelveNeiTuring[6].color == 1);
		ni += (twelveNeiTuring[7].color == 1);
	}else{
		printf("Mottled Pattern!\r\n");		
		na += (twelveNeiTuring[0].color == 1);
		na += (twelveNeiTuring[1].color == 1);
		na += (twelveNeiTuring[2].color == 1);
		na += (twelveNeiTuring[3].color == 1);

		ni += (twelveNeiTuring[4].color == 1);
		ni += (twelveNeiTuring[5].color == 1);
		ni += (twelveNeiTuring[6].color == 1);
		ni += (twelveNeiTuring[7].color == 1);
		ni += (twelveNeiTuring[8].color == 1);
		ni += (twelveNeiTuring[9].color == 1);	
		ni += (twelveNeiTuring[10].color == 1);	
		ni += (twelveNeiTuring[11].color == 1);	
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
		set_rgb(255, 0, 0);
	}	
	
	if (TEST_TURING) {
		for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++) {
			if (twelveNeiTuring[i].dropletId != 0) {
				printf("%hu[%04X] Color: %hu\r\n", i, twelveNeiTuring[i].dropletId, twelveNeiTuring[i].color);
			}
		}
	}
	
}

void printNs(){
	printf("\r\nPrint neighbors' ID\r\n");
	printf("X[%04X]\r\n", me.dropletId);
	for (uint8_t i=0; i<NUM_NEIGHBOR_12; i++)
	{
		if (me.neighborIds[i] != 0)
		{
			printf("\t%hu [%04X]\r\n", i, me.neighborIds[i]);
		} 
		else
		{
			printf("\t%hu [----]\r\n", i);
		}
		
	}
}

uint8_t user_handle_command(char* command_word, char* command_args){
	//if(strcmp(command_word, "pn")==0){
		//printNs();
	//}
	//if(strcmp(command_word, "pp")==0){
		//printProb();
	//}
	//if(strcmp(command_word, "pt")==0){
		//printTuring();
	//}
	//if(strcmp(command_word, "pa")==0){
		//printNs();
		////printrgbs();
		//printRGBs_ordered();
		//printRGB();		
		//printProb();
		//printTuring();
		//printTuringHistory();
		//printTuringHistoryCorrected();

	}

	if(strcmp(command_word, "set_thresh")==0){
		threshold_mottled = atoi(command_args);
	}

	return 0;
}

//Returns 1 if a new bot was added to the array.
//Returns 0 if the bot was already in the array.
uint8_t addBot(PosColor pos){
	uint8_t foundIdx = NUM_TRACKED_BOTS;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if((otherBots[i].x == pos.x) && (otherBots[i].y == pos.y)){
			return 0; //The bot already existed; no need to add it.
		}
		if(!POS_C_DEFINED(&(otherBots[i]))){
			foundIdx = i;
		}
	}
	otherBots[foundIdx] = pos;
	qsort(otherBots, NUM_TRACKED_BOTS+1, sizeof(PosColor), distCmp);
	return 1;
}

void chooseTransmittedBots(uint8_t (*indices)[NUM_CHOSEN_BOTS]){
	uint8_t numSelected = 0;
	uint8_t numChoices = 0;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(POS_C_DEFINED(otherBots[i])){
			numChoices++;
		}else{
			break;
		}
	}//Now, numChoices is the number of valid trackedBots.
	uint16_t randomizer[numChoices];
	for(uint8_t i=0;i<numChoices;i++){
		randomizer[i] = (((uint16_t)i)<<8) | rand_byte();
	}
	qsort(randomizer, numChoices, sizeof(uint16_t), randomizerCmp);
	for(uint8_t i=0;i<NUM_CHOSEN_BOTS;i++){
		(*indices)[i] = ((uint8_t)((randomizer[i]>>8)&0xFF));
	}
}
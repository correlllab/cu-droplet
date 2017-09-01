/*
* camouflage.c
* For Camouflage Project
* Created: 5/25/2016, re-factored on 6/30/2016
* Updated: 8/12/2017 (with localization)
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
	me.slot = me.dropletId%(SLOTS_PER_FRAME-1);
	//me.mySlot = get_droplet_ord(me.dropletId)-100; // For AUDIO_DROPLET
	//me.mySlot = (get_droplet_id()%(SLOTS_PER_FRAME-1));
	me.degree = 1;
	
	me.rgb[0] = 0;
	me.rgb[1] = 0;
	me.rgb[2] = 0;
	me.nA = 0;
	me.nI = 0;
	/********************* other variables ************************/ 
	// fourDr: use as a message to send out
	me.p.x = NAN;
	me.p.y = NAN;
	myPosColor.x = UNDF;
	myPosColor.y = UNDF;
	measRoot = NULL;
	lastMeasAdded = NULL;
	nbrPatternRoot = NULL;
	lastPatternAdded = NULL;
	
	#if (!ONLY_ACTIVE_BOTS_SEND_TURING_MSG)
	turingRoot_a = NULL;
	lastAddedturingNode_a = NULL;
	turingRoot_i = NULL;
	lastAddedturingNode_i = NULL;
	#endif 

	// global
	frameCount = 0;
	loopID = 0xFFFF;
	phase = Localize;
	printf("Initializing Camouflage Project. My slot is %03hu\r\n", me.slot);
	frameStart = get_time();
	
	for(uint8_t i=0;i<NUM_PHASES;i++){
		frameLength[i] = ((uint32_t)slotLength[i])*((uint32_t)SLOTS_PER_FRAME);
	}

	for(uint8_t i=0;i<NUM_TRACKED_BOTS+1;i++){
		otherBots[i].x = UNDF;
		otherBots[i].y = UNDF;
		otherBots[i].col = 0;
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
		if(loopID == me.slot){
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
		if(phase==Localize){
			handleRNB();
		}
		rnb_updated = 0;
	}

	/* Define the duration of loop */
	delay_ms(LOOP_DELAY_MS);
}

void setColor(){
	if(loopID == me.slot){
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
			case Consensus: set_rgb(0, 0, 200); break; //fall through; consensus and turing should behave the same way.
			case Turing:
				if(me.turingColor){
					set_rgb(200, 0, 0);
				}
				else{
					set_rgb(200,200,200);
				}
				break;
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

void printPosColor(PosColor pos){
	if(POS_C_DEFINED(&pos)){
		printf("\t\t(% 5d, % 5d) : %hu\r\n", pos.x, pos.y, pos.col);
	}
}

void printOtherBots(){
	for(uint8_t i=0;i<NUM_TRACKED_BOTS+1;i++){
		printPosColor(otherBots[i]);
	}
}

void chooseTransmittedBots(uint8_t (*indices)[NUM_CHOSEN_BOTS]){
	//printf("\tChoosing Transmitted.\r\n");
	//printOtherBots();
	uint8_t numChoices = 0;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(POS_C_DEFINED(&(otherBots[i]))){
			numChoices++;
		}
		else{
			break;
		}
	}
	printf("\tNum Choices: %hu\r\n", numChoices);
	for(uint8_t i=0;i<NUM_CHOSEN_BOTS;i++){
		if(i<numChoices){
			randomizer[i] = (((uint16_t)i)<<8) | rand_byte();
			//printf("\t\t%04X\r\n", randomizer[i]);
		}else{
			randomizer[i] = 0xFFFF;
			//printf("\t\t--\r\n");
		}
	}
	qsort(randomizer, NUM_CHOSEN_BOTS, sizeof(uint16_t), randomizerCmp);
	//printf("\tSorted:\r\n");
	for(uint8_t i=0;i<NUM_CHOSEN_BOTS;i++){
		(*indices)[i] = ((uint8_t)((randomizer[i]>>8)&0xFF));
		//if( (*indices)[i] != 0xFF){
			////printf("\t\t%hu (%04X)\r\n", (*indices)[i], randomizer[i]);
		//}else{
			////printf("\t\t--\r\n");	
		//}
	}
}

void sendBotPosMsg(){
	delay_ms(20);
	BotPosMsg msg;
	msg.bots[0] = myPosColor;
	
	uint8_t indices[NUM_CHOSEN_BOTS];
	uint8_t idx = 0;
	chooseTransmittedBots(&indices);
	for(uint8_t i=1;i<NUM_TRANSMITTED_BOTS;i++){
		idx = indices[i-1];
		if(idx!=0xFF){
			msg.bots[i] = otherBots[idx];
		}else{
			msg.bots[i].x = UNDF;
			msg.bots[i].y = UNDF;
			msg.bots[i].col = 0;
		}
	}

	msg.flag = BOT_POS_MSG_FLAG;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(BotPosMsg));
}

void sendPatternMsg(){
	PatternMsg msg;
	msg.flag = PATTERN_MSG_FLAG;
	msg.dropletId = me.dropletId;
	msg.degree = me.degree;
	msg.p.x = me.p.x;
	msg.p.y = me.p.y;
	
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(PatternMsg));
}

#if ONLY_ACTIVE_BOTS_SEND_TURING_MSG

void sendTuringMsg(){
	if(me.turingColor){
		TuringMsg msg;
		msg.flag = TURING_MSG_FLAG;
		msg.x = myPosColor.x;
		msg.y = myPosColor.y;
		ir_send(ALL_DIRS, (char*)(&msg), sizeof(TuringMsg));
	}
}

#else

void sendTuringMsg(){
	TuringMsg msg;
	msg.flag = TURING_MSG_FLAG;
	msg.x = myPosColor.x;
	msg.y = myPosColor.y;
	msg.t_color = me.turingColor;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(TuringMsg));
}

#endif

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
			case Consensus: handlePatternMsg((PatternMsg*)msg_struct->msg); break;
			case Turing: handleTuringMsg((TuringMsg*)msg_struct->msg, msg_struct->sender_ID); break;
			default: break;
		}
	}
}

//Returns 1 if a new bot was added to the array.
//Returns 0 if the bot was already in the array.
//Returns 0 if the bot has the same position as me.
uint8_t addBot(PosColor pos){
	uint8_t foundIdx = NUM_TRACKED_BOTS;
	if(pos.x==myPosColor.x && pos.y==myPosColor.y){
		return 0;
	}
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
	//printf("\tAdded Pos:\r\n");
	//printPosColor(pos);
	//printf("\tOther Bots:\r\n");
	//printOtherBots();
	return 1;
}


void handleBotPosMsg(BotPosMsg* msg, id_t senderID){	
	for(uint8_t i=0;i<NUM_TRANSMITTED_BOTS;i++){
		if(POS_C_DEFINED(&((msg->bots)[i]))){
			PosColor pos = (msg->bots)[i];
			uint8_t result = addBot( pos );
			if(result){
				//printf("\taddbot() - Add bot: [%d, %d] (%hu) from %04X\r\n", pos.x, pos.y, i, senderID);
			}else{
				//printf("\taddbot() - The bot already existed: [%d, %d] (%hu) from %04X\r\n", pos.x, pos.y, i, senderID);
			}
		}
	}
}

/* Used in Consensus Phase */
void handlePatternMsg(PatternMsg* msg){
	if(msg->flag == PATTERN_MSG_FLAG){
		if(lastPatternAdded==NULL){
			lastPatternAdded = (PatternNode*)myMalloc(sizeof(PatternNode));
			nbrPatternRoot = lastPatternAdded;
		}else{
			lastPatternAdded->next = (PatternNode*)myMalloc(sizeof(PatternNode));
			lastPatternAdded = lastPatternAdded->next;
		}
		lastPatternAdded->degree = msg->degree;
		lastPatternAdded->p.x = msg->p.x;
		lastPatternAdded->p.y = msg->p.y;
		lastPatternAdded->next = NULL;
	}
}

#if ONLY_ACTIVE_BOTS_SEND_TURING_MSG

void handleTuringMsg(TuringMsg* msg, id_t senderID){
	if(msg->flag == TURING_MSG_FLAG){
		//TODO
		Vector pos = {msg->x, msg->y, 1};
		Vector activatorTF, inhibitorTF;
		matrixTimesVector(&activatorTF, &(me.patternTransformA), &pos);
		matrixTimesVector(&inhibitorTF, &(me.patternTransformI), &pos);
		printf("\t{{% 4d, % 4d}, {% 6.3f, % 6.3f}, {% 6.3f, % 6.3f}}\r\n", msg->x, msg->y, activatorTF[0], activatorTF[1], inhibitorTF[0], inhibitorTF[1]);
		if((activatorTF[0]*activatorTF[0]+activatorTF[1]*activatorTF[1])<=1.0){ //if the position is inside unit circle after activator transformation..
			me.nA++;
		}else if((inhibitorTF[0]*inhibitorTF[0]+inhibitorTF[1]*inhibitorTF[1])<=1.0){ //if the position is inside unit circle after inhibitor transformation..
			me.nI++;
		}
	}
}

#else

uint8_t isInList(id_t id, uint8_t color, uint8_t activator){
	TuringNode* tmp = NULL;
	if (activator == 1){
		tmp = turingRoot_a;
		while(tmp != NULL){
			if (tmp->id == id){
				tmp->t_color = color;
				return 1;
			}
			tmp = tmp->next;
		}
	}
	else{
		tmp = turingRoot_i;
		while(tmp != NULL){
			if (tmp->id == id){
				tmp->t_color = color;
				return 1;
			}
			tmp = tmp->next;
		}
	}
	return 0;
}

void handleTuringMsg(TuringMsg* msg, id_t senderID){
	if(msg->flag == TURING_MSG_FLAG){
		Vector pos = {msg->x, msg->y, 1};
		Vector activatorTF, inhibitorTF;
		matrixTimesVector(&activatorTF, &(me.patternTransformA), &pos);
		matrixTimesVector(&inhibitorTF, &(me.patternTransformI), &pos);
		printf("\t{{% 4d, % 4d}, {% 6.3f, % 6.3f}, {% 6.3f, % 6.3f}}\r\n", msg->x, msg->y, activatorTF[0], activatorTF[1], inhibitorTF[0], inhibitorTF[1]);
		if((activatorTF[0]*activatorTF[0]+activatorTF[1]*activatorTF[1])<=1.0){ //if the position is inside unit circle after activator transformation..
			if(isInList(senderID, msg->t_color, 1) == 0){
				if(lastAddedturingNode_a==NULL){
					lastAddedturingNode_a = (TuringNode*)myMalloc(sizeof(TuringNode));
					turingRoot_a = lastAddedturingNode_a;
					}
				else{
					lastAddedturingNode_a->next = (TuringNode*)myMalloc(sizeof(TuringNode));
					lastAddedturingNode_a = lastAddedturingNode_a->next;
				}
				lastAddedturingNode_a->t_color = msg->t_color;
				lastAddedturingNode_a->id = senderID;
				lastAddedturingNode_a->next = NULL;
			}
		}
		else if((inhibitorTF[0]*inhibitorTF[0]+inhibitorTF[1]*inhibitorTF[1])<=1.0){ //if the position is inside unit circle after inhibitor transformation..
			if(isInList(senderID, msg->t_color, 0) == 0){
				if(lastAddedturingNode_i==NULL){
					lastAddedturingNode_i = (TuringNode*)myMalloc(sizeof(TuringNode));
					turingRoot_i = lastAddedturingNode_i;
				}
				else{
					lastAddedturingNode_i->next = (TuringNode*)myMalloc(sizeof(TuringNode));
					lastAddedturingNode_i = lastAddedturingNode_i->next;
				}
				lastAddedturingNode_i->t_color = msg->t_color;
				lastAddedturingNode_i->id = senderID;
				lastAddedturingNode_i->next = NULL;
			}
		}
	}
}

#endif

void localizeSlot(){
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
	printf("\tX[%04X] R: %d G: %d B: %d (ori)\r\n", me.dropletId, red, green, blue);
	broadcast_rnb_data();
}

void localizeEOP(){
	if(frameCount<(NUM_LOCALIZE-1)){
		if(POS_DEFINED(&myPos)){
			printf("\tMy Pos: [%d, %d, %d]\r\n", myPos.x, myPos.y, myPos.o);
		}else{
			printf("\tPosition not yet found.\r\n");
		}
	}else{
		if(!POS_DEFINED(&myPos)){
			printf("Localize ended without my figuring out my position.\r\n");
			warning_light_sequence();
		}
		me.rgb[0] = meas_find_median(red_array, NUM_LOCALIZE);
		me.rgb[1] = meas_find_median(green_array, NUM_LOCALIZE);
		me.rgb[2] = meas_find_median(blue_array, NUM_LOCALIZE);

		me.turingColor = (me.rgb[0]+me.rgb[1])<90;

		myPosColor.x = myPos.x;
		myPosColor.y = myPos.y;
		myPosColor.col = packColor(me.rgb[0], me.rgb[1], me.rgb[2], me.turingColor);
		
		phase=Prepare;
		frameCount = 0;
		printf("\r\n*************  Start PREPARE Phase   ***************\r\n");
	}

}

void prepareEOP(){
	if (frameCount<(NUM_PREPARE-1)) {
		if(TEST_PREPARE){
			;
		}
	}else{
		printf("Once only loop in Prepare\r\n");
		decidePatternB();
		phase=Consensus;
		frameCount = 0;
		printf("\r\n*************  Start CONSENSUS Phase   ***************\r\n");
	}
}

void consensusEOP(){
	weightedAverage();
	if (frameCount<(NUM_CONSENSUS-1)) {
		allPatterns[frameCount] = me.p;
	}else{
		//Compute the appropriate transformations:
		float pTheta = atan2(me.p.y, me.p.x)/2;
		
		printf("pTheta: % 4d\r\n\tNOTE: This theta has been UNDOUBLED.\r\n", (int16_t)rad_to_deg(pTheta));
		/*
		 * pTheta is the 'characteristic angle' of the pattern.
		 * All of a pattern's stripes run parallel to this angle.
		 */
		Matrix translate = {{1, 0, -myPosColor.x}, {0, 1,  -myPosColor.y}, {0, 0, 1}};
		Matrix rotate = {{cos(pTheta), sin(pTheta), 0}, {-sin(pTheta), cos(pTheta), 0}, {0, 0, 1}};
		Matrix activatorScale = {{1.0/ACTIVATOR_WIDTH, 0, 0}, {0, 1.0/ACTIVATOR_HEIGHT, 0}, {0, 0, 1}};
		Matrix inhibitorScale = {{1.0/INHIBITOR_WIDTH, 0, 0}, {0, 1.0/INHIBITOR_HEIGHT, 0}, {0, 0, 1}};

		Matrix tmp;
		matrixMultiply(&tmp, &rotate, &translate);
		matrixMultiply(&(me.patternTransformA), &activatorScale, &tmp);
		matrixMultiply(&(me.patternTransformI), &inhibitorScale, &tmp);

		printMatrixMathematica(&(me.patternTransformA));
		printMatrixMathematica(&(me.patternTransformI));

		phase=Turing;
		frameCount = 0;
		printf("\r\n*************  Start TURING Phase   ***************\r\n");
	}
}

void turingEOP(){
	updateTuringColor();
	if (frameCount<(NUM_TURING-1)){
		//Nothing right now.
	}else{
		printf("\r\nAll Done!\r\n");
		//displayMenu();
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
	me.p.x = 0.0;
	me.p.y = 0.0;
	float LofGx, LofGy;
	uint8_t count = 0;
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		// reach the end of filled spots of otherBots[]
		if(!POS_C_DEFINED(&(otherBots[i]))){
			break;
		}
		count ++;
		//Maybe add a cut off here, so we ignore contributions from robots farther than ______ away?
		LofGxy(otherBots[i].x - myPosColor.x, otherBots[i].y - myPosColor.y, &LofGx, &LofGy);

		me.p.x += unpackColorToBinary(otherBots[i].col) * LofGx;
		me.p.y += unpackColorToBinary(otherBots[i].col) * LofGy;
		
		//me.p.x += fabs(unpackColorToGray(otherBots[i].col)) * LofGx;
		//me.p.y += fabs(unpackColorToGray(otherBots[i].col)) * LofGy;
		
		printf("\tLofGx: %.3f, LofGy: %.3f --> p.x: %.3f, p.y: %.3f, pTheta: %5.3f\r\n", LofGx, LofGy, me.p.x, me.p.y, atan2(me.p.y, me.p.x));
	}
	printf("\tNum of tracked bots is: %hu\r\n", count);
	
	me.p.x = fabs(me.p.x);
	me.p.y = fabs(me.p.y);
}

void testLoG(NMPoint* pt){
	float theta =  atan2(pt->x, pt->y);
	float cosTheta = cos(theta);
	float sinTheta = sin(theta);
	float p = unpackColorToBinary(myPosColor.col)*LofGx(0,0);
	for(uint8_t i=0;i<NUM_TRACKED_BOTS;i++){
		if(POS_C_DEFINED(&(otherBots[i]))){
			float xDiff = otherBots[i].x - myPosColor.x;
			float yDiff = otherBots[i].y - myPosColor.y;
			float x = xDiff*cosTheta - yDiff*sinTheta;
			float y = xDiff*sinTheta + yDiff*cosTheta;
			x = x/PATTERN_WIDTH;
			y = y/PATTERN_WIDTH;
			p += unpackColorToBinary(otherBots[i].col) * (LofGx(x, y));
		}
	}
	pt->val = fabsf(p);
}

/*
 * Occurs at end of Prepare phase.
 * Note: ONLY ONCE
 */
void decidePatternB(){ //Using optimal rotation.
	Simplex spx;
	spx[0].x = 0.96592582628;
	spx[0].y = 0.2588190451;
	testLoG(&spx[0]);
	spx[1].x = -0.70710678118;
	spx[1].y =  0.70710678118;
	testLoG(&spx[1]);
	spx[2].x = -0.2588190451;
	spx[2].y = -0.96592582628;
	testLoG(&spx[2]);
	float result;
	result = NMStep(&spx);
	uint8_t c = 1;
	while(result>0.1 && c<50){
		printf("\tSimplex Step: %2hu (%5.3f)\r\n", c, result);		
		result = NMStep(&spx);
		c++;
	}
	if(spx[0].y < 0){
		spx[0].x = -spx[0].x;
		spx[0].y = -spx[0].y;
	}
	float theta = -atan2(spx[0].y, spx[0].x);
	me.p.x = cos(2*theta);
	me.p.y = sin(2*theta);
	printf("Px: %5.3f, Py: %5.3f (% 4d) NOTE: This theta is now doubled.\r\n\n", me.p.x, me.p.y, (int16_t)rad_to_deg(2*theta));
}

float NMStep(Simplex* spx){
	qsort(*spx, 3, sizeof(NMPoint), simplexCmp);
	printf("\t                % 4d, %6.3f\r\n", (int16_t)rad_to_deg(atan2((*spx)[0].y, (*spx)[0].x)), (*spx)[0].val);
	printf("\t                % 4d, %6.3f\r\n", (int16_t)rad_to_deg(atan2((*spx)[0].y, (*spx)[1].x)), (*spx)[1].val);
	printf("\t                % 4d, %6.3f\r\n", (int16_t)rad_to_deg(atan2((*spx)[0].y, (*spx)[2].x)), (*spx)[2].val);	
	NMPoint xo, xr;
	//xo is the centroid o x[0] & x[1];
	xo.x = ((*spx)[0].x + (*spx)[1].x)/2.0;
	xo.y = ((*spx)[0].y + (*spx)[1].y)/2.0;
	testLoG(&xo);
	//xr is x[2] reflected opposite xo;
	xr.x = xo.x + NM_ALPHA*(xo.x - (*spx)[2].x);
	xr.y = xo.y + NM_ALPHA*(xo.y - (*spx)[2].y);
	testLoG(&xr);
	if((xr.val >= (*spx)[0].val) && (xr.val < (*spx)[1].val)){
		float tmp = hypotf(xr.x-(*spx)[2].x,xr.y-(*spx)[2].y);
		(*spx)[2] = xr;
		return tmp;
	}else if(xr.val < (*spx)[0].val){
		NMPoint xe;
		xe.x = xo.x + NM_GAMMA*(xr.x - xo.x);
		xe.y = xo.y + NM_GAMMA*(xr.y - xo.y);
		testLoG(&xe);
		if(xe.val < xr.val){
			float tmp = hypotf(xe.x-(*spx)[2].x,xe.y-(*spx)[2].y);
			(*spx)[2] = xe; 
			return tmp;
		}else{
			float tmp = hypotf(xr.x-(*spx)[2].x,xr.y-(*spx)[2].y);
			(*spx)[2] = xr;
			return tmp;
		}
	}else{
		NMPoint xc;
		xc.x = xo.x + NM_RHO*((*spx)[2].x - xo.x);
		xc.y = xo.y + NM_RHO*((*spx)[2].y - xo.y);
		testLoG(&xc);
		if(xc.val < (*spx)[2].val){
			float tmp = hypotf(xc.x-(*spx)[2].x,xc.y-(*spx)[2].y);
			(*spx)[2] = xc;
			return tmp;
		}else{
			(*spx)[1].x = (*spx)[0].x + NM_SIGMA*((*spx)[1].x - (*spx)[0].x);
			(*spx)[1].y = (*spx)[0].y + NM_SIGMA*((*spx)[1].y - (*spx)[0].y);
			(*spx)[2].x = (*spx)[0].x + NM_SIGMA*((*spx)[2].x - (*spx)[0].x);
			(*spx)[2].y = (*spx)[0].y + NM_SIGMA*((*spx)[2].y - (*spx)[0].y);
			return 5.0;
		}
	}
}

void weightedAverage(){
	float wi;
	float wc;
	Pattern p;
	p.x = 0;
	p.y = 0;
	uint8_t maxDegree;
	wc = 1.0;
	uint8_t degree = 0;
	PatternNode* tmp = nbrPatternRoot;
	
	// count the number of nodes in the the list
	// this will be my degree
	while(tmp != NULL){
		degree += 1;
		tmp = tmp->next;
	}
	me.degree = degree;
	printf("My updated degree: %hu\r\n", degree);
	
	if (nbrPatternRoot == NULL){
		printf("\tNo, pattern Node not in the list!!!\r\n");
	}else{
		printf("\tYes, pattern Node in the list!!!\r\n");
	}
	// weighted averaging using Metropolis weights
	while(nbrPatternRoot != NULL){
		printf("\tPattern: [%0.4f, %0.4f]\r\n", nbrPatternRoot->p.x, nbrPatternRoot->p.y);
		maxDegree = me.degree;
		if (maxDegree < nbrPatternRoot->degree){
			maxDegree = nbrPatternRoot->degree;
		}
		wi = 1.0/(1.0+maxDegree);
		wc -= wi;
		p.x += wi*nbrPatternRoot->p.x;
		p.y += wi*nbrPatternRoot->p.y;
		tmp = nbrPatternRoot->next;
		myFree(nbrPatternRoot);
		nbrPatternRoot = tmp;
	}
	lastPatternAdded = NULL;
	
	p.x += wc*me.p.x;
	p.y += wc*me.p.y;
	
	if (TEST_CONSENSUS){
		printf("Pre-pattern: [%0.4f, %0.4f] Cur-pattern: [%0.4f, %0.4f]\r\n",
		me.p.x, me.p.y, p.x, p.y);
	}
	me.p.x = p.x; 
	me.p.y = p.y;
}

#if ONLY_ACTIVE_BOTS_SEND_TURING_MSG

void updateTuringColor(){
	float ss = 0.0f;
	if (me.turingColor == 1){
		me.nA += 1;
	}
	turingHistory[frameCount][0] = me.turingColor;
	turingHistory[frameCount][1] = me.nA;
	turingHistory[frameCount][2] = me.nI;

	ss += (float)me.nA - (float)me.nI*TURING_F;
	if (ss > 0){
		me.turingColor = 1;
		}else if (ss < 0){
		me.turingColor = 0;
	}

	me.nA = 0;
	me.nI = 0;
	
	if (TEST_TURING) {
		printf("\tTuring color: %hu [%hu, %hu]\r\n", me.turingColor, me.nA, me.nI);
	}
}

#else

// Change me.turing_color according to Young's model
// the neighbors' colors are also added to turingHistory array for record
void updateTuringColor(){
	me.nA = 0;
	me.nI = 0;	
	float ss = 0.0f;
	TuringNode* tmp = NULL;
	
	// count nA
	if (me.turingColor == 1){
		me.nA += 1;
	}	
	printf("Activator:\r\n");
	printf("\tX[%04X]: %hu\r\n", me.dropletId, me.turingColor);
	tmp = turingRoot_a;
	while(tmp != NULL)
	{
		printf("\t[%04X]: %hu\r\n", tmp->id, tmp->t_color);
		if(tmp->t_color == 1){
			me.nA += 1;
		}
		tmp = tmp->next;
	}

	// count nI
	printf("Inhibitor:\r\n");
	tmp = turingRoot_i;
	while(tmp != NULL)
	{
		printf("\t[%04X]: %hu\r\n", tmp->id, tmp->t_color);
		if(tmp->t_color == 1){
			me.nI += 1;
		}
		tmp = tmp->next;
	}	
	
	turingHistory[frameCount][0] = me.turingColor;
	turingHistory[frameCount][1] = me.nA;
	turingHistory[frameCount][2] = me.nI;

	ss += (float)me.nA - (float)me.nI*TURING_F;
	if (ss > 0){
		me.turingColor = 1;
	}else if (ss < 0){
		me.turingColor = 0;
	}
	
	if (TEST_TURING) {
		printf("\tTuring color: %hu [%hu, %hu]\r\n", me.turingColor, me.nA, me.nI);
	}
}

#endif

void printRGBs_ordered(){
	printf("\r\nPrint all rgbs read (ordered)\r\n");
	for (uint8_t i=0; i<NUM_LOCALIZE; i++){
		printf("\t%hu: %d %d %d\r\n", i, red_array[i], green_array[i], blue_array[i]);
	}
}

void printPos(){
	printf("\r\n\tMy Pos: % 4d, % 4d, %4d\r\n", myPos.x, myPos.y, myPos.o);
}

void printTuringInfo(){
	printf("\r\nPrint turing info\r\n");
	for (uint8_t i=0; i<NUM_TURING; i++){
		printf("\t%hu: color: %hu [%hu, %hu]\r\n", i, turingHistory[i][0], turingHistory[i][1], turingHistory[i][2]);
	}
	#if (!ONLY_ACTIVE_BOTS_SEND_TURING_MSG)	
	TuringNode* tmp = NULL;
	printf("Activator:\r\n");
	printf("\tX[%04X]: %hu\r\n", me.dropletId, me.turingColor);
	tmp = turingRoot_a;
	while(tmp != NULL)
	{
		printf("\t[%04X]: %hu\r\n", tmp->id, tmp->t_color);
		tmp = tmp->next;
	}

	// count nI
	printf("Inhibitor:\r\n");
	tmp = turingRoot_i;
	while(tmp != NULL)
	{
		printf("\t[%04X]: %hu\r\n", tmp->id, tmp->t_color);
		tmp = tmp->next;
	}
	#endif
}

uint8_t user_handle_command(char* command_word, char* command_args){
	if(strcmp(command_word, "pc")==0){
		printRGBs_ordered();
		return 1;
	}
	if(strcmp(command_word, "pp")==0){
		printPos();
		return 1;
	}
	if(strcmp(command_word, "pt")==0){
		printTuringInfo();
		return 1;
	}
	if(strcmp(command_word, "pa")==0){
		printPos();
		printRGBs_ordered();
		printOtherBots();
		printTuringInfo();
		
		printMatrixMathematica(&(me.patternTransformA));
		printMatrixMathematica(&(me.patternTransformI));
		return 1;
	}
//
//if(strcmp(command_word, "set_thresh")==0){
	//threshold_mottled = atoi(command_args);
//}
//
	return 0;
}

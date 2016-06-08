/*
* consensusAlg.c
* For Camouflage Project
* Created: 5/25/2016 9:04:54 PM
* Author : Yang Li and Joe Jackson
* Description:


Algorithmic Description:

INITIALIZATION:
Project one of the three patterns on Droplets
Each droplet senses the color on it, and store it

GRADIENT PHASE:
Each frame, if its slot: broadcast to its neighbors where it is (distance and bearing)
Each frame, if not its slot: try to receive messages (color) from other Droplet, and store them in four_Direction_Array
Each frame, during the last idle slot: Compute the gradients in each direction and update the guess of pattern

CONSENSUS PHASE:
Each frame, if its slot: broadcast to its neighbors
Each frame, if not its slot: try to receive messages (guessed pattern) from other Droplet, and store them
Each frame, during the last idle slot: Try to make consensus on which the most possible pattern to apply

TURING PATTERN PHASE: [To be detailed]
Generate a pattern and decide which color each Droplet should have


*/

#include "consensusAlg.h"
#define TEST 1
/*
* Any code in this function will be run once, when the robot starts.
*/
void init()
{
	frameCount = 0;
	loopID = 0xFFFF;
	// initialize the degree of the Droplet
	// at the beginning it only knows that he
	myDegree = 1;
	countNeighbor = 1;
	
	// from gradient phase
	phase = 0;
	countGradient = 0;
	countConsensus = 0;
	
	uint16_t red_led;
	uint16_t green_led;
	uint16_t blue_led;

	// Yang: sense the projected color
	get_rgb(&red_led,&green_led,&blue_led);
	myGRB.RGB[0] = red_led ;
	myGRB.RGB[1] = green_led;
	myGRB.RGB[2] = blue_led;
	
	myGRB.flag = HIST_MSG_FLAG;
	myGRB.droplet_ID = get_droplet_id();
	// initialize the four direction with it own RGB
	for(uint8_t i=0; i<NUM_DIRS; i++){
		fourDirRGB[i] = myGRB;
	}
	
	//ensure each Droplet sends messages at its own slot
	mySlot = get_droplet_order_camouflage(get_droplet_id());
	frameStart = get_time();
	printf("Init-Camouflage Project. mySlot is %003d\r\n", mySlot);
	if(TEST)
	{
		printf("myRGB: %03u %03u %03u\r\n", myGRB.RGB[0], myGRB.RGB[1], myGRB.RGB[2]);
	}
}

/*
* The code in this function will be called repeatedly, as fast as it can execute.
*/
/************************************************************************/
/*
*/
/************************************************************************/
void loop()
{
	switch (phase){
		case 0: gradientPhase(); break;
		case 1: consensusPhase(); break;
		case 2: turingPhase(); break;
		default: break;
	}
}

void gradientPhase(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime > FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
		printf("[Gradient Phase]\tCurrent frame No. is %u\r\n", frameCount);
	}
	
	/*code here executes once per slot.*/
	// The first condition is to ensure this
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		printf("Current loopID is %03u\r\n", loopID);
		if(loopID==mySlot)
		{
			/* Do stuff. send messages. do rnb broadcast. */
			broadcast_rnb_data();
			set_rgb(255, 0, 0);
			sendRGBMsg();
		}
		else if(loopID==SLOTS_PER_FRAME-1)
		{
			/* End of frame. Might do some final processing here? */
			set_rgb(0, 255, 0);
			// Compute two gradients and decide which pattern
			// At this point, only use Red channel
			int16_t diff_row, diff_col, diff;
			diff_row = abs(2*myGRB.RGB[0] - fourDirRGB[0].RGB[0] - fourDirRGB[1].RGB[0]);
			diff_col = abs(2*myGRB.RGB[0] - fourDirRGB[2].RGB[0] - fourDirRGB[3].RGB[0]);
			
			if( 0 ) // row less than col
			{
				myPattern = 0;
			}
			else if (0) // col less than row
			{
				myPattern = 1;
			}
			else{ // row and col are similar
				myPattern = 2;
			}
			
			countGradient++;
			if(countGradient > NUM_GRADIENT){
				phase++;
			}
		}
		else
		{
			led_off();
		}
		
	}
	
	/* code here executes once per loop. */
	if(rnb_updated){
		if(last_good_rnb.conf > 1.0){
			printf("ID: %04X Rang: %0.4f\r\n", last_good_rnb.id_number, last_good_rnb.range);
			float bearing = last_good_rnb.bearing;
			if(bearing>PI/3.0 && bearing<PI*2.0/3.0) // left
			{
				fourDirDroplet[0].droplet_ID = last_good_rnb.id_number;
				fourDirDroplet[0].range = last_good_rnb.range;
				fourDirDroplet[0].bearing = last_good_rnb.bearing;
			}
			else if(bearing<-PI/3.0 && bearing>-PI*2.0/3.0) // right
			{
				fourDirDroplet[1].droplet_ID = last_good_rnb.id_number;
				fourDirDroplet[1].range = last_good_rnb.range;
				fourDirDroplet[1].bearing = last_good_rnb.bearing;
			}
			else if(bearing>-PI/6.0 && bearing<PI/6.0) // top
			{
				fourDirDroplet[2].droplet_ID = last_good_rnb.id_number;
				fourDirDroplet[2].range = last_good_rnb.range;
				fourDirDroplet[2].bearing = last_good_rnb.bearing;
			}
			else if(bearing>5.0*PI/6.0 || bearing<-5.0*PI/6.0) // bottom
			{
				fourDirDroplet[3].droplet_ID = last_good_rnb.id_number;
				fourDirDroplet[3].range = last_good_rnb.range;
				fourDirDroplet[3].bearing = last_good_rnb.bearing;
			}
		}

		rnb_updated = 0;
	}
	
	/* Define the duration of loop */
	delay_ms(LOOP_DELAY_MS);
}

void consensusPhase(){

	uint32_t frameTime = get_time()-frameStart;
	if(frameTime > FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
		printf("[Consensus Phase]\tCurrent frame No. is %u\r\n", frameCount);
	}
	
	/*code here executes once per slot.*/
	// The first condition is to ensure this
	if(loopID!=(frameTime/SLOT_LENGTH_MS) && 0){
		loopID = frameTime/SLOT_LENGTH_MS;
		printf("Current loopID is %03u\r\n", loopID);
		if(loopID==mySlot)
		{
			/* Do stuff. send messages. do rnb broadcast. */
			broadcast_rnb_data();
			set_rgb(myGRB.RGB[0], myGRB.RGB[1], myGRB.RGB[2]);
			
			// store current histogram to the neighbor_hists at index 0
			
		}
		else if(loopID==SLOTS_PER_FRAME-1)
		{
			/* End of frame. Might do some final processing here? */
			set_rgb(0, 255, 0);
			// Compute weights, and do the weighted average
			float weights[countNeighbor];
			uint16_t maxDegree;
			float sumWeights = 0.0;
			for(uint8_t i=1; i<countNeighbor; i++){
				maxDegree = neighborHist[0].degree;
				if(maxDegree < neighborHist[i].degree){
					maxDegree = neighborHist[i].degree;
				}
				weights[i] = 1.0/(1+maxDegree);
				sumWeights += weights[i];
			}
			weights[0] = 1.0-sumWeights;
			
			float sumHist;
			for(uint8_t i=0; i<NUM_PATTERNS; i++){
				sumHist = 0.0;
				for(uint8_t j=0; j<countNeighbor; j++){
					sumHist += (float)(weights[j]*neighborHist[j].patterns[i]/65535.0);
				}
				prePatternHist[i] = curPatternHist[i];
				curPatternHist[i] = sumHist;
			}
			
			if(TEST)
			{
				// Test: is neighbor right?
				printf("Neighbor size: %u\r\n",countNeighbor);
				for(uint8_t i=0; i<countNeighbor; i++){
					printf("Weight %u: [%0.2f]\t", i, weights[i]);
				}
				printf("\r\n");
				
				for(uint8_t i=0; i<NUM_PATTERNS; i++){
					printf("Color %u: [%0.2f]\t", i, curPatternHist[i]);
				}
				printf("\r\n");
			}
			
			// reset the degree and number of neighbors to start a new frame
			myDegree = 1;
			countNeighbor = 1;
			
			countConsensus++;
			if(countConsensus > NUM_CONSENSUS){
				phase++;
			}			
		}
		else
		{
			led_off();
		}
		
	}
	
	/* code here executes once per loop. */
	if(rnb_updated){
		if(last_good_rnb.conf > 1.0){
			last_good_rnb.range;
			last_good_rnb.bearing;
			last_good_rnb.heading;
			//do stuff!
			printf("ID: %04X Rang: %0.4f\r\n", last_good_rnb.id_number, last_good_rnb.range);
			myDegree++;
		}
		rnb_updated = 0;
	}
	
	/* Define the duration of loop */
	delay_ms(LOOP_DELAY_MS);
}

void turingPhase(){
	
}


void sendRGBMsg(){
	rgbMsg msg;
	msg = myGRB;
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(rgbMsg));
}

void sendGradientMsg(){
	patternMsg msg;
	msg.degree = myDegree;
	msg.flag = HIST_MSG_FLAG;
	for(uint8_t i=0;i<NUM_PATTERNS;i++){
		// normalize float value [0, 1] to [0, 65535], easier to transmit message
		msg.patterns[i] = (uint16_t)(curPatternHist[i]*65535);
	}
	
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(patternMsg));
}

/*
* After each pass through loop(), the robot checks for all messages it has
* received, and calls this function once for each message.
*/
void handle_msg(ir_msg* msg_struct)
{
	rgbMsg* rgbmsg;
	patternMsg* ptmsg;
	switch (phase){
		case 0: // gradient phase
		rgbmsg = (rgbMsg**)(msg_struct->msg);
		if(rgbmsg->flag == HIST_MSG_FLAG){
			// store the sender ID
			for (uint8_t i=0; i<NUM_DIRS; i++)
			{
				if (rgbmsg->droplet_ID == fourDirDroplet[i].droplet_ID)
				{
					fourDirRGB[i].droplet_ID = rgbmsg->droplet_ID;
					for (uint8_t j=0; j<3; j++)
					{
						fourDirRGB[i].RGB[j] = rgbmsg->RGB[j];
					}
					break;
				}
			}
		}
		break;
		
		case 1: // consensus phase
		ptmsg = (patternMsg*)(msg_struct->msg);
		if(ptmsg->flag == HIST_MSG_FLAG){
			// store the sender ID
			neighborHist[countNeighbor].droplet_ID = msg_struct->sender_ID;
			// store the new ptmsg to neighbor_hists
			neighborHist[countNeighbor].degree = ptmsg->degree;
			for (uint8_t i=0; i<NUM_PATTERNS; i++){
				neighborHist[countNeighbor].patterns[i] = ptmsg->patterns[i];
			}
			countNeighbor ++;
			if (countNeighbor > NUM_DROPLETS) // in case of error
			{
				printf("There is overflow of storing ptmsg from neighbors, myID: %04X\r\n",
				get_droplet_id());
			}
		}		
		break;
		
		case 2: 
		break;
		
		default: 
		break;
	}

}



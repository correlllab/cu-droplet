/*
 * consensusAlg.c
 * For Camouflage Project
 * Description: 
 *
 * Created: 5/25/2016 9:04:54 PM
 * Author : Yang Li and Joe Jackson
 */ 

#include "consensusAlg.h"

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
	counter_neighbor = 1;
	
	//initialize the histogram of the Droplet with a random color
	for(uint8_t i=0; i<NUM_BINS; i++){
		curHistogram[i] = 0.0;
		oriHistogram[i] = 0.0;
		preHistogram[i] = 0.0;
	}
	
	// 
	uint8_t rand_color = rand_byte()%NUM_BINS;
	oriHistogram[rand_color] = 1.0;
	preHistogram[rand_color] = 1.0;
	curHistogram[rand_color] = 1.0;
	
	//ensure each Droplet sends messages at its own slot
	mySlot = get_droplet_order_camouflage(get_droplet_id());
	frameStart = get_time();
	printf("Init-Camouflage Project. mySlot is %003d\r\n", mySlot);
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
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime > FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
		printf("Current frame No. is %u\r\n", frameCount);
	}
	
	/*code here executes one per slot.*/
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		printf("Current loopID is %03u\r\n", loopID);
		if(loopID==mySlot)
		{
			/* Do stuff. send messages. do rnb broadcast. */
			broadcast_rnb_data();
			set_rgb(255, 0, 0);
			sendHistMsg();
		}
		else if(loopID==SLOTS_PER_FRAME-1)
		{
			/* End of frame. Might do some final processing here? */
			set_rgb(0, 255, 0);
			// Compute weights, and do the weighted average
			float weights[counter_neighbor];
			uint16_t maxDegree;
			float sumWeights = 0.0;
			for(uint8_t i=1; i<counter_neighbor; i++){
				maxDegree = neighbor_hists[0].degree;
				if(maxDegree < neighbor_hists[i].degree){
					maxDegree = neighbor_hists[i].degree;
				}
				weights[i] = 1.0/(1+maxDegree);
				sumWeights += weights[i];
			}
			weights[0] = 1.0-sumWeights;
			
			float sumHist;
			for(uint8_t i=0; i<NUM_BINS; i++){
				sumHist = 0.0;
				for(uint8_t j=0; j<counter_neighbor; j++){
					sumHist += (float)(weights[j]*1.0*neighbor_hists[j][i]/65535.0)
				}
				preHistogram[i] = curHistogram[i];
				curHistogram[i] = sumHist;
			}
			// reset the degree to start a new frame
			myDegree=1;
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

void sendHistMsg(){
	HistMsg msg;
	msg.flag = HIST_MSG_FLAG;
	msg.degree = myDegree;
	for(uint8_t i=0;i<NUM_BINS;i++){
		// normalize float value [0, 1] to [0, 65535], easier to transmit message
		msg.hist[i] = (uint16_t)(curHistogram[i]*65535);
	}
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(HistMsg));

	//char txtMsg[6] = "Hello.";
	//ir_send(ALL_DIRS, &txtMsg, 6);
}

/*
* After each pass through loop(), the robot checks for all messages it has
* received, and calls this function once for each message.
*/
void handle_msg(ir_msg* msg_struct)
{
	HistMsg* msg = (HistMsg*)(msg_struct->msg);
	if(msg->flag == HIST_MSG_FLAG){
		// store the sender ID
		neighbor_hists[counter_neighbor].dropletID = msg_struct->sender_ID;
		// store the new histmsg to neighbor_hists
		neighbor_hists[counter_neighbor].degree = msg->degree;
		for (uint8_t i=0; i<NUM_BINS; i++){
			neighbor_hists[counter_neighbor].hist[i] = msg->hist[i];
		}
		counter_neighbor ++;
		if (counter_neighbor > NUM_DROPLETS) // in case of error
		{
			printf("There is overflow of storing histmsg from neighbors, myID: %04X\r\n", 
			get_droplet_id());
		}
	}
}



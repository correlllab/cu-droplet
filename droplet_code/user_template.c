#include "user_template.h"

//This function is called once, after all of the Droplet's systems have been initialized.
void init(){
	clearRoles(&myRoles);
	isBlinking = 0;
	frameStart = getTime();
	mySlot = getSlot(getDropletID());	
	
	#ifdef AUDIO_DROPLET
		enableMicInterrupt();
	#endif	
}

/*
 * This function is called repeatedly, as fast as it can. Note that this droplet can only
 * receive new rnb measurements or ir messages after this function returns. Things work
 * better if you let it return frequently.
 */
void loop(){
	uint32_t frameTime = getTime()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
	}
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){ //This is a new slot.
		loopID = frameTime/SLOT_LENGTH_MS;	
		if(loopID==mySlot){ //This is my slot.
			broadcastRnbData();			
		}else if(loopID==SLOTS_PER_FRAME-1){
			if(POS_DEFINED(&myPos)){
				printf("\tMy Pos: {%d, %d, %d}\r\n", myPos.x, myPos.y, myPos.o);
				printPosCovar(&myPosCovar);
				
				checkPosition();
				printRoles(&myRoles);
			}
		}
	}
	delayMS(LOOP_DELAY_MS);	
}

/*
 * This function is called once for every range and bearing measurement this droplet has
 * received since the last time loop returned.
 */
void handleMeas(Rnb* meas){
	useRNBmeas(meas);
}

/*
 * This function is called once for every message this droplet has received since the last
 * time loop returned, after handleMeas is called for any rnb measurements received.
 */
void handleMsg(irMsg* msgStruct){

}

/*
 * The two functions below are optional; they do not have to be defined. If they are defined, 
 * they will be called in response to the appropriate events.
 
 optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection.
 */

/* If defined, this function will be called when the microphone detects a sharp rising edge.
 * In practice, this works well for things like detecting claps or someone tapping on the 
 * Droplet's shell.
 */
void userMicInterrupt(){
	blinkLED(0,100);
}

/*
 * If defined, this function will be called with any serial commandWords that do not match
 * other commands serial_handler.c checks for. See the serial_handler documentation for
 * details on commandWord and commandArgs.
 */
//uint8_t userHandleCommand(char* commandWord, char* commandArgs){}

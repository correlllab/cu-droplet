#include "user_template.h"

//This function is called once, after all of the Droplet's systems have been initialized.
void init(){
	clearRoles(&myRoles);
	isBlinking = 0;
	lastMsgTime = getTime();
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

void sendButtonPressMsg(id_t id, Role r, uint8_t hopsLeft){
	if((hopsLeft==0) || (getTime()-lastMsgTime < ANTI_FLOOD_DELAY)){
		blinkLED(0, 100); //Won't send yet.
		return;
	}else{
		blinkLED(1, 100); //Sending!
		lastMsgTime = getTime();
	}
	ButtonPressMsg msg;
	msg.src = id;
	msg.pressType = r;
	msg.hopLife = hopsLeft;
	msg.flag = BUTTON_PRESS_MSG_FLAG;
	irSend(ALL_DIRS, (char*)(&msg), sizeof(ButtonPressMsg));
}

/*
 * This function is called once for every range and bearing measurement this droplet has
 * received since the last time loop returned.
 */
void handleMeas(Rnb* meas){
	useRNBmeas(meas);
}

void controlTV(Role r){
	static const uint8_t addr = 0;
	uint8_t cmd;
	switch(r){
		case  ROLE_POWER: cmd = 0x08;
		case ROLE_CHN_UP: cmd = 0x00;
		case ROLE_CHN_DN: cmd = 0x01;
		case ROLE_VOL_UP: cmd = 0x02;
		case ROLE_VOL_DN: cmd = 0x03;
		default:		  cmd = 0xAA; //info
	}
	sendIRBurst(1, addr, cmd);
}

/*
 * This function is called once for every message this droplet has received since the last
 * time loop returned, after handleMeas is called for any rnb measurements received.
 */
void handleMsg(irMsg* msgStruct){
	if(IS_BUTTON_PRESS_MSG(msgStruct)){
		ButtonPressMsg* msg = (ButtonPressMsg*)msgStruct;
		      if(hasRole(&myRoles,ROLE_EMTR)){
			controlTV(msg->pressType);
		}else if(hasRole(&myRoles,ROLE_BODY)){ //If we are part of the body.
			sendButtonPressMsg(msg->src, msg->pressType, msg->hopLife-1);
		}
	}
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
	Role assocRole;
	if(hasRole(&myRoles, ROLE_POWER)){
		assocRole = ROLE_POWER;	
	}else if(hasRole(&myRoles,ROLE_CHN_UP)){
		assocRole = ROLE_CHN_UP;
	}else if(hasRole(&myRoles,ROLE_CHN_DN)){
		assocRole = ROLE_CHN_DN;
	}else if(hasRole(&myRoles,ROLE_VOL_UP)){
		assocRole = ROLE_VOL_UP;
	}else if(hasRole(&myRoles,ROLE_VOL_DN)){
		assocRole = ROLE_VOL_DN;
	}else{
		return;
	}
	if(hasRole(&myRoles, ROLE_EMTR)){
		controlTV(assocRole);
	}else{
		sendButtonPressMsg(getDropletID(),  assocRole, 3);
	}
}

/*
 * If defined, this function will be called with any serial commandWords that do not match
 * other commands serial_handler.c checks for. See the serial_handler documentation for
 * details on commandWord and commandArgs.
 */
//uint8_t userHandleCommand(char* commandWord, char* commandArgs){}

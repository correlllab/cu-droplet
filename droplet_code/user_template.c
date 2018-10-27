#include "user_template.h"

//This function is called once, after all of the Droplet's systems have been initialized.
void init(){
	clearRoles(&myRoles);
	isBlinking = 0;
	lastMsgTime = getTime();
	frameStart = getTime();
	lastTapDetectedTime = getTime();
	
	mySlot = getSlot(getDropletID());	
	
	#ifdef AUDIO_DROPLET
		enableMicInterrupt();
	#endif	
	if(seedFlag){
		checkPosition();
		printRoles(&myRoles);
		printf("\r\n");
	}
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
			if(!seedFlag) broadcastRnbData();			
		}else if(loopID==SLOTS_PER_FRAME-1){
			if(POS_DEFINED(&myPos)){
				printf("\tMy Pos: {%d, %d, %d}\r\n", myPos.x, myPos.y, myPos.o);
				Matrix mat;
				decompressP(&mat, &myPosCovar);
				Vector eigVals;
				eigenvalues(&eigVals, &mat);
				if(sqrt(eigVals[0]))
				printPosCovar(&myPosCovar);
				
				checkPosition();
				printRoles(&myRoles);
				printf("\r\n");
			}
		}
		getMyColor();
	}
	delayMS(LOOP_DELAY_MS);	
}

void getMyColor(){
	if(isBlinking) return;
	if(hasRole(&myRoles,ROLE_EMTR)){
		setRGB(0,0,0);
	}else if(hasRole(&myRoles, ROLE_POWER)){
		setRGB(20,40,20);
	}else if(hasRole(&myRoles,ROLE_CHN_UP)){
		setRGB(0,50,30);
	}else if(hasRole(&myRoles,ROLE_CHN_DN)){
		setRGB(0,30,50);
	}else if(hasRole(&myRoles,ROLE_VOL_UP)){
		setRGB(50,30,0);
	}else if(hasRole(&myRoles,ROLE_VOL_DN)){
		setRGB(30,50,0);
	}else if(hasRole(&myRoles,ROLE_MUTE)){
		setRGB(24,28,28);
	}else{
		setRGB(27,27,27);
	}
}

void sendButtonPressMsg(id_t id, Role r, uint8_t hopsLeft){
	if((hopsLeft==0) || (getTime()-lastMsgTime < ANTI_FLOOD_DELAY)){
		return;
	}else{
		lastMsgTime = getTime();
	}
	ButtonPressMsg msg;
	msg.src = id;
	msg.pressType = r;
	msg.hopLife = 0;
	msg.flag = BUTTON_PRESS_MSG_FLAG;
	irSend(DIR0|DIR2|DIR4, (char*)(&msg), sizeof(ButtonPressMsg));
}

/*
 * This function is called once for every range and bearing measurement this droplet has
 * received since the last time loop returned.
 */
void handleMeas(Rnb* meas){
	printf("\t(RNB) ID: %04X | R: %4u B: %4d H: %4d\r\n", meas->id, meas->range, meas->bearing, meas->heading);
	useRNBmeas(meas);
}

void controlTV(Role r){
	blinkLED(0,50);
	static const uint8_t addr = 0x20;
	uint8_t cmd;
	switch(r){
		case  ROLE_POWER: cmd = 0x10; break;
		case ROLE_CHN_UP: cmd = 0x00; break;
		case ROLE_CHN_DN: cmd = 0x80; break;
		case ROLE_VOL_UP: cmd = 0x40; break;
		case ROLE_VOL_DN: cmd = 0xC0; break;
		case ROLE_MUTE:	  cmd = 0x90; break;
		default:		  cmd = 0xAA; //info
	}
	printf("Sending %02hX to %02hX.\r\n",cmd, addr);
	sendIRBurst(addr, cmd);
}

/*
 * This function is called once for every message this droplet has received since the last
 * time loop returned, after handleMeas is called for any rnb measurements received.
 */
void handleMsg(irMsg* msgStruct){
	if(IS_BOT_MEAS_MSG(msgStruct)){
		handleBotMeasMsg((BotMeasMsg*)(msgStruct->msg), msgStruct->senderID);
	}else if(IS_BUTTON_PRESS_MSG(msgStruct)){
		ButtonPressMsg* msg = (ButtonPressMsg*)(msgStruct->msg);
		if(hasRole(&myRoles,ROLE_EMTR)){			  
			printf("%02hX\r\n", msg->pressType);
			controlTV(msg->pressType);
		}else if(hasRole(&myRoles,ROLE_BODY)){ //If we are part of the body.
			sendButtonPressMsg(msgStruct->senderID, msg->pressType, msg->hopLife-1);
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
	if((getTime()-lastTapDetectedTime)<ANTI_DOUBLETAP_DELAY){
		return;	
	}
	Role assocRole;
	uint8_t hopLife=1;
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
	}else if(hasRole(&myRoles,ROLE_MUTE)){
		assocRole = ROLE_MUTE;
		hopLife = 2;
	}else{
		return;
	}
	if(hasRole(&myRoles, ROLE_EMTR)){
		controlTV(assocRole);
	}else{
		blinkLED(1,50);
		sendButtonPressMsg(getDropletID(),  assocRole, hopLife);
	}
	lastTapDetectedTime = getTime();
}




//void lessGeneralCheckPosition(){
	//float r = hypot(myPos.x,myPos.y);
	//float th = atan2(myPos.y,myPos.x);
	//if(r>100) return;
	//clearRoles(&myRoles);
	//giveRole(&myRoles,ROLE_BODY);
	//if(r<30){
		//giveRole(&myRoles,ROLE_POWER);		
		//return;
	//}
	//uint8_t regionIdx = conversionFunc(radToDeg(th));
	//switch(regionIdx){
		//case 0: giveRole(&myRoles,ROLE_CHN_DN); break;
		//case 1: giveRole(&myRoles,ROLE_MUTE); break;
		//case 2: giveRole(&myRoles,ROLE_VOL_DN); break;
		//case 3: giveRole(&myRoles,ROLE_VOL_UP); break;
		//case 4: giveRole(&myRoles,ROLE_EMTR); break;
		//case 5: giveRole(&myRoles,ROLE_CHN_UP); break;		
	//}	
//}
/*
 * If defined, this function will be called with any serial commandWords that do not match
 * other commands serial_handler.c checks for. See the serial_handler documentation for
 * details on commandWord and commandArgs.
 */
//uint8_t userHandleCommand(char* commandWord, char* commandArgs){}

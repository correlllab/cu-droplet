#include "user_template.h"
//#include <inttypes.h>
//#include <stdio.h>


uint32_t lastMessageSent;
//#define MSG_PERIOD 400
#define MAX_RECV_COUNT 100

void sendMsg(void);

uint16_t recvCount;

typedef struct droplet_messages_struct{
	uint16_t initMsgID;
	uint16_t finalMsgID;
	uint16_t msgCount;
	float success_rate;
}DropletMessages;
DropletMessages msgLog[121];

void startListening(){
	dataCollecting = 1;
}

void startTransmitting(){
	startSending = 1;
	setRGB(20,20,20);
}

//This function is called once, after all of the Droplet's systems have been initialized.
void init(){
	setRedLED(50);
	lastMessageSent = getTime();
	msgCount=0;
	dataCollecting=0;
	startSending=0;
	scheduleTask(3000,startTransmitting,NULL);
	for(int i=0; i<121; i++)
		msgLog[i].msgCount = 0;
		
	recvCount = 0;
	MSG_PERIOD = 1100;
	MS_DROPLET_COMM_TIME = 8;

}

void setMsgPeriod(uint32_t value){
	MSG_PERIOD = value;
	
	if(value>= 700){
		setHSV((value/10),250,50);
	}
	else if(value >= 600)
	setHSV(50,250,50);
	else if(value >= 500)
	setHSV(25,250,50);
	else if(value >= 400)
	setHSV(0,250,50);
}

void sendMsg(){
	TestMsg msg;
	msg.text[0]= 'H';
	msg.text[1]='i';
	msg.text[2]='.';
	
	//msg.text="Hi.";
	msgCount = (msgCount + 1)%0x0FFF;/*%2000+4000;// + 2000;//65534;*/
	msg.msgId = msgCount; //| getBitMask(getDropletID());//++;					//RIYA
	//char* msg_str;
	//sprintf(msg_str, "Message=%s, Message_ID=%d", msg.text, msg.msgId);
	irSend(ALL_DIRS, (char*)(&msg), sizeof(TestMsg));
}

/*
 * This function is called repeatedly, as fast as it can. Note that this droplet can only
 * receive new rnb measurements or ir messages after this function returns. Things work
 * better if you let it return frequently.
 */
void loop(){

	if(startSending){
		if(getTime()-lastMessageSent > MSG_PERIOD){
			//if(getDropletID()!=RECEIVER_ID){
				sendMsg();
			
			//}
			lastMessageSent = getTime();
		}
	}
	
	delayMS(10);


}

/*
 * This function is called once for every range and bearing measurement this droplet has
 * received since the last time loop returned.
 */
void handleMeas(Rnb* meas){

}


/*
 * This function is called once for every message this droplet has received since the last
 * time loop returned, after handleMeas is called for any rnb measurements received.
 */
void handleMsg(irMsg* msgStruct){
	TestMsg* msg = (TestMsg*)(msgStruct->msg);

	volatile uint8_t msgSender = 0;
	//msgSender = (uint8_t)(log((msg->msgId)>>12)/log(2));
	msgSender = getDropletOrd(msgStruct->senderID);
	uint16_t msgID = (msg->msgId)&0x0FFF;
	
	//uint8_t dataSender = (uint8_t)(log((data->msgId)>>12)/log(2));
	if(!dataCollecting){
		return;
	}
	//if(getDropletID()!=RECEIVER_ID){
		//return;
	//}
	printf("Got %04hX %6u at %lu\r\n", getIdFromOrd(msgSender), msgID, getTime());
	DropletMessages *thisDropletLog = &msgLog[getDropletOrd(msgStruct->senderID)];

	thisDropletLog->msgCount++;
	thisDropletLog->finalMsgID = msgID;
	
	if(thisDropletLog->msgCount == 1) thisDropletLog->initMsgID = msgID;
		
	recvCount++;
	if(thisDropletLog->msgCount == MAX_RECV_COUNT)
	{ 
		float meanSuccessRate = 0;
		uint8_t numSenders =0;
		
		//for(int i=0; i<121; i++)
			//msgLog[i].msgCount = 0;
		
		printf("<|");
		for(int i=0; i<121; i++){
				
			if(msgLog[i].msgCount>0){
				numSenders++;
				msgLog[i].success_rate = (float)msgLog[i].msgCount/(float)((msgLog[i].finalMsgID-msgLog[i].initMsgID) + 1);
				meanSuccessRate+=msgLog[i].success_rate;
				//printf("{%04hX, %f}, ", getIdFromOrd(i), msgLog[i].success_rate);
				printf("\"%04hX\"-> %f, ", getIdFromOrd(i), msgLog[i].success_rate);
			}
				
		}
		
		printf("|>");
		meanSuccessRate /= numSenders;
		//printf(",\r\n %f}\r\n",meanSuccessRate);
		dataCollecting = 0;
	}
	

}

/*
 *	the function below is optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection. it should return '1' if command_word was a valid command,
 *  '0' otherwise.
 */
uint8_t userHandleCommand(char* command_word, char* command_args){
	if(strcmp_P(command_word,PSTR("period"))==0){
		const char delim[2] = " ";
	 
		char* token = strtok(command_args,delim);
		uint32_t periodValue = (uint32_t)atoi(token);
	 
		setMsgPeriod(periodValue);
		return 1;
	}else if(strcmp_P(command_word,PSTR("start_send"))==0){
		setRGB(20,20,20);
		startSending = 1;	
		return 1;
	}else if(strcmp_P(command_word,PSTR("start_listen"))==0){
		dataCollecting = 1;
		return 1;
	}else if(strcmp_P(command_word,PSTR("stop_send"))==0){
		setRGB(0,0,0);
		startSending = 0;
		return 1;
	}else if(strcmp_P(command_word,PSTR("stop_listen"))==0){
		dataCollecting = 0;
		return 1;
	}else if(strcmp_P(command_word,PSTR("set_backoff"))==0){
		const char delim[2] = " ";
		
		char* token = strtok(command_args,delim);
		uint32_t backoffValue = (uint32_t)atoi(token);
		
		MS_DROPLET_COMM_TIME = backoffValue;
		
		setRGB(20,20,20);
		delayMS(10);
		setRGB(20,20,20);
		return 1;
	}
	return 0;
}
/* The two functions below are optional; they do not have to be defined. If they are defined, 
 * they will be called in response to the appropriate events.
 
 optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection.
 */

/* If defined, this function will be called when the microphone detects a sharp rising edge.
 * In practice, this works well for things like detecting claps or someone tapping on the 
 * Droplet's shell.
 */
//void userMicInterrupt(){}

/*
 * If defined, this function will be called with any serial commandWords that do not match
 * other commands serial_handler.c checks for. See the serial_handler documentation for
 * details on commandWord and commandArgs.
 */
//uint8_t userHandleCommand(char* commandWord, char* commandArgs){}


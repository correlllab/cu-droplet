#include "user_template.h"
#include <inttypes.h>
#include <stdio.h>


uint32_t lastMessageSent;
//#define MSG_PERIOD 400
#define MAX_RECV_COUNT 100

void sendMsg(void);

// uint16_t msg028C;
// uint16_t msgFCD0;
// uint16_t msg8625;
// uint16_t msg6C6F;
// uint16_t msg5161;
// 
// uint16_t msgID028Cinit;
// uint16_t msgID028Cfinal;
// uint16_t msgID5161init;
// uint16_t msgID5161final;
// uint16_t msgIDFCD0init;
// uint16_t msgIDFCD0final;
// uint16_t msgID8625init;
// uint16_t msgID8625final;
// uint16_t msgID6C6Finit;
// uint16_t msgID6C6Ffinal;

uint16_t recvCount;
//uint8_t backoffCount;

uint8_t bincounts[10];
uint8_t bins;

typedef struct droplet_messages_struct{
	uint16_t initMsgID;
	uint16_t finalMsgID;
	uint16_t msgCount;
	float success_rate;
}DropletMessages;
DropletMessages msgLog[121];



void startListening(){
	dataCollecting = 1;
	setRGB(20,100,100);
}

void startTransmitting(){
	startSending = 1;
	setRGB(20,20,20);
}

/*
 * any code in this function will be run once, when the robot starts.
 */
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
	MSG_PERIOD = 800;
	MS_DROPLET_COMM_TIME = 8;
	//backoffCount = 1;
	bins = 10;
	for(int i=0; i<bins; i++)
		bincounts[i] = 0;

}

uint32_t getBitMask(id_t id){
	switch(id){
		//case 0x73AF: return 0x10000;
		case 0x028C: return 0x00010000;
		case 0x5161: return 0x00008000;
		case 0xFCD0: return 0x00004000;
		case 0x8625: return 0x00002000;
		case 0x6C6F: return 0x00001000;
		default: return 0;
	}
}


void setMsgPeriod(uint32_t value){
	MSG_PERIOD = value;
	
	if(value >= 600)
	setHSV(50,250,50);
	else if(value >= 500)
	setHSV(100,250,50);
	else if(value >= 400)
	setHSV(0,250,50);
}

void sendMsg(){
	Msg msg;
	msg.text[0]= 'H';
	msg.text[1]='i';
	//msg.text[2]='.';
	
	//msg.text="Hi.";
	msgCount = (msgCount + 1)%0x0FFF;/*%2000+4000;// + 2000;//65534;*/
	msg.msgId = msgCount; //| getBitMask(getDropletID());//++;					//RIYA
	//char* msg_str;
	//sprintf(msg_str, "Message=%s, Message_ID=%d", msg.text, msg.msgId);
	//msg.time_scheduled = getTime();
	msg.attempts = 0;
	irSend(ALL_DIRS, (char*)(&msg), sizeof(Msg));
}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */



void loop(){

	if(startSending){
		if(getTime()-lastMessageSent > MSG_PERIOD){
			//if(getDropletID()!=RECEIVER_ID){
			sendMsg();
			//if(backoffCount<10)
				//printf("\n\rMaximum backoff = %u",getExponentialBackoff(backoffCount++));
			
			//}
			lastMessageSent = getTime();
		}
	}
	
	delayMS(10);


}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */



void handleMsg(irMsg* msgStruct){
	Msg* msg = (Msg*)(msgStruct->msg);
	
	
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

	//printf("\n\rGot %04hX %6u at %lu\r\n. Time scheduled: %lu, time sent: %lu", getIdFromOrd(msgSender), msgID, getTime(), msg->time_scheduled, msg->time_sent);
	printf("Got %04hX %6u at %lu. Attempt no.:%u\r\n", getIdFromOrd(msgSender), msgID, getTime(),msg->attempts);
	bincounts[msg->attempts]++;
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
		for(uint8_t i=0; i<121; i++){
			
			if(msgLog[i].msgCount>0){
				numSenders++;
				msgLog[i].success_rate = (float)msgLog[i].msgCount/(float)((msgLog[i].finalMsgID-msgLog[i].initMsgID) + 1);
				meanSuccessRate+=msgLog[i].success_rate;
				//printf("{%04hX, %f}, ", getIdFromOrd(i), msgLog[i].success_rate);
				printf("\"%04hX\"-> %f, ", getIdFromOrd(i), msgLog[i].success_rate);
			}
			
		}
		
		printf("|>");
		
		printf("\n\rbincounts3 = {");
		for(int i=0;i<bins-1;i++)
			printf("%u,",bincounts[i]);
		printf("%u};",bincounts[bins-1]);
		printf("\n\rbins3 = Range[0,%u];",bins-1);

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


//WROTE THIS RIGHT NOW
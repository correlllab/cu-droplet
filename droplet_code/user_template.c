#include "user_template.h"
#include <inttypes.h>
#include <stdio.h>


uint32_t lastMessageSent;
//#define MSG_PERIOD 400
#define MAX_RECV_COUNT 500

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

typedef struct droplet_messages_struct{
	uint16_t initMsgID;
	uint16_t finalMsgID;
	uint16_t msgCount;
	float success_rate;
}DropletMessages;
DropletMessages msgLog[121];

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	setRedLED(50);
	lastMessageSent = getTime();
	msgCount=0;

	for(int i=0; i<121; i++)
		msgLog[i].msgCount = 0;
	//msgFCD0 = 0;
	//msg8625 = 0;
	//msg6C6F = 0;
	//msg5161 = 0;
	
	recvCount = 0;
	MSG_PERIOD = 400;

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
	Msg msg;
	msg.text[0]= 'H';
	msg.text[1]='i';
	msg.text[2]='.';
	
	//msg.text="Hi.";
	msgCount = (msgCount + 1)%0x0FFF;/*%2000+4000;// + 2000;//65534;*/
	msg.msgId = msgCount; //| getBitMask(getDropletID());//++;					//RIYA
	//char* msg_str;
	//sprintf(msg_str, "Message=%s, Message_ID=%d", msg.text, msg.msgId);
	irSend(ALL_DIRS, (char*)(&msg), sizeof(Msg));
}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){	
	//float new_bearing, new_heading;
	//uint16_t new_steps;
	if(getTime()-lastMessageSent > MSG_PERIOD){
		if(getDropletID()!=0x3F9D){
			sendMsg();
			//sendMsg();
			//sendMsg();
			//sendMsg();
		}
		//sendMsg();
		lastMessageSent = getTime();
	
	}
	
	
	delayMS(10);
}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */



void handleMsg(irMsg* msgStruct){
	
//	
	Msg* msg = (Msg*)(msgStruct->msg);

	volatile uint8_t msgSender = 0;
	//msgSender = (uint8_t)(log((msg->msgId)>>12)/log(2));
	msgSender = getDropletOrd(msgStruct->senderID);
	uint16_t msgID = (msg->msgId)&0x0FFF;
	
	//uint8_t dataSender = (uint8_t)(log((data->msgId)>>12)/log(2));

	
	if(getDropletID()==0x3F9D){
		printf("Got %01hu %6u at %lu\r\n", msgSender, msgID, getTime());
		
		
		
		//if(msgSender == 4)	{++msg028C;	msgID028Cfinal = msgID; }
		//else if(msgSender == 3)	{++msg5161;	msgID5161final = msgID; }
		//else if(msgSender == 2)	{++msgFCD0;	msgIDFCD0final = msgID; }
		//else if(msgSender == 1)	{++msg8625;	msgID8625final = msgID; }
		//else if (msgSender == 0) {++msg6C6F; msgID6C6Ffinal = msgID; }
			
		msgLog[getDropletOrd(msgStruct->senderID)].msgCount++;
		msgLog[getDropletOrd(msgStruct->senderID)].finalMsgID = msgID;
		
		//if(msgSender==4 && msg028C == 1) msgID028Cinit = msgID;
		//if(msgSender==3 && msg5161 == 1) msgID5161init = msgID;
		//if(msgSender==2 && msgFCD0 == 1) msgIDFCD0init = msgID;
		//if(msgSender==1 && msg8625 == 1) msgID8625init = msgID;
		//if(msgSender==0 && msg6C6F == 1) msgID6C6Finit = msgID;
		
		if(msgLog[getDropletOrd(msgStruct->senderID)].msgCount == 1) msgLog[getDropletOrd(msgStruct->senderID)].initMsgID = msgID;
		
		recvCount++;
		if(recvCount == MAX_RECV_COUNT)
		{ 
			
			//printf("\n7D78: FirstMsg - LastMsg + 1: %u  Total messages: %u", (msgID7d78final-msgID7d78init) + 1, msg7d78);
			//printf("\n4db0: FirstMsg - LastMsg + 1: %u  Total messages: %u", (msgID4db0final-msgID4db0init) + 1, msg4db0);
			//printf("\n43ba: FirstMsg - LastMsg + 1: %u  Total messages: %u", (msgID43bafinal-msgID43bainit) + 1, msg43ba);
			//float success_rate[121];
			//float success_rate_FCD0;
			//float success_rate_8625;
			//float success_rate_6C6F;
			//float success_rate_5161;
			
			for(int i=0; i<121; i++){
				
				if(msgLog[i].msgCount>0){
					msgLog[i].success_rate = (float)msgLog[i].msgCount/(float)((msgLog[i].finalMsgID-msgLog[i].initMsgID) + 1);
					printf("%u, %f ", i, msgLog[i].success_rate);
				}
				
			}
			
			//success_rate_FCD0 = (float)msgFCD0/(float)((msgIDFCD0final-msgIDFCD0init) + 1);
			//success_rate_8625 = (float)msg8625/(float)((msgID8625final-msgID8625init) + 1);
			//success_rate_6C6F = (float)msg6C6F/(float)((msgID6C6Ffinal-msgID6C6Finit) + 1);
			//success_rate_5161 = (float)msg5161/(float)((msgID6C6Ffinal-msgID6C6Finit) + 1);
			
			
			//printf("\nFCD0: Success rate: %f ", success_rate_FCD0);
			//printf("\n8625: Success rate: %f ", success_rate_8625);
			//printf("\n6C6F: Success rate: %f ", success_rate_6C6F);
			//printf("\n5161: Success rate: %f ", success_rate_5161);
		}
		
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
	}
	return 0;
}
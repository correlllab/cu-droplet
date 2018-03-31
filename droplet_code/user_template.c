#include "user_template.h"

uint32_t lastMessageSent;
//#define MSG_PERIOD 400
#define MAX_RECV_COUNT 100
#define RCVR_ID 0xAF6A
void sendMsg(void);

typedef struct droplet_messages_struct{
	uint16_t initMsgID;
	uint16_t finalMsgID;
	uint16_t msgCount;
	float ratioRcvd;
}DropletMessages;
DropletMessages msgLog[121];

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	printf("IR_MAX_MSG_ATTEMPT_DUR: %8u\r\n", IR_MAX_MSG_ATTEMPT_DUR);
	printf("IR_EXP_MSG_ATTEMPT_DUR: %8u\r\n", IR_EXP_MSG_ATTEMPT_DUR);
	printf("   HISTOGRAM_BIN_WIDTH: %8u\r\n", HISTOGRAM_BIN_WIDTH);
	printf("\r\n");
	lastMessageSent = getTime();
	rxEnabled = 0;
	msgCount = 0;
	for(int i=0; i<121; i++)
		msgLog[i].msgCount = 0;
		
	for(uint16_t i=0;i<NUM_HISTOGRAM_BINS;i++){
		histogram[i] = 0;
	}
	senderThisTime= (getDropletID()==RCVR_ID) ? 1 : (randReal()<=(0./16.));
	if(senderThisTime){
		setRedLED(50);
	}else{
		setBlueLED(50);
	}
	MSG_PERIOD = 600;

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
	msg.text[2]='.';
	msgCount++;
	msg.msgId = msgCount;
	msg.time = getTime();
	irSend(ALL_DIRS, (char*)(&msg), sizeof(Msg));
}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){	
	//float new_bearing, new_heading;
	//uint16_t new_steps;
	if(getTime()-lastMessageSent > MSG_PERIOD){
		if(senderThisTime){
			sendRtsByte();
			//sendMsg();
		}
		lastMessageSent = getTime();
	}
	
	
	delayMS(10);
}

void handleMsgTime(uint32_t time){
	uint16_t idx = time/HISTOGRAM_BIN_WIDTH;
	if(idx >= HISTOGRAM_SLOTS){
		printf("Unexpectedly large candidate index (%u)\r\n", idx);
		idx = HISTOGRAM_SLOTS+1;
	}
	histogram[idx]++;	
}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handleMsg(irMsg* msgStruct){
	Msg* msg = (Msg*)(msgStruct->msg);
	uint16_t msgID = (msg->msgId);
	if(getDropletID()==RCVR_ID && rxEnabled){
		uint8_t ord = getDropletOrd(msgStruct->senderID);
		if(!msgLog[ord].msgCount){
			msgLog[ord].initMsgID = msgID;	
		}
		msgLog[ord].msgCount++;
		msgLog[ord].finalMsgID = msgID;
		handleMsgTime(msg->time);
		printf("\t{%04X, %3u, %3u, %10lu},\r\n", msgStruct->senderID, msgLog[ord].msgCount, msgLog[ord].finalMsgID-msgLog[ord].initMsgID, getTime());
		if(msgLog[ord].msgCount == MAX_RECV_COUNT){ 
			processResults();
			printResults();
			rxEnabled = 0;
		}
	}
}

void printHistogram(){
	printf("{{0");
	for(uint16_t i=0;i<NUM_HISTOGRAM_BINS;i++){
		printf(", %u", (i+1)*HISTOGRAM_BIN_WIDTH);
	}
	printf("}, {%u", histogram[0]);
	for(uint16_t i=1;i<NUM_HISTOGRAM_BINS;i++){
		printf(", %u", histogram[i]);
	}
	printf("}}");
}

void printResults(){
	printf("<|\r\n\"irMT\"->%u, \"NumTries\"->%u, \"period\"->%lu", 
							(uint16_t)IR_MSG_TIMEOUT, (uint16_t)IR_MAX_MSG_TRIES, (uint32_t)MSG_PERIOD);
	printf(", \"numSenders\"->%hu, \"msgSize\"->%hu,\r\n\"results\"-><|", numSenders, (uint8_t)sizeof(Msg));
	uint8_t first=1;
	for(uint8_t i=0;i<121;i++){
		if(msgLog[i].msgCount>0){
			if(first){
				printf("\"%04X\"->%6.4f", getIdFromOrd(i), msgLog[i].ratioRcvd);
				first = 0;
			}else{
				printf(", \"%04X\"->%6.4f", getIdFromOrd(i), msgLog[i].ratioRcvd);
			}
		}
	}
	printf("|>,\"hist\"->\r\n");
	printHistogram();
	printf("\r\n|>\r\n");								
}

void processResults(){
	numSenders=0;
	for(uint8_t i=0;i<121;i++){
		if(msgLog[i].msgCount>0){
			numSenders++;
			uint16_t numSent = msgLog[i].finalMsgID - msgLog[i].initMsgID;
			uint16_t numMissed = numSent - msgLog[i].msgCount;
			float numRcvd = msgLog[i].msgCount;
			msgLog[i].ratioRcvd = numRcvd/numSent;		
			histogram[HISTOGRAM_SLOTS] += numMissed;
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
	}else if(strcmp_P(command_word,PSTR("start_rx"))==0){
		rxEnabled = 1;	
		return 1;
	}
	return 0;
}
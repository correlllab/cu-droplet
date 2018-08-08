#include "user_template.h"
#include <inttypes.h>
#include <stdio.h>


#define TARGET_HAMMING_WEIGHT 81
static inline void setIdMaskBit(BotIdMask* mask, id_t id);

uint32_t lastMessageSent;
#define MAX_RECV_COUNT 100


void sendMsg(void);

uint16_t recvCount;

uint8_t bincounts[10];
uint8_t bins;

typedef struct droplet_messages_struct{
	uint16_t initMsgID;
	uint16_t finalMsgID;
	uint16_t msgCount;
	float success_rate;
}DropletMessages;
DropletMessages msgLog[121];



void startListening(void){
	dataCollecting = 1;
	setRGB(20,100,100);
}

void startTransmitting(void){
	startSending = 1;
	setRGB(20,0,0);
}


/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	
	//GLOBAL VARIABLE INITIALIZATIONS
	recvCount = 0;
	MSG_PERIOD = 800;
	MS_DROPLET_COMM_TIME = 8;
	IR_MAX_TIME_LIVED = 500;
	
	#ifdef SYNCHRONIZED
		schedulePeriodicTask(2000, lightsOn, NULL);
	#else
		setRGB(6,6,6);

		lastMessageSent = getTime();
		msgCount=0;
		dataCollecting=0;
		startSending=0;
		
		for(int i=0; i<121; i++)
		msgLog[i].msgCount = 0;
	
	
	
		//backoffCount = 1;
		bins = 10;
		for(int i=0; i<bins; i++)
			bincounts[i] = 0;

		for(uint8_t i=0; i<4; i++)
			allIDsMask[i] = 0;
	
		throughputStarted = 0;
		lastThroughputMsgSent = getTime();
		scheduleTask(30000,startThroughputMessaging,NULL);	
	#endif

}

void startThroughputMessaging(){
		if(!throughputStarted){
			throughputStarted = 1;
			throughputMsgStart = getTime();
		}
		setIdMaskBit(&allIDsMask, getDropletID());		
		sendThroughputMsg();
}

void sendThroughputMsg(){
	BitMaskMsg msg;
	memcpy(msg.mask, allIDsMask, sizeof(BotIdMask));
	msg.flag = BIT_MASK_MSG_FLAG;
	irSend(ALL_DIRS, (char*)(&msg), sizeof(BitMaskMsg));
}


void sendDurationMsg(){
	DurationMsg msg;
	msg.flag = DURATION_MSG_FLAG;
	msg.dur = maxThroughputDuration;
	irSend(ALL_DIRS, (char*)&msg, sizeof(DurationMsg));
	printf("New Max Duration: %lu.\r\n", maxThroughputDuration);
}


void handleDurationmsg(DurationMsg* msg){
	if(msg->dur > maxThroughputDuration){
		maxThroughputDuration = msg->dur;
		sendDurationMsg();
	}
}

void handleThroughputMsg(BitMaskMsg* msg){
	//Code for determining the starting state of the uint128_t mask
	bitwiseIdMaskXOR(&(msg->mask), &allIDsMask);
	uint8_t informationDifference = !idMaskLogicalNegation(&(msg->mask));
	
	uint8_t numberOfOnesBefore = idMaskHammingWeight(&allIDsMask);
	bitwiseIdMaskOR(&allIDsMask, &(msg->mask));
	uint8_t numberOfOnesAfter = idMaskHammingWeight(&allIDsMask);
	
	if(numberOfOnesAfter > numberOfOnesBefore){
		if(numberOfOnesAfter==TARGET_HAMMING_WEIGHT){
			maxThroughputDuration = getTime()-throughputMsgStart;
			scheduleTask(5000,sendDurationMsg,NULL);
			setRGB(50,50,50);
		}else{
			setHSV((((uint16_t)numberOfOnesAfter)*360)/TARGET_HAMMING_WEIGHT, 255, 255);
		}
	}
	if(informationDifference){ //new message gives us information.
		if(!throughputStarted){
			throughputStarted = 1;
			throughputMsgStart = getTime();
		}
		sendThroughputMsg();
		printf("Sending (%hu)\r\n", numberOfOnesAfter);
		printIdMask(&allIDsMask);
	}	

	
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
	Msg* msg;
	msg = (Msg*)myMalloc(DESIRED_MSG_LENGTH);

	
	memcpy(msg->text, "Hi.12345678912", DESIRED_MSG_LENGTH-sizeof(Msg));
	
	msgCount = (msgCount + 1)%0x0FFF;
	msg->msgId = msgCount;

	msg->attempts = 0;
	irSend(ALL_DIRS, (char*)msg, DESIRED_MSG_LENGTH);
	myFree(msg);
	
}


void loop(){
	
	#ifdef SYNCHRONIZED
		delayMS(10);
		if(getTime()-lastMessageSent > MSG_PERIOD){
			
			//startSending = 0;		//RIYA Uncomment for Throughput test
			sendMsg();
			printf("%lu\r\n",getTime());
			lastMessageSent = getTime();
		}
	#else
		if(startSending){
			if(getTime()-lastMessageSent > MSG_PERIOD){
			
				startSending = 0;		//RIYA Uncomment for Throughput test
				sendMsg();
		
				lastMessageSent = getTime();
			}
		}else if(!throughputStarted){
			if((getTime() - lastThroughputMsgSent) < 5000){
				sendThroughputMsg();
				lastThroughputMsgSent = getTime();	
				delayMS(MSG_PERIOD);
			}
		}
	
		delayMS(10);
	#endif
}

void lightsOn(){
	setRGB(200,200,200);
	scheduleTask(1000, lightsOff, NULL);
}

void lightsOff(){
	setRGB(0,0,0);
}

/*
 * This function is called once for every range and bearing measurement this droplet has
 * received since the last time loop returned.
 */
void handleMeas(Rnb* meas){
	
}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */



void handleMsg(irMsg* msgStruct){
	if(((BitMaskMsg*)(msgStruct->msg))->flag==BIT_MASK_MSG_FLAG && msgStruct->length==sizeof(BitMaskMsg)){
		handleThroughputMsg((BitMaskMsg*)(msgStruct->msg));	
	}else if(((DurationMsg*)(msgStruct->msg))->flag==DURATION_MSG_FLAG && msgStruct->length==sizeof(DurationMsg)){
		handleDurationmsg((DurationMsg*)(msgStruct->msg));
	}else{
		Msg* msg = (Msg*)(msgStruct->msg);
	
	
		volatile uint8_t msgSender = 0;
		//msgSender = (uint8_t)(log((msg->msgId)>>12)/log(2));
	
		msgSender = getDropletOrd(msgStruct->senderID);
		uint16_t msgID = (msg->msgId)&0x0FFF;
	
		if(!dataCollecting){
			return;
		}
		
		printf("Got %04X %6u at %lu. Attempt no.:%hu.MsgText= %s\r\n", getIdFromOrd(msgSender), msgID, getTime(),msg->attempts,msg->text);
	

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
		

		
			printf("<|");
			for(uint8_t i=0; i<121; i++){
			
				if(msgLog[i].msgCount>0){
					numSenders++;
					msgLog[i].success_rate = (float)msgLog[i].msgCount/(float)((msgLog[i].finalMsgID-msgLog[i].initMsgID) + 1);
					meanSuccessRate+=msgLog[i].success_rate;
					//printf("{%04X, %f}, ", getIdFromOrd(i), msgLog[i].success_rate);
					printf("\"%04X\"-> %f, ", getIdFromOrd(i), msgLog[i].success_rate);
				}
			
			}
		
			printf("|>");
		
			printf("\n\rbincounts3 = {");
			for(int i=0;i<bins-1;i++)
				printf("%hu,",bincounts[i]);
			printf("%hu};",bincounts[bins-1]);
			printf("\n\rbins3 = Range[0,%hu];",bins-1);

			dataCollecting = 0;
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
	}else if(strcmp_P(command_word,PSTR("start_send"))==0){
		setRGB(20,0,0);
		setRGB(20,20,20);
		startSending = 1;	
		return 1;
	}else if(strcmp_P(command_word,PSTR("start_listen"))==0){
		setRGB(0,0,20);
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
	else if(strcmp_P(command_word,PSTR("set_ttl"))==0){
		const char delim[2] = " ";
		
		char* token = strtok(command_args,delim);
		uint32_t IR_MAX_TIME_LIVED = (uint32_t)atoi(token);
		setRGB(20,20,60);
		//scheduleTask(3000,startThroughputMessaging,NULL);	//Comment for Throughput test

	}
	else if(strcmp_P(command_word,PSTR("start_density"))==0){
		scheduleTask(3000,startThroughputMessaging,NULL);	//Comment for Throughput test
		setRGB(60,20,20);
		return 1;
	}
	
	return 0;
}

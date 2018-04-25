#include "user_template.h"


/*uint8_t data[512];
uint8_t read_data[512];
uint32_t address =0x0004;
uint8_t byte;
 *
 * any code in this function will be run once, when the robot starts.
 */

void init(){
	/*tmpTime = getTime();
	for(uint8_t i=0;i<7;i++){
		printf("%hu ", testData[i]);
	}
	printf("\r\n");*/
	//delayMS(5000);
	//volatile int dummyArray[512];
	//memset(dummyArray, 0x55, 512);

	/*int p=0;
	
	do 
	{
		if(p%3 ==0)
		{
		setRGB(0,0,255);	
		}
		
		else setRGB(255,0,0);
		p++;
	} while (p<100);*/
	
	for (int i= 0;i<16;i++)
	{
		setHSV(i*20,200,100);
		delayMS(2000);
	}
	//setRGB(255,0,0);
	//delayMS(10000);
	//dummyArray[i] = i;
	//setRGB(0,255,100);
	
}


/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	

	/*for(uint8_t i=0;i<4;i++){
		tmpArray[i] = randQuad();
		printf("%lu\r\n", tmpArray[i]);
	}
	
	printf("%lu", tmpTime);
	tmpTime = getTime();*/
	//if(getTime()%2000<1000){
	//	setHSV()
	//}
}

/*
 * after each pass through loop(), the robot checks for all mes sages it has 
 * received, and calls this function once for each message.
 */
void handleMsg(irMsg* msgStruct){

}

///*
 //*	the function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. it should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t userHandleCommand(char* command_word, char* command_args){
	//return 0;
//}
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
	
	/*for (int i= 0;i<16;i++)
	{
		setHSV(i*20,200,100);
		delayMS(2000);
	}*/
	
	setRGB(0,255,0);
	
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
 * This function is called once for every range and bearing measurement this droplet has
 * received since the last time loop returned.
 */
void handleMeas(Rnb* meas){

}

/*
 * This function is called once for every message this droplet has received since the last
 * time loop returned, after handleMeas is called for any rnb measurements received.
>>>>>>> f26063acd552cf1469f24b84132289025a352b6e
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
//void userMicInterrupt(){}

/*
 * If defined, this function will be called with any serial commandWords that do not match
 * other commands serial_handler.c checks for. See the serial_handler documentation for
 * details on commandWord and commandArgs.
 */
//uint8_t userHandleCommand(char* commandWord, char* commandArgs){}


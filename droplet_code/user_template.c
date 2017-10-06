#include "user_template.h"

typedef struct color_switch_info_struct{
uint8_t r;
uint8_t g;
uint8_t b;
uint16_t time;
}ColorSwitchInfo;

#define NUM_COLOR_SWITCHES 12
ColorSwitchInfo colorSwitches[NUM_COLOR_SWITCHES] = 
{{100,0,0,500}, {100,100,100,1500}, {0,100,0,2500}, {100,100,100,3500}, {0,0,100,4500}, {100,100,100,5500}, {0,100,100,6500}, {100,100,100,7500}, {100,0,100,8500}, {100,100,100,9500}, {100,100,0,10500}, {0,0,0,11500}};

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
	//enable_sync_blink(500);
	//set_sync_blink_color(0, 100, 0);
	//set_sync_blink_duration(4000);
	//set_rgb(0,0,150);
	schedulePeriodicTask(2000, lightsOn, NULL);
}



/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	//uint8_t delayed = 0;
	//int16_t modTime = get_time()%12000;
	//for(uint8_t i=0;i<NUM_COLOR_SWITCHES;i++){
		//ColorSwitchInfo* cSwitch = &(colorSwitches[i]);
		//int16_t diff = cSwitch->time - modTime;
		//if(abs(diff) < 1){
			//set_rgb(cSwitch->r, cSwitch->g, cSwitch->b);
			//break;
		//}else if((uint16_t)diff < 20){
			//delay_ms(diff-1);
			//delayed = 1;
			//break;
		//}
	//}
	//if(!delayed){
		//delay_ms(10);
	//}
	delayMS(10);
}

void lightsOn(){
	setRGB(200,200,200);
	scheduleTask(1000, lightsOff, NULL);
}

void lightsOff(){
	setRGB(0,0,0);
}

/*
 * after each pass through loop(), the robot checks for all messages it has 
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
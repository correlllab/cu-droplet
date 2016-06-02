// This program contains the code to be dumped into both Broadcasting Droplet and receiving Droplet


#include "user_template.h"

//#define BroadcastDroplet      // On initialization Broadcasting Droplet is green in color
//#define ReceiverDroplet	    // On initialization Receiving Droplet is blue in color

#ifdef ReceiverDroplet
	#define TURN			0
	#define MOVEFORWARD		1
	#define ROTATECONST		55
	#define DIR_CLKWISE		6
	#define DIR_ANTICLKWISE 7
	#define DIR_FOR			0
	#define DIR_BACK		3
	#define NUM_OF_SAMPLES  20   // Number of reading to take for averaging the Bearing angle and range
#endif
/*
 * Any code in this function will be run once, when the robot starts.
 */


void init()
{
	// Setting the color of Broadcasting Droplet into green at initialization
	#ifdef BroadcastDroplet
		set_green_led(255);
	#endif
	
	// Setting the color of Receiver Droplet into Blue at initialization
	#ifdef ReceiverDroplet
		set_blue_led(255);
	#endif
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */

#ifdef ReceiverDroplet
	uint16_t BroadcastId;
	float rangeOld;
	float range;
	// Helps in taking the average of the bearing angle.
	
	float bearingAngle;
	float bearingAngleOld = 0; 
	
	float headingAngle;
	float confLevel;
	
	uint8_t updateDirection;
	int8_t statusOfRec;
	int bearingAngleInit = 0,j=0;
#endif

void loop()
{
	#ifdef BroadcastDroplet
		//delay_ms(500);
		//printf("ID %04X \n\r",get_droplet_id());
		broadcast_rnb_data();    // Broadcasting data from Broadcast Droplet
	#endif
	
	#ifdef ReceiverDroplet
	
		// Capturing Broadcasting Data
		if(rnb_updated)
		{
			BroadcastId = last_good_rnb.id_number;
			range = last_good_rnb.range;
			bearingAngle = last_good_rnb.bearing;
			headingAngle = last_good_rnb.heading;
			confLevel = last_good_rnb.conf;
			rnb_updated = 0; //Note! This line must be included for things to work properly.
			
			// helps in taking the average values of bearing angle and range
			if (bearingAngleInit == 0)
			{
				bearingAngleOld =  bearingAngle;
				rangeOld = range;
			}
			
			bearingAngleInit = 1;
			
			// capturing the values of bearing angle and range only if confLevel is greater than 1
			if (confLevel>1)
			{
				bearingAngle = (bearingAngle + bearingAngleOld)/2;   // Taking the average of bearingAngle
				printf("bearingAngle %f \n\r",bearingAngle);
				printf("Range %f \n\r",range);
				bearingAngleOld =  bearingAngle;
				rangeOld = range;
				j++;
			}
			
			if (j == NUM_OF_SAMPLES) 
			{
				if(bearingAngle > 0.5)
				{
					walk(DIR_ANTICLKWISE, ROTATECONST * bearingAngle); // walking in anti clockwise direction
					while(is_moving() == DIR_ANTICLKWISE);             // wait till the rotation is completed 
					set_rgb(255,255,255);                              // set the rgb to white
				
				
				/*else if (bearingAngle < -0.5)
				{
					walk(DIR_CLKWISE, ROTATECONST * bearingAngle * -1);	   // walking in clockwise direction
					while(is_moving() == DIR_CLKWISE);				   // wait till the rotation is completed 	
					set_rgb(0,255,255);								   // 
				}*/
				
					// Once the Droplet moves the required bearing angle, Giving the required commands to move in forward direction
					// the forward motion is determined by the calcuated range
					
					walk(DIR_FOR, (uint16_t)range*5);				  // Move in forward direction
					while(is_moving() == DIR_FOR);                    // wait till the forward motion is completed 
					set_rgb(0,255,255);            
				
					stop_move();                                      // Stop movement
					statusOfRec = is_moving();						  
					printf("Status %d", statusOfRec);
					while(statusOfRec == -1);
				}
			}	
		}
	#endif
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{
	/*MsgStruct* msgStruct = (MsgStruct*)(msg_struct->msg);
	if(msgStruct->flag==MSG_STRUCT_FLAG){
		
	}
	if(msg_struct->msg[0]=='h'){
		set_rgb(200,0,0);
	}
	msg_struct->*/
}

/*
 *	The function below is optional - commenting it in can be useful for debugging if you want to query
 *	user variables over a serial connection.
 */
/*uint8_t user_handle_command(char* command_word, char* command_args)
{
	if(strcmp(command_word, "start")){
		
		return 1;
	}
	return 0;
}
*/
///*
 //*	The function below is optional - if it is commented in, and the leg interrupts have been turned on
 //*	with enable_leg_status_interrupt(), this function will get called when that interrupt triggers.
 //*/	
//void	user_leg_status_interrupt()
//{
	//
//}
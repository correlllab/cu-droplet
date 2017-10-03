#include "user_template.h"

uint32_t lastMessageSent;
#define MSG_PERIOD 200

/*
 * any code in this function will be run once, when the robot starts.
 */
void init(){
set_red_led(50);
lastMessageSent = get_time();
msgCount=1000;
}


void sendMsg(){
	Msg msg;
	msg.text[0]= 'H';
	msg.text[1]='i';
	msg.text[2]='.';
	
	//msg.text="Hi.";
	msgCount = (msgCount + 1)%2000;
	msg.msgId = msgCount;//++;
	//char* msg_str;
	//sprintf(msg_str, "Message=%s, Message_ID=%d", msg.text, msg.msgId);
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(Msg));
}

/*
 * the code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	
	float new_bearing, new_heading;
	uint16_t new_steps;
	if(get_time()-lastMessageSent > MSG_PERIOD){
		sendMsg();
		sendMsg();
		sendMsg();
		//sendMsg();
		//sendMsg();
		lastMessageSent = get_time();
	}
	
	//ir_send(ALL_DIRS, "Hi.", 3);
	
	//if(rnb_updated){
		//
		//last_good_rnb.id;
		//last_good_rnb.range;
		//last_good_rnb.bearing;
		//last_good_rnb.heading;
//
		//
		//
		////printf("\n\r%04x",last_good_rnb.id);
		////printf("\n\r%04x",last_good_rnb.id);
		////printf("\n\rRange-> %f,Bearing-> %f,Heading-> %f,ID-> %04x",
			////last_good_rnb.range), last_good_rnb.bearing*180/M_PI,
			////last_good_rnb.heading*180/M_PI, last_good_rnb.id);
		//printf("\n\rRange-> %u",last_good_rnb.range);
		//printf("\n\rBearing-> %f",last_good_rnb.bearing*180/M_PI);
		//printf("\n\rHeading-> %f",last_good_rnb.heading*180/M_PI);
		//printf("\n\rID-> %04x",last_good_rnb.id);
		////rad_to_deg()
		//new_bearing = last_good_rnb.bearing*180/M_PI;
		//new_heading = last_good_rnb.heading*180/M_PI; 
		//
		//printf("\n\rnew_Bearing-> %f",new_bearing);
		//printf("\n\rnew_Heading-> %f",new_heading);
		//
		////useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		//
		//
		////kinda working
	//
		//
		//stop_move();
		//
		//if(new_bearing > -10255 && new_bearing < -228)
		//{
			//
			//if(is_moving() < 0)
			//{
				//move_steps(6,5);
				//printf("\n\rDirection 6,5");
			//}
			//
		//}
		//else if(new_bearing < 10255 && new_bearing > 228)
		//{
			//if(is_moving() < 0)
			//{
				//move_steps(7,5);
				//printf("\n\rDirection 7,5");
				//
			//}
			//
		//}
		//else if(new_bearing < 228 && new_bearing > -228)
		//{
			//if (last_good_rnb.range < 75)
				//stop_move();
			//else
			//{
				//if(is_moving() < 0)
				//{
					//move_steps(0,10);
					//printf("\n\rDirection 0,10");
				//}
				//
			//}
				//
		//}
		//
		////stop_move();
		//
		//rnb_updated = 0; //Note! This line must be included for things to work properly.
	//}else{
		//delay_ms(100);
		//broadcast_rnb_data();
	//}
	
	delay_ms(10);
}

/*
 * after each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){
	Msg* msg = (Msg*)(msg_struct->msg);
	printf("Got %6u\r\n", msg->msgId);
	
	
	//for(uint8_t l=0;l<msg_struct->length;l++){
		//printf("%c", msg_struct->msg[l]);
	//}
	//printf("%s",msg_struct->msg);
	////int l = 0;
	//while(*(msg_struct->msg)!=NULL){
		//printf("%c", *(msg_struct->msg));
		//msg_struct->msg += 1;
	//}
}

///*
 //*	the function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. it should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){
	//return 0;
//}
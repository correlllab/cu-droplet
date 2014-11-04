#include "user_template.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	if(rand_byte()<2) //lets send a message!
	{
		if(rand_byte()%2==0)	ir_send(ALL_DIRS, "<3Y", 3);
		else					ir_send(ALL_DIRS, "<3N", 3);
	}
	delay_ms(10);
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{
	if(strcmp(msg_struct->msg, "<3Y")==0)		printf("\tGot '<3Y' from %hx.\r\n", msg_struct->sender_ID);
	else if(strcmp(msg_struct->msg, "<3N")==0)	printf("\tGot '<3N' from %hx.\r\n", msg_struct->sender_ID);
	else{ printf("!!\tGot '"); printf(msg_struct->msg); printf("' from %hx.\r\n", msg_struct->sender_ID); }
}
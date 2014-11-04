#include "main.h"

int main(void)
{
	init_all_systems ();
	
	while(1)
	{
		if(rand_byte()<2) ir_send(ALL_DIRS, "<3Y", 3);
		check_messages ();	
		delay_ms(10);
	}
}

void check_messages ()
{	
	while ( last_ir_msg != NULL )
	{
		uint16_t senderID;
		char tempStr [ IR_BUFFER_SIZE ];		
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{	
			memset	( tempStr, 0, IR_BUFFER_SIZE );
			memcpy	( tempStr, last_ir_msg->msg, last_ir_msg->msg_length );
			uint16_t senderID = last_ir_msg->sender_ID;			
			msg_node *temp	= last_ir_msg;
			last_ir_msg		= last_ir_msg->prev;
			free ( temp->msg );
			free ( temp );
		}
			if(strcmp(tempStr, "<3Y")==0) printf("\tGot '<3Y'.\r\n");
			else{ printf("!!\tGot '"); printf(tempStr); printf("'\r\n"); }
	}
}
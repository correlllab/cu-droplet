#include "tdma_template.h"

uint8_t m = 3;
uint8_t n = 3;
uint8_t dist = 50;
uint8_t margin = 20;
id_t undf_id = 0xFFFF;
uint8_t filled;

typedef struct cell_struct
{
	id_t ID;			// ID of the Droplet representing this cell
}cell;

cell me;
cell N;
cell NE;
cell E;
cell SE;
cell S;
cell SW;
cell W;
cell NW;
cell neighbors[8];

typedef struct pos_msg_struct{
	int16_t posx;
	int16_t posy;
} PosMsg;

void sendPosMsg(void){
	PosMsg msg;

	msg.posx = myPos.x;
	msg.posy = myPos.y;
	
	ir_send(ALL_DIRS, (char*)(&msg), sizeof(PosMsg));
}

void reset_neighbors(void){
	me.ID = get_droplet_id();
	N.ID = undf_id;
	NE.ID = undf_id;
	E.ID = undf_id;
	SE.ID = undf_id;
	S.ID = undf_id;
	SW.ID = undf_id;
	W.ID = undf_id;
	NW.ID = undf_id;
}

void update_neighbors(void){
	neighbors[0] = N;
	neighbors[1] = NE;
	neighbors[2] = E;
	neighbors[3] = SE;
	neighbors[4] = S;
	neighbors[5] = SW;
	neighbors[6] = W;
	neighbors[7] = NW;
}

void count_neighbors_expected(void){
	// Knowing how many neighbors to expect (am I a corner, border or center ?)
	
	reset_neighbors();
	
	if(POS_DEFINED(&myPos)){
		if (myPos.y >= 0-margin && myPos.y <= 0+margin)
		{
			N.ID = 0;
			NW.ID = 0;
			NE.ID = 0;
		}
		
		if (myPos.y >= -(n-1)*dist-margin && myPos.y <= -(n-1)*dist+margin)
		{
			S.ID = 0;
			SW.ID = 0;
			SE.ID = 0;
		}
		
		if (myPos.x >= 0-margin && myPos.x <= 0+margin)
		{
			W.ID = 0;
			NW.ID = 0;
			SW.ID = 0;
		}
		
		if (myPos.x >= (m-1)*dist-margin && myPos.x <= (m-1)*dist+margin)
		{
			E.ID = 0;
			NE.ID = 0;
			SE.ID = 0;
		}
	}
	
	update_neighbors();
}

void show_neighbors_expected(void){
	// Show if I am a corner, border or center
	uint8_t blank_neighbors = 0;
	for (int i=0; i<8; i++)
	{
		if (neighbors[i].ID == 0)
		{
			blank_neighbors += 1;
		}
	}
	
	if (blank_neighbors == 5)
	{
		set_rgb(255,0,0);
	}
	
	if (blank_neighbors == 3)
	{
		set_rgb(255,0,255);
	}
	
	if (blank_neighbors == 0)
	{
		set_rgb(0,0,255);
	}

}

void show_neighbors_filled(void){
	uint8_t neighbors_filled = 0;
	for (int i=0; i<8; i++)
	{
		if (neighbors[i].ID != undf_id)
		{
			neighbors_filled += 1;
		}
	}
	
	if (neighbors_filled == 8)
	{
		set_green_led(255);
		filled = 1;
	}
	
}

void print_neighbors(void){
	
	printf("N ID is %04X \r\n", N.ID);
	printf("NE ID is %04X \r\n", NE.ID);
	printf("E ID is %04X \r\n", E.ID);
	printf("SE ID is %04X \r\n", SE.ID);
	printf("S ID is %04X \r\n", S.ID);
	printf("SW ID is %04X \r\n", SW.ID);
	printf("W ID is %04X \r\n", W.ID);
	printf("NW ID is %04X \r\n", NW.ID);
}


/*
 * Any code in this function will be run once, when the robot starts.
 */
void init(){
	#ifndef SYNCHRONIZED
	printf("Error! It doesn't make sense to do TDMA without synchronizing the Droplets!\r\n");
	#endif
	loopID = 0xFFFF;
	frameCount = 0;
	mySlot = (get_droplet_id()%(SLOTS_PER_FRAME-1));
	frameStart = get_time();
	
	reset_neighbors();
	
	update_neighbors();
	
	filled = 0;
	set_rgb(255,255,255);
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop(){
	uint32_t frameTime = get_time()-frameStart;
	if(frameTime>FRAME_LENGTH_MS){
		frameTime = frameTime - FRAME_LENGTH_MS;
		frameStart += FRAME_LENGTH_MS;
		frameCount++;
	}
	if(loopID!=(frameTime/SLOT_LENGTH_MS)){
		loopID = frameTime/SLOT_LENGTH_MS;
		if(loopID==mySlot){
			broadcast_rnb_data();
			
			/*if (POS_DEFINED(&myPos))
			{
				sendPosMsg();
			}*/
		}else if(loopID==(SLOTS_PER_FRAME-1)){
			//compute new state based on messages.
			
			
			
				count_neighbors_expected();
				show_neighbors_expected();
				show_neighbors_filled();
			
			if (filled == 1)
			{
				print_neighbors();
			}
			
		}
		
	}
	if(rnb_updated){
		//Handle rnb data in last_good_rnb struct.
		useRNBmeas(last_good_rnb.id, last_good_rnb.range, last_good_rnb.bearing, last_good_rnb.heading);
		rnb_updated=0;
	}
	delay_ms(LOOP_DELAY_MS);
}
/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct){
	if(((BotMeasMsg*)(msg_struct->msg))->flag==BOT_MEAS_MSG_FLAG && msg_struct->length==sizeof(BotMeasMsg)){
		handleBotMeasMsg((BotMeasMsg*)(msg_struct->msg), msg_struct->sender_ID);
	}
	
	/*if(msg_struct->length == sizeof(PosMsg))
	{
		PosMsg* posMsg = (PosMsg*)(msg_struct->msg);
		
		if (posMsg->posx >= myPos.x-dist-margin && posMsg->posx <= myPos.x-dist+margin)
		{
			if (posMsg->posy >= myPos.y-dist-margin && posMsg->posy <= myPos.y-dist+margin )
			{
				SW.ID = msg_struct->sender_ID;
			}
			
			if (posMsg->posy >= myPos.y-margin && posMsg->posy <= myPos.y+margin)
			{
				W.ID = msg_struct->sender_ID;
			}
			
			if (posMsg->posy >= myPos.y+dist-margin && posMsg->posy <= myPos.y+dist+margin )
			{
				NW.ID = msg_struct->sender_ID;
			}
		}
		
		if (posMsg->posx >= myPos.x+dist-margin && posMsg->posx <= myPos.x+dist+margin)
		{
			if (posMsg->posy >= myPos.y-dist-margin && posMsg->posy <= myPos.y-dist+margin)
			{
				SE.ID = msg_struct->sender_ID;
			}
			
			if (posMsg->posy >= myPos.y-margin && posMsg->posy <= myPos.y+margin )
			{
				E.ID = msg_struct->sender_ID;
			}
			
			if (posMsg->posy >= myPos.y+dist-margin && posMsg->posy <= myPos.y+dist+margin )
			{
				NE.ID = msg_struct->sender_ID;
			}
		}
		
		if (posMsg->posx >= myPos.x-margin && posMsg->posx <= myPos.x+margin )
		{
			if (posMsg->posy >= myPos.y-dist-margin && posMsg->posy <= myPos.y-dist+margin)
			{
				S.ID = msg_struct->sender_ID;
			}
			
			if (posMsg->posy >= myPos.y+dist-margin && posMsg->posy <= myPos.y+dist+margin )
			{
				N.ID = msg_struct->sender_ID;
			}
		}
		
		update_neighbors();
	}*/
}

///*
 //*	The function below is optional - commenting it in can be useful for debugging if you want to query
 //*	user variables over a serial connection. It should return '1' if command_word was a valid command,
 //*  '0' otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args){
	//return 0;
//}


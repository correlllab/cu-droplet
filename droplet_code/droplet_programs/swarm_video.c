#include "droplet_programs/swarm_video.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	state = UNPROGRAMMED;
	interp=0;
	loop_end=get_time();
	magic_number=loop_end%FRAME_DELAY;
}

//full power red (hits the rail at 127) tends to increase green by ~12 and blue by ~3
//full power green (106) tends to increase red by ~11 and blue by ~12
//full power blue (37) tends to increase red by ~3 and green by ~11
//these effects accumulate, not quite additively: full power white light reads as 
//(127, 124, 48) which is a little less than the sum.

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{
	uint8_t r,g,b;
	if(state==RECORD)
	{
		get_cal_rgb(&r, &g, &b);
		if(r>127)	vid_frames[frame_count][R]=255;
		else		vid_frames[frame_count][R]=0;
		if(g>127)	vid_frames[frame_count][G]=255;
		else		vid_frames[frame_count][G]=0;
		if(b>127)	vid_frames[frame_count][B]=255;
		else		vid_frames[frame_count][B]=0;
		
		
		if		(!vid_frames[frame_count][R]&&vid_frames[frame_count-1][R]&&!vid_frames[frame_count-2][R])	vid_frames[frame_count-1][R]=0; //010->000
		else if (vid_frames[frame_count][R]&&!vid_frames[frame_count-1][R]&&vid_frames[frame_count-2][R])	vid_frames[frame_count-1][R]=255; //101->111
		if		(!vid_frames[frame_count][G]&&vid_frames[frame_count-1][G]&&!vid_frames[frame_count-2][G])	vid_frames[frame_count-1][G]=0; //010->000
		else if (vid_frames[frame_count][G]&&!vid_frames[frame_count-1][G]&&vid_frames[frame_count-2][G])	vid_frames[frame_count-1][G]=255; //101->111
		if		(!vid_frames[frame_count][B]&&vid_frames[frame_count-1][B]&&!vid_frames[frame_count-2][B])	vid_frames[frame_count-1][B]=0; //010->000
		else if (vid_frames[frame_count][B]&&!vid_frames[frame_count-1][B]&&vid_frames[frame_count-2][B])	vid_frames[frame_count-1][B]=255; //101->111				
		
		frame_count++;
		if(frame_count>MAX_FRAME_COUNT)
		{
			total_frames=MAX_FRAME_COUNT;
			state=PLAY;
			frame_count = 0;
		}
	}
	else if(state==PLAY)
	{
		if(!interp)
		{
			set_rgb(vid_frames[frame_count][R], vid_frames[frame_count][G], vid_frames[frame_count][B]);
			frame_count++;
			frame_count=frame_count%total_frames;
			interp=1;
		}
		else if(interp==1)
		{
			uint8_t rMean = (uint8_t)((3*((uint16_t)vid_frames[frame_count][R])+((uint16_t)vid_frames[frame_count+1][R]))/4);
			uint8_t gMean = (uint8_t)((3*((uint16_t)vid_frames[frame_count][G])+((uint16_t)vid_frames[frame_count+1][G]))/4);
			uint8_t bMean = (uint8_t)((3*((uint16_t)vid_frames[frame_count][B])+((uint16_t)vid_frames[frame_count+1][B]))/4);
			set_rgb(rMean, gMean, bMean);
			interp=2;
		}
		else if(interp==2)
		{
			uint8_t rMean = (uint8_t)((((uint16_t)vid_frames[frame_count+1][R])+((uint16_t)vid_frames[frame_count+1][R]))/2);
			uint8_t gMean = (uint8_t)((((uint16_t)vid_frames[frame_count+1][G])+((uint16_t)vid_frames[frame_count+1][G]))/2);
			uint8_t bMean = (uint8_t)((((uint16_t)vid_frames[frame_count+1][B])+((uint16_t)vid_frames[frame_count+1][B]))/2);
			set_rgb(rMean, gMean, bMean);
			interp=3;
		}
		else
		{
			uint8_t rMean = (uint8_t)((((uint16_t)vid_frames[frame_count][R])+3*((uint16_t)vid_frames[frame_count+1][R]))/4);
			uint8_t gMean = (uint8_t)((((uint16_t)vid_frames[frame_count][G])+3*((uint16_t)vid_frames[frame_count+1][G]))/4);
			uint8_t bMean = (uint8_t)((((uint16_t)vid_frames[frame_count][B])+3*((uint16_t)vid_frames[frame_count+1][B]))/4);
			set_rgb(rMean, gMean, bMean);
			interp=0;
		}

	}

	uint32_t delay_dur;	
	if(state==PLAY)	delay_dur = FRAME_DELAY/4;
	else			delay_dur = FRAME_DELAY;

	while((get_time()-loop_end)<(FRAME_DELAY-5));
	uint32_t curr_time = get_time()%FRAME_DELAY;
	uint32_t wait;
	if(curr_time<(magic_number%delay_dur))	wait=(magic_number%delay_dur)-curr_time;
	else									wait=((magic_number%delay_dur)+FRAME_DELAY)-curr_time;
	busy_delay_ms(wait);
	loop_end = get_time();
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{
	if(strcmp(msg_struct->msg,"record")==0)
	{
		state=RECORD;
	}
	else if(strcmp(msg_struct->msg,"play")==0)
	{
		total_frames=frame_count;
		state=PLAY;
		frame_count=0;
	}
}
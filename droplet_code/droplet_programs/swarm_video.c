#include "droplet_programs/swarm_video.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	const int8_t num_samples = 5;
	int16_t r_avg=0, g_avg=0, b_avg=0;
	int8_t r, g, b;
	delay_ms(1000);
	for(uint8_t i=0; i<num_samples; i++)
	{
		get_rgb_sensors(&r, &g, &b);
		r_avg+=r;
		g_avg+=g;
		b_avg+=b;
		delay_ms(100);
	}
	r_baseline= r_avg/num_samples;
	g_baseline= g_avg/num_samples;
	b_baseline= b_avg/num_samples;
	printf("Baselines:\r\n%3hd  %3hd  %3hd\r\n", r_baseline, g_baseline, b_baseline);
	state = UNPROGRAMMED;
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
	//printf("%d\r\n",state);
	uint8_t r,g,b;
	get_rgb_wrapper(&r, &g, &b);
	if(state==UNPROGRAMMED)
	{
		if((r>MIN_R_THRESH)&&(g>MIN_G_THRESH)&&(b>MIN_B_THRESH)) //If we are seeing full white.
		{			
			calib_white[R]=r;
			calib_white[G]=g;
			calib_white[B]=b;
			for(uint8_t i=0;i<3;i++)
			{
				calib_matrix[R][i]=0;
				calib_matrix[G][i]=0;
				calib_matrix[B][i]=0;
			}
			state = CALIB_R;			
		}
	}
	else if(state==CALIB_R)
	{
		if((r>MIN_R_THRESH)&&(g<MIN_G_THRESH)&&(b<MIN_B_THRESH)) //If we are seeing just red.
		{
			time_vals[0]=get_time();
			calib_matrix[R][R]=r;
			calib_matrix[R][G]=g;
			calib_matrix[R][B]=b;
			state = CALIB_G;			
		}
	}
	else if(state==CALIB_G)
	{
		if((r<MIN_R_THRESH)&&(g>MIN_G_THRESH)&&(b<MIN_B_THRESH)) //If we are seeing just green.
		{
			time_vals[1]=get_time();
			calib_matrix[G][R]=r;
			calib_matrix[G][G]=g;
			calib_matrix[G][B]=b;
			state = CALIB_B;			
		}
	}
	else if(state==CALIB_B)
	{
		if((r<MIN_R_THRESH)&&(g<MIN_G_THRESH)&&(b>MIN_B_THRESH)) //If we are seeing just blue.
		{
			time_vals[2]=get_time();
			calib_matrix[B][R]=r;
			calib_matrix[B][G]=g;
			calib_matrix[B][B]=b;
			
			uint32_t gap_a = (time_vals[1]-time_vals[0]);
			uint32_t gap_b = (time_vals[2]-time_vals[1]);
			if((gap_a>MAX_FRAME_DELAY)||(gap_b>MAX_FRAME_DELAY))
			{
				//Too long between frames. Giving up.
				state = UNPROGRAMMED;
			}
			else
			{
				invert_calib_matrix();
				frame_delay = (time_vals[2]-time_vals[0])/2;
				frame_count = 0;
				state = RECORD;
			}
		}
	}	
	else if(state==RECORD)
	{
		get_calibrated_frame_vals(	&(vid_frames[R][frame_count]), 
									&(vid_frames[G][frame_count]), 
									&(vid_frames[B][frame_count]), 
									r, g, b);
		//printf("IN:\t%hhu\t%hhu\t%hhu\r\n",r,g,b);
		//printf("OUT:\t%hhu\t%hhu\t%hhu\r\n",vid_frames[R][frame_count], vid_frames[G][frame_count], vid_frames[B][frame_count]);
		frame_count++;
		if(frame_count>MAX_FRAME_COUNT)
		{
			state=PLAY;
			frame_count = 0;
		}
	}
	else if(state==PLAY)
	{
		set_rgb(vid_frames[R][frame_count], vid_frames[G][frame_count], vid_frames[B][frame_count]);
		frame_count++;
	}
	if(state==RECORD||state==PLAY)	delay_ms(frame_delay);
	else									delay_ms(10);
}

void get_rgb_wrapper(uint8_t* r_dest, uint8_t* g_dest, uint8_t* b_dest)
{
	int8_t r,g,b;
	get_rgb_sensors(&r, &g, &b);
	int16_t tmp_r, tmp_g, tmp_b;
	tmp_r = r-r_baseline;
	tmp_g = g-g_baseline;
	tmp_b = b-b_baseline;
	if(tmp_r<0) tmp_r = 0;
	if(tmp_g<0) tmp_g = 0;
	if(tmp_b<0) tmp_b = 0;
	*r_dest = (uint8_t)(tmp_r);
	*g_dest = (uint8_t)(tmp_g);
	*b_dest = (uint8_t)(tmp_b);
}

void get_calibrated_frame_vals(uint8_t* r_out, uint8_t* g_out, uint8_t* b_out, uint8_t r, uint8_t g, uint8_t b)
{
	float tmp_r_out, tmp_g_out, tmp_b_out;
	
	tmp_r_out = inv_calib_matrix[R][R]*r + inv_calib_matrix[R][G]*g + inv_calib_matrix[R][B]*b;
	tmp_g_out = inv_calib_matrix[G][R]*r + inv_calib_matrix[G][G]*g + inv_calib_matrix[G][B]*b;
	tmp_b_out = inv_calib_matrix[B][R]*r + inv_calib_matrix[B][G]*g + inv_calib_matrix[B][B]*b;
	
	if(tmp_r_out>255)		*r_out = 255;
	else if(tmp_r_out<0)	*r_out = 0;
	else					*r_out = (uint8_t)tmp_r_out;
	
	if(tmp_g_out>255)		*g_out = 255;
	else if(tmp_g_out<0)	*g_out = 0;
	else					*g_out = (uint8_t)tmp_g_out;
	
	if(tmp_b_out>255)		*b_out = 255;
	else if(tmp_b_out<0)	*b_out = 0;
	else					*b_out = (uint8_t)tmp_b_out;		
}

void invert_calib_matrix()
{
	float det = calib_matrix[R][R]*(calib_matrix[G][G]*calib_matrix[B][B]-calib_matrix[B][G]*calib_matrix[G][B])+
				calib_matrix[G][R]*(calib_matrix[B][G]*calib_matrix[R][B]-calib_matrix[R][G]*calib_matrix[B][B])+
				calib_matrix[B][R]*(calib_matrix[R][G]*calib_matrix[G][B]-calib_matrix[G][G]*calib_matrix[R][B]);
	det = det/255;
	inv_calib_matrix[R][R] = (calib_matrix[G][G]*calib_matrix[B][B]-calib_matrix[B][G]*calib_matrix[G][B])/det;
	inv_calib_matrix[R][G] = (calib_matrix[B][G]*calib_matrix[R][B]-calib_matrix[R][G]*calib_matrix[B][B])/det;
	inv_calib_matrix[R][B] = (calib_matrix[R][G]*calib_matrix[G][B]-calib_matrix[G][G]*calib_matrix[R][B])/det;
	inv_calib_matrix[G][R] = (calib_matrix[B][R]*calib_matrix[G][B]-calib_matrix[G][R]*calib_matrix[B][B])/det;
	inv_calib_matrix[G][G] = (calib_matrix[R][R]*calib_matrix[B][B]-calib_matrix[B][R]*calib_matrix[R][B])/det;
	inv_calib_matrix[G][B] = (calib_matrix[R][B]*calib_matrix[G][R]-calib_matrix[R][R]*calib_matrix[G][B])/det;
	inv_calib_matrix[B][R] = (calib_matrix[G][R]*calib_matrix[B][G]-calib_matrix[B][R]*calib_matrix[G][G])/det;
	inv_calib_matrix[B][G] = (calib_matrix[B][R]*calib_matrix[R][G]-calib_matrix[R][R]*calib_matrix[B][G])/det;
	inv_calib_matrix[B][B] = (calib_matrix[R][R]*calib_matrix[G][G]-calib_matrix[G][R]*calib_matrix[R][G])/det;
	//printf("Calib Matrix:\r\n");
	//for(uint8_t i=0;i<3;i++) printf("\t%ld\t%ld\t%ld\r\n", calib_matrix[i][R], calib_matrix[i][G], calib_matrix[i][B]);
	//printf("\r\nInv CalibMatrix:\r\n");
	//for(uint8_t i=0;i<3;i++) printf("\t%f\t%f\t%f\r\n", inv_calib_matrix[i][R], inv_calib_matrix[i][G], inv_calib_matrix[i][B]);
	//printf("\r\n");
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{

}
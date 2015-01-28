#include "droplet_programs/swarm_video.h"

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	
	const int8_t num_samples = 5;
	int16_t r_avg=0, g_avg=0, b_avg=0;
	off_count = 0;
	int16_t rCal, gCal, bCal;
	delay_ms(1000);
	for(uint8_t i=0; i<num_samples; i++)
	{
		get_rgb_sensors(&rCal, &gCal, &bCal);	
		r_avg+=rCal;
		g_avg+=gCal;
		b_avg+=bCal;
		delay_ms(100);
	}
	r_baseline= r_avg/num_samples;
	g_baseline= g_avg/num_samples;
	b_baseline= b_avg/num_samples;
	
	printf("Baselines:\r\n%3hd  %3hd  %3hd\r\n", r_baseline, g_baseline, b_baseline);
		
	uint8_t r, g, b;
	printf("CM:\r\n");
	for(uint8_t i=0; i<NUM_CALIB_MEAS ; i++)
	{
		get_rgb_wrapper(&r, &g, &b);
		cm[i][R] = r;
		cm[i][G] = g;
		cm[i][B] = b;
		printf("\t%hu\t%hu\t%hu\r\n",r, g, b);
		delay_ms(500);
	}
	printf("\r\n");
	calc_calib_matrix();
	state = RECORD;

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
	if(state!= PLAY)
	{ 
		get_rgb_wrapper(&r, &g, &b);			
		printf("\tIN:\t%hhu\t%hhu\t%hhu",r, g, b);
		if(state!=RECORD) printf("\r\n");
	}
	if(state==RECORD)
	{
		get_calibrated_frame_vals(	&(vid_frames[R][frame_count]), 
									&(vid_frames[G][frame_count]), 
									&(vid_frames[B][frame_count]), 
									r, g, b);

		//printf("IN:\t%hhu\t%hhu\t%hhu\r\n",r, g, b);
		printf("\t\tOUT:\t%hhu\t%hhu\t%hhu\r\n",vid_frames[R][frame_count], vid_frames[G][frame_count], vid_frames[B][frame_count]);
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
		set_rgb(vid_frames[R][frame_count], vid_frames[G][frame_count], vid_frames[B][frame_count]);
		frame_count++;
		frame_count=frame_count%total_frames;
	}
	delay_ms(FRAME_DELAY);
}

void get_rgb_wrapper(uint8_t* r_dest, uint8_t* g_dest, uint8_t* b_dest)
{
	int16_t r,g,b;
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
	
	tmp_r_out = calib_result[R][R]*(r*1.) + calib_result[R][G]*(g*1.) + calib_result[R][B]*(b*1.);
	tmp_g_out = calib_result[G][R]*(r*1.) + calib_result[G][G]*(g*1.) + calib_result[G][B]*(b*1.);
	tmp_b_out = calib_result[B][R]*(r*1.) + calib_result[B][G]*(g*1.) + calib_result[B][B]*(b*1.);
	
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

void calc_calib_matrix()
{	
	//First, calculate cm transpose.
	uint8_t cmT[3][NUM_CALIB_MEAS];
	for(uint8_t i=0;i<3;i++)
	{
		for(uint8_t j=0;j<NUM_CALIB_MEAS;j++)
		{
			cmT[i][j]=cm[j][i];
		}
	}
	
	//Next, calculate the information matrix: tr(cm)*cm
	float infoM[3][3];
	for(uint8_t i=0;i<3;i++)
	{
		for(uint8_t j=0;j<3;j++)
		{
			infoM[i][j]=0;
			for(uint8_t k=0;k<NUM_CALIB_MEAS;k++)
			{
				infoM[i][j]+=(uint16_t)cmT[i][k]*(uint16_t)cm[k][j];
			}
		}
	}
	
	//Next, invert the information matrix.
	float infoMInv[3][3];
	float infoMdet = infoM[R][R]*(infoM[G][G]*infoM[B][B]-infoM[G][B]*infoM[B][G])-
					 infoM[R][G]*(infoM[G][R]*infoM[B][B]-infoM[G][B]*infoM[B][R])+
					 infoM[R][B]*(infoM[G][R]*infoM[B][G]-infoM[G][G]*infoM[B][R]);			 
	infoMInv[R][R] = (infoM[G][G]*infoM[B][B]-infoM[G][B]*infoM[B][G])/infoMdet;
	infoMInv[R][G] = (infoM[R][B]*infoM[B][G]-infoM[R][G]*infoM[B][B])/infoMdet;
	infoMInv[R][B] = (infoM[R][G]*infoM[G][B]-infoM[R][B]*infoM[G][G])/infoMdet;
	infoMInv[G][R] = (infoM[G][B]*infoM[B][R]-infoM[G][R]*infoM[B][B])/infoMdet;
	infoMInv[G][G] = (infoM[R][R]*infoM[B][B]-infoM[R][B]*infoM[B][R])/infoMdet;
	infoMInv[G][B] = (infoM[R][B]*infoM[G][R]-infoM[R][R]*infoM[G][B])/infoMdet;
	infoMInv[B][R] = (infoM[G][R]*infoM[B][G]-infoM[G][G]*infoM[B][R])/infoMdet;
	infoMInv[B][G] = (infoM[R][G]*infoM[B][R]-infoM[R][R]*infoM[B][G])/infoMdet;
	infoMInv[B][B] = (infoM[R][R]*infoM[G][G]-infoM[R][G]*infoM[G][R])/infoMdet;

	//Next, calculate ((tr(cm)*cm)^-1)*tr(cm) 
	float magic[3][NUM_CALIB_MEAS];
	for(uint8_t i=0;i<3;i++)
	{
		for(uint8_t j=0;j<NUM_CALIB_MEAS;j++)
		{
			magic[i][j]=0;
			for(uint8_t k=0;k<3;k++)
			{
				magic[i][j]+=infoMInv[i][k]*cmT[k][j];
			}
		}
	}
	
	//Finally, we can get the calibration values by multiplying magic (((tr(cm)*cm)^-1)*tr(cm))) by our known values for the projector.
	for(uint8_t i=0;i<3;i++)
	{
		for(uint8_t j=0;j<3;j++)
		{
			calib_result[i][j]=0;
			for(uint8_t k=0;k<NUM_CALIB_MEAS;k++)
			{
				calib_result[i][j]+=magic[j][k]*(float)known_calib_vals[i][k];
			}
		}
	}
	
	//printf("Calib Matrix:\r\n");
	//for(uint8_t i=0;i<7;i++) printf("%c:\t%ld\t%ld\t%ld\r\n", col_names[i], cm[i][R], cm[i][G], cm[i][B]);
	printf("\r\nCalib Result:\r\n");
	for(uint8_t i=0;i<3;i++) printf("\t%f\t%f\t%f\r\n", calib_result[i][R], calib_result[i][G], calib_result[i][B]);
	printf("\r\n");
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{

}
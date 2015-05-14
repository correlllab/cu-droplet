//#include "droplet_programs/color_calib.h"
//
///*
 //* Any code in this function will be run once, when the robot starts.
 //*/
//void init()
//{
	//curr_frame=0;
	//last_msg=0;
  	//take_meas=0;
	//stop_meas=0;
//}
///*
 //* The code in this function will be called repeatedly, as fast as it can execute.
 //*/
//void loop()
//{
	//uint8_t r, g, b;
	//if(take_meas)
	//{
		//if((get_time()-last_msg)>500)
		//{			
			//get_rgb_sensors(&r, &g, &b);
			//cm[curr_frame][R] = r;
			//cm[curr_frame][G] = g;
			//cm[curr_frame][B] = b;
			//take_meas=0;
			//printf("%03hu %03hu %03hu\t<-\t%03hu %03hu %03hu\r\n",r, g, b,	known_calib_vals[curr_frame][R], 
																			//known_calib_vals[curr_frame][G],
																			//known_calib_vals[curr_frame][B]);
			//curr_frame++;																			
			//if(curr_frame>=NUM_CALIB_MEAS)
			//{
				//printf("Got all measurements. Calculating..\r\n");
				//stop_meas=1;
				//calc_calib_matrix();
			//}
		//}
	//}
	//delay_ms(20);
//}
//
///*
 //* After each pass through loop(), the robot checks for all messages it has 
 //* received, and calls this function once for each message.
 //*/
//void handle_msg(ir_msg* msg_struct)
//{
	//if(stop_meas) return;
	//if(msg_struct->length==5)
	//{
		//take_meas=1;
		//last_msg=get_time();
		//printf("Got '!'\t");
	//}
//}
//
//void calc_calib_matrix()
//{	
	////First, calculate cm transpose.
	//uint8_t cmT[3][NUM_CALIB_MEAS];
	//for(uint8_t i=0;i<3;i++)
	//{
		//for(uint8_t j=0;j<NUM_CALIB_MEAS;j++)
		//{
			//cmT[i][j]=cm[j][i];
		//}
	//}
	//
	////Next, calculate the information matrix: tr(cm)*cm
	//float infoM[3][3];
	//for(uint8_t i=0;i<3;i++)
	//{
		//for(uint8_t j=0;j<3;j++)
		//{
			//infoM[i][j]=0;
			//for(uint8_t k=0;k<NUM_CALIB_MEAS;k++)
			//{
				//infoM[i][j]+=(uint16_t)cmT[i][k]*(uint16_t)cm[k][j];
			//}
		//}
	//}
	//
	////Next, invert the information matrix.
	//float infoMInv[3][3];
	//float infoMdet = infoM[R][R]*(infoM[G][G]*infoM[B][B]-infoM[G][B]*infoM[B][G])-
					 //infoM[R][G]*(infoM[G][R]*infoM[B][B]-infoM[G][B]*infoM[B][R])+
					 //infoM[R][B]*(infoM[G][R]*infoM[B][G]-infoM[G][G]*infoM[B][R]);			 
	//infoMInv[R][R] = (infoM[G][G]*infoM[B][B]-infoM[G][B]*infoM[B][G])/infoMdet;
	//infoMInv[R][G] = (infoM[R][B]*infoM[B][G]-infoM[R][G]*infoM[B][B])/infoMdet;
	//infoMInv[R][B] = (infoM[R][G]*infoM[G][B]-infoM[R][B]*infoM[G][G])/infoMdet;
	//infoMInv[G][R] = (infoM[G][B]*infoM[B][R]-infoM[G][R]*infoM[B][B])/infoMdet;
	//infoMInv[G][G] = (infoM[R][R]*infoM[B][B]-infoM[R][B]*infoM[B][R])/infoMdet;
	//infoMInv[G][B] = (infoM[R][B]*infoM[G][R]-infoM[R][R]*infoM[G][B])/infoMdet;
	//infoMInv[B][R] = (infoM[G][R]*infoM[B][G]-infoM[G][G]*infoM[B][R])/infoMdet;
	//infoMInv[B][G] = (infoM[R][G]*infoM[B][R]-infoM[R][R]*infoM[B][G])/infoMdet;
	//infoMInv[B][B] = (infoM[R][R]*infoM[G][G]-infoM[R][G]*infoM[G][R])/infoMdet;
//
	////Next, calculate ((tr(cm)*cm)^-1)*tr(cm) 
	//float magic[3][NUM_CALIB_MEAS];
	//for(uint8_t i=0;i<3;i++)
	//{
		//for(uint8_t j=0;j<NUM_CALIB_MEAS;j++)
		//{
			//magic[i][j]=0;
			//for(uint8_t k=0;k<3;k++)
			//{
				//magic[i][j]+=infoMInv[i][k]*cmT[k][j];
			//}
		//}
	//}
	//
	////Finally, we can get the calibration values by multiplying magic (((tr(cm)*cm)^-1)*tr(cm))) by our known values for the projector.
	//for(uint8_t i=0;i<3;i++)
	//{
		//for(uint8_t j=0;j<3;j++)
		//{
			//calib_result[i][j]=0;
			//for(uint8_t k=0;k<NUM_CALIB_MEAS;k++)
			//{
				//calib_result[i][j]+=magic[j][k]*(float)known_calib_vals[k][i];
			//}
		//}
	//}
	//printf("\r\nColor Calib Matrix:\r\n");
	//for(uint8_t i=0;i<3;i++) printf("\t%f\t%f\t%f\r\n", calib_result[i][R], calib_result[i][G], calib_result[i][B]);
	//printf("\r\n");
	//write_color_settings(calib_result);
//}
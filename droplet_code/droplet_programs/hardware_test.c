//#include "droplet_programs/hardware_test.h"
//
//const uint8_t octave = 3;
							//
//uint16_t one_up_durs[6] = {120, 120, 120, 120, 120, 120};
//uint8_t one_up_notes[6] = {0x44,0x47,0x54,0x40,0x42,0x57};
//int16_t last_hue;
//uint8_t waiting;
//uint8_t moving_dir;
//uint16_t tetris_durs[58] = {4,2,2,4,2,2,4,2,2,4,2,2,6,2,4,4,4,4,2,2,2,2,6,2,4,2,2,6,2,4,2,2,4,2,2,
							//4,4,4,4,4,4,8,8,8,8,8,8,8,4,4,8,8,8,8,4,4,8,8};
//uint8_t tetris_notes[58] = {0x44,0x3b,0x40,0x42,0x40,0x3b,0x39,0x39,0x40,0x44,0x42,0x40,0x3b,0x40,
							//0x42,0x44,0x40,0x39,0x39,0x39,0x3b,0x40,0x42,0x45,0x49,0x47,0x45,0x44,
							//0x40,0x44,0x42,0x40,0x3b,0x3b,0x40,0x42,0x44,0x40,0x39,0x39,0xFF,0x44,
							//0x40,0x42,0x3b,0x40,0x39,0x38,0x3b,0xFF,0x44,0x40,0x42,0x3b,0x40,0x44,0x49,0x48};
//
///*
 //* Any code in this function will be run once, when the robot starts.
 //*/
//void init()
//{
	//uint16_t tempo=144;
	//
	//for(uint8_t i=0;i<58;i++)
	//{
		//tetris_durs[i]*=(15000/tempo);
	//}
	//for(uint8_t i=0;i<58;i++)//octave switch
	//{ 
		//tetris_notes[i]-=0x10;
	//}	
	//last_hue=0;
	//state = MIC_TEST;
	//moving_dir=0;
	//waiting=1;
	//state_changed_printout();	
//}
//
///*
 //* The code in this function will be called repeatedly, as fast as it can execute.
 //*/
//void loop()
//{	
	//uint16_t r,g,b,c;
	//if(waiting)
	//{
		//delay_ms(150);
		//return;
	//}
	//switch(state)
	//{
		//case MIC_TEST:
			//#ifdef AUDIO_DROPLET
				//printf("\t%d\r\n",get_mic_reading());
			//#endif
			//delay_ms(50);
			//break;
		//case IR_IO_TEST:
			//printf("\t%02hX\r\n", check_collisions());
			//delay_ms(250);
			//break;
		//case RGB_IO_TEST:
			//last_hue = (last_hue+5)%360;
			//set_hsv(last_hue, 255, 100);
			//get_rgb(&r, &g, &b);
			//printf("\t%5u %5u %5u\r\n",r,g,b);
			//delay_ms(150);
			//break;
		//case MOTOR_TEST: 
		///* This is used to test the motors, but also to 'burn them in'.
		 //* I have noticed that motor performance a bit changes a bit 
		 //* when they are first used, before settling down to something consistent.
		 //*/
			//if(is_moving()<0)
			//{
				//move_steps(moving_dir,20);
				//moving_dir = (moving_dir+1)%2;
			//}
			//delay_ms(150);			
			//break;
		//case SPEAKER_TEST:
			//#ifdef AUDIO_DROPLET
				//play_song(one_up_notes, one_up_durs, 6,0);
				//delay_ms(10000);	
			//#endif					
			//break;
		//case DONE:
			//delay_ms(150);
	//}
//}
//
///*
 //* After each pass through loop(), the robot checks for all messages it has 
 //* received, and calls this function once for each message.
 //*/
//void handle_msg(ir_msg* msg_struct)
//{
	//
//}
//
///*
 //* The function below is optional, and need not be included. If included,
 //* it should return '1' if the command_word is a valid command, and '0', 
 //* otherwise.
 //*/
//uint8_t user_handle_command(char* command_word, char* command_args)
//{
	//if(command_word[0]!='x') return 0; //If it is 'x', then its time for the next test.
	//if(waiting)
	//{
		//waiting=0;
		//return 1;
	//}
	//else
	//{
		//waiting=1;
		//state++;
	//}
	//state_changed_printout();
	//return 1;
//}
//
//void state_changed_printout()
//{
	//switch(state)
	//{
		//case MIC_TEST:
			//#ifdef AUDIO_DROPLET
				//printf("\r\nStarting Droplet Tests for %X.\r\n\n", get_droplet_id());
				//printf("Testing Microphone.\r\nValues should change based on sounds.\r\n");
			//#else
				//printf("This Droplet doesn't have a mic! Nothing to test.\r\n");
				//waiting=0;
			//#endif				
			//return 1;
		//case IR_IO_TEST:
			//printf("Testing IR Sensors/Emitters.\r\nCover each with something and the corresponding bit should flip.\r\n");
			//return 1;
		//case RGB_IO_TEST:
			//printf("Testing RGB LED and Sensor.\r\nShould see oscillating light, with sensor matching.\r\n");
			//return 1;
		//case MOTOR_TEST:
			//set_rgb(0,0,0);
			//printf("Testing motors.\r\nThey should vibrate.\r\n");
			//motor_adjusts[0][1]=1000;
			//motor_adjusts[0][2]=1000;
			//motor_adjusts[0][3]=1000;
			//motor_adjusts[1][1]=-1000;
			//motor_adjusts[1][2]=-1000;
			//motor_adjusts[1][3]=-1000;
			//moving_dir=0;
			//return 1;	
		//case SPEAKER_TEST:
			//#ifdef AUDIO_DROPLET
				//printf("Testing speaker.\r\nShould hear the Mario one-up jingle.\r\n");
			//#else
				//printf("This Droplet doesn't have a speaker! Nothing to test.\r\n");
				//waiting=0;
			//#endif			
			//return 1;
		//case DONE:
			//stop_move();
			//printf("All tests completed for %X\r\n",get_droplet_id());
			//waiting=0;			
			//return 1;
		//default:
			//printf("All tests already completed.\r\n");
			//waiting=0;			
			//return 1;
	//}
//}

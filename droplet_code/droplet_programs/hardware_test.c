#include "droplet_programs/hardware_test.h"

//const uint8_t octave = 3;
//const sound_str sound1, sound2, sound3, sound4, sound5, sound6;
//sound1.dur=sound2.dur=sound3.dur=sound4.dur=sound5.dur=sound6.dur=150;
//sound1.note=sound2.note=sound3.note=sound4.note=sound5.note=sound6.note=(octave<<4);
//sound1.note+=4;
//sound2.note+=7;
//sound3.note+=(1<<4)|4;
//sound4.note+=0;
//sound5.note+=2;
//sound6.note+=(1<<4)|7;
							
uint16_t one_up_durs[6] = {120, 120, 120, 120, 120, 120};
uint8_t one_up_notes[6] = {0x44,0x47,0x54,0x40,0x42,0x57};
int16_t last_hue;
uint8_t waiting;	
//uint16_t tetris_durs[58] = {4,2,2,4,2,2,4,2,2,4,2,2,6,2,4,4,4,4,2,2,2,2,6,2,4,2,2,6,2,4,2,2,4,2,2,
							//4,4,4,4,4,4,8,8,8,8,8,8,8,4,4,8,8,8,8,4,4,8,8};
//uint8_t tetris_notes[58] = {0x44,0x3b,0x40,0x42,0x40,0x3b,0x39,0x39,0x40,0x44,0x42,0x40,0x3b,0x40,
							//0x42,0x44,0x40,0x39,0x39,0x39,0x3b,0x40,0x42,0x45,0x49,0x47,0x45,0x44,
							//0x40,0x44,0x42,0x40,0x3b,0x3b,0x40,0x42,0x44,0x40,0x39,0x39,0xFF,0x44,
							//0x40,0x42,0x3b,0x40,0x39,0x38,0x3b,0xFF,0x44,0x40,0x42,0x3b,0x40,0x44,0x49,0x48};

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	//uint16_t tempo=144;
	
	//for(uint8_t i=0;i<58;i++)
	//{
		//tetris_durs[i]*=(15000/tempo);
	//}
	//for(uint8_t i=0;i<58;i++)//octave switch
	//{ 
		//tetris_notes[i]-=0x10;
	//}	
	last_hue=((((uint16_t)rand_byte())<<8)|((uint16_t)rand_byte()))%360;
	printf("Hue: %d\r\n",last_hue);
	state = MIC_TEST;
	waiting=1;
	state_changed_printout();	
}

/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{	
	uint16_t r,g,b,c;
	if(waiting)
	{
		delay_ms(150);
		return;
	}
	switch(state)
	{
		case MIC_TEST:
			printf("\t%d\r\n",get_mic_reading());
			delay_ms(50);
			break;
		case IR_IO_TEST:
			printf("\t%02hX\r\n", check_collisions());
			delay_ms(150);
			break;
		case RGB_IO_TEST:
			last_hue=last_hue+(((int8_t)(rand_byte()%29))-14);
			if(last_hue<0)		   last_hue+=360;
			else if(last_hue>=360) last_hue-=360;
			set_hsv(last_hue, 255, 100);
			printf("\tHue: %d\r\n",last_hue);			
			get_rgb(&r, &g, &b, &c);
			printf("\t%5u %5u %5u %5u\r\n",r,g,b,c);
			delay_ms(150);
			break;
		case MOTOR_TEST:
			if(is_moving()<0) move_steps(0,100);
			delay_ms(150);			
			break;
		case SPEAKER_TEST:
			play_song(one_up_notes, one_up_durs, 6);
			delay_ms(10000);			
			break;
		case DONE:
			delay_ms(150);
	}
	//delay_ms(1000);
}

/*
 * After each pass through loop(), the robot checks for all messages it has 
 * received, and calls this function once for each message.
 */
void handle_msg(ir_msg* msg_struct)
{
	
}

/*
 * The function below is optional, and need not be included. If included,
 * it should return '1' if the command_word is a valid command, and '0', 
 * otherwise.
 */
uint8_t user_handle_command(char* command_word, char* command_args)
{
	if(command_word[0]!='X') return; //If it is 'X', then its time for the next test.
	if(waiting)
	{
		waiting=0;
		return;
	}
	else
	{
		waiting=1;
		state++;
	}
	state_changed_printout();
}

void state_changed_printout()
{
	switch(state)
	{
		case MIC_TEST:
			printf("\r\nStarting Droplet Tests for %X.\r\n\n", get_droplet_id());
			printf("Testing Microphone.\r\nValues should change based on sounds.\r\n");
			return 1;
		case IR_IO_TEST:
			printf("Testing IR Sensors/Emitters.\r\nCover each with something and the corresponding bit should flip.\r\n");
			return 1;
		case RGB_IO_TEST:
			printf("Testing RGB LED and Sensor.\r\nShould see oscillating light, with sensor matching.\r\n");
			return 1;
		case SPEAKER_TEST:
			printf("Testing speaker.\r\nShould hear the Mario one-up jingle.\r\n");
			play_song(one_up_notes, one_up_durs, 6);
			return 1;
		case MOTOR_TEST:
			set_rgb(0,0,0);
			printf("Testing motors.\r\nThey should vibrate.\r\n");
			motor_adjusts[0][1]=1000;
			motor_adjusts[0][2]=1000;
			return 1;
		case DONE:
			stop_move();
			printf("All tests completed for %X\r\n",get_droplet_id());
			return 1;
		default:
			printf("All tests already completed.\r\n");
			return 1;
	}
}

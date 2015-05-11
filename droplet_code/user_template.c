#include "user_template.h"

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
	
uint16_t tetris_durs[58] = {4,2,2,4,2,2,4,2,2,4,2,2,6,2,4,4,4,4,2,2,2,2,6,2,4,2,2,6,2,4,2,2,4,2,2,
							4,4,4,4,4,4,8,8,8,8,8,8,8,4,4,8,8,8,8,4,4,8,8};
uint8_t tetris_notes[58] = {0x44,0x3b,0x40,0x42,0x40,0x3b,0x39,0x39,0x40,0x44,0x42,0x40,0x3b,0x40,
							0x42,0x44,0x40,0x39,0x39,0x39,0x3b,0x40,0x42,0x45,0x49,0x47,0x45,0x44,
							0x40,0x44,0x42,0x40,0x3b,0x3b,0x40,0x42,0x44,0x40,0x39,0x39,0xFF,0x44,
							0x40,0x42,0x3b,0x40,0x39,0x38,0x3b,0xFF,0x44,0x40,0x42,0x3b,0x40,0x44,0x49,0x48};

/*
 * Any code in this function will be run once, when the robot starts.
 */
void init()
{
	uint16_t tempo=144;
	
	for(uint8_t i=0;i<58;i++)
	{
		tetris_durs[i]*=(15000/tempo);
	}
	for(uint8_t i=0;i<58;i++)//octave switch
	{ 
		tetris_notes[i]-=0x10;
	}	
	last_r = 0;
	last_g = 0;
	last_b = 0;
	last_c = 0;
	last_meas_time=0;
}

uint8_t stupid_do_once = 0;


/*
 * The code in this function will be called repeatedly, as fast as it can execute.
 */
void loop()
{	
	delay_ms(10);
	if(!stupid_do_once)
	{
		rgb_power_on();
		stupid_do_once=1;
	}
	//play_song(tetris_notes, tetris_durs, 58);	
	//delay_ms(60000);
	uint16_t r,g,b,c;
	get_rgb(&r,&g,&b,&c);
	if(!(r==last_r&&g==last_g&&b==last_b&&c==last_c))
	{
		//something changed
		printf("%5u %5u %5u %5u | %lu\r\n",r,g,b,c, get_time()-last_meas_time);
		last_r=r;
		last_g=g;
		last_b=b;
		last_c=c;		
		last_meas_time=get_time();
	}
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
	
}

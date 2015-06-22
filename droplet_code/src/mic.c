#include "mic.h"

//The code below assumes that ir_sensor_init() has already been called, which does some of the set up for ADCB.
void mic_init()
{
	PORTB.DIRCLR = PIN5_bm;
	PORTB.PIN5CTRL = PORT_OPC_WIREDOR_gc | PORT_ISC_INPUT_DISABLE_gc;
	PORTD.DIRCLR = PIN5_bm;
	PORTD.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
	////port d, pin 5
	
	ADCB.CH3.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_8X_gc;
	ADCB.CH3.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE4_gc | ADC_CH_MUXPOS_PIN5_gc;
	
}

int16_t get_mic_reading()
{
	int16_t reading;
	ADCB.CH3.CTRL |= ADC_CH_START_bm;
	while (ADCB.CH3.INTFLAGS==0);
	reading = (((((int16_t)(ADCB.CH3.RESH))<<8)|((int16_t)(ADCB.CH3.RESL))));
	return reading;
}

//length can't be much higher than 1000 before we run out of memory.
//Warning! I have not tested a sample rate higher than 10kHz.
//Since each ADC measurement is 12 bits long, I'm packing 4 measurements in every 3 bytes, below.
void mic_recording(uint16_t length, uint16_t sample_rate)
{
		uint32_t time_start = get_time();
		uint16_t sample_delay = ((uint16_t)(((uint32_t)1000000)/((uint32_t)sample_rate)));
		uint16_t array_len = 3*(length/4);
		int16_t recording[array_len];
		uint8_t delta_time_stamp[array_len/3];
		int16_t mic_reading_temp;
		uint32_t prev_time=get_time();
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			for(uint16_t i=0;i<array_len;i+=3)
			{
				recording[i] = (0x0FFF&get_mic_reading());
				busy_delay_us(sample_delay);
				mic_reading_temp = get_mic_reading();
				recording[i] |= 0xF000&(mic_reading_temp<<12);
				recording[i+1] = 0x00FF&(mic_reading_temp>>4);
				busy_delay_us(sample_delay);
				mic_reading_temp = get_mic_reading();
				recording[i+1] |= 0xFF00&(mic_reading_temp<<8);
				recording[i+2] = 0x000F&(mic_reading_temp>>8);
				busy_delay_us(sample_delay);
				recording[i+2] |= (0xFFF0&(get_mic_reading()<<4));
				busy_delay_us(sample_delay);
				//if(i==0) printf("Just took first four measurements: %lu\r\n",get_time());
			}
		}
		printf("{", get_time()-time_start);
		for(uint16_t i=0;i<array_len-3;i+=3)
		{
			printf("%4d, ",((0x0FFF&recording[i])<<4)>>4);
			printf("%4d, ", (((0x000F&(recording[i]>>12))|(0x0FF0&(recording[i+1]<<4)))<<4)>>4);
			printf("%4d, ",(((0x00FF&(recording[i+1]>>8))|(0x0F00&(recording[i+2]<<8)))<<4)>>4);
			printf("%4d, ", recording[i+2]>>4);
			
		}
		printf("%4d, ",((0x0FFF&recording[array_len-3])<<4)>>4);
		printf("%4d, ", (((0x000F&(recording[array_len-3]>>12))|(0x0FF0&(recording[array_len-2]<<4)))<<4)>>4);
		printf("%4d, ",(((0x00FF&(recording[array_len-2]>>8))|(0x0F00&(recording[array_len-1]<<8)))<<4)>>4);
		printf("%4d}", recording[array_len-1]>>4);
}
#include "mic.h"

//The code below assumes that ir_sensor_init() has already been called, which does some of the set up for ADCB.
void mic_init()
{
	#ifdef AUDIO_DROPLET	
		PORTB.DIRCLR = PIN5_bm;
		PORTB.PIN5CTRL = PORT_OPC_WIREDOR_gc | PORT_ISC_INPUT_DISABLE_gc;
		PORTD.DIRCLR = PIN5_bm;
		PORTD.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;

		ADCB.CH3.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_8X_gc;
		ADCB.CH3.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE4_gc | ADC_CH_MUXPOS_PIN5_gc;
	#else
		printf_P(PSTR("ERROR: mic_init called, but this Droplet doesn't have a mic!\r\n"));		
	#endif
}

int16_t get_mic_reading()
{
	#ifdef AUDIO_DROPLET	
		int16_t reading;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			ADCB.CH3.CTRL |= ADC_CH_START_bm;
			while (ADCB.CH3.INTFLAGS==0);
		}
		reading = (((((int16_t)(ADCB.CH3.RESH))<<8)|((int16_t)(ADCB.CH3.RESL))));
		return reading;
	#else
		printf_P(PSTR("ERROR: get_mic_reading called, but this Droplet doesn't have a mic!\r\n"));
		return 0;
	#endif
}

//length can't be much higher than 1000 before we run out of memory.
//Warning! I have not tested a sample rate higher than 10kHz.
//Since each ADC measurement is 12 bits long, I'm packing 4 measurements in every 3 bytes, below.
//Array length must be divisible by 3.
void mic_recording(uint16_t* recording, uint16_t array_len, uint16_t sample_rate){
	#ifdef AUDIO_DROPLET
	if(array_len%3!=0) printf("Array length must be divisible by 3.\r\n");
	uint32_t time_start = get_time();
	uint16_t sample_delay = ((uint16_t)(((uint32_t)1000000)/((uint32_t)sample_rate)));
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
	#endif
}

void unpackMicRecording(uint16_t* unpacked, uint16_t unpacked_len, uint16_t* packed, uint16_t packed_len){
	if(unpacked_len!=((packed_len/3)*4)){
		printf("Error: unpacked length must be 4/3s of packed length.\r\n");
		return;
	}
	uint16_t unpackedIdx=0;
	for(uint16_t i=0;i<packed_len;i+=3)
	{
		unpacked[unpackedIdx]=((0x0FFF&packed[i])<<4)>>4;
		unpacked[unpackedIdx+1]=(((0x000F&(packed[i]>>12))|(0x0FF0&(packed[i+1]<<4)))<<4)>>4;
		unpacked[unpackedIdx+2]=(((0x00FF&(packed[i+1]>>8))|(0x0F00&(packed[i+2]<<8)))<<4)>>4;
		unpacked[unpackedIdx+3]=packed[i+2]>>4;
		unpackedIdx+=4;
	}	
}

void print_recording(uint16_t* recording, uint16_t array_len){
	#ifdef AUDIO_DROPLET
	printf("{");
	for(uint16_t i=0;i<array_len-3;i+=3)
	{
		printf_P(FORMATTED_PRINT_STRING,((0x0FFF&recording[i])<<4)>>4);
		printf_P(FORMATTED_PRINT_STRING, (((0x000F&(recording[i]>>12))|(0x0FF0&(recording[i+1]<<4)))<<4)>>4);
		printf_P(FORMATTED_PRINT_STRING,(((0x00FF&(recording[i+1]>>8))|(0x0F00&(recording[i+2]<<8)))<<4)>>4);
		printf_P(FORMATTED_PRINT_STRING, recording[i+2]>>4);
		
	}
	printf_P(FORMATTED_PRINT_STRING,((0x0FFF&recording[array_len-3])<<4)>>4);
	printf_P(FORMATTED_PRINT_STRING, (((0x000F&(recording[array_len-3]>>12))|(0x0FF0&(recording[array_len-2]<<4)))<<4)>>4);
	printf_P(FORMATTED_PRINT_STRING,(((0x00FF&(recording[array_len-2]>>8))|(0x0F00&(recording[array_len-1]<<8)))<<4)>>4);
	printf("%4d}", recording[array_len-1]>>4);
	#endif	
}
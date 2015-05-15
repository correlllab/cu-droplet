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
void mic_recording(uint16_t length)
{
		uint32_t time_start = get_time();
		int16_t recording[length];
		for(uint16_t i=0;i<length;i++)
		{
			recording[i] = get_mic_reading();
			delay_us(100);
		}
		printf("Recording took %lu ms.\r\n{", get_time()-time_start);
		for(uint16_t i=0;i<length-1;i++) printf("%d, ",recording[i]);
		printf("%d}\r\n\n",recording[length-1]);
}
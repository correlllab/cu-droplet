#include <avr/io.h>

#include "RGB_sensor.h"

void rgb_sensor_init()
{
	RGB_SENSOR_PORT.DIRCLR = RGB_SENSOR_R_PIN_bm | RGB_SENSOR_G_PIN_bm | RGB_SENSOR_B_PIN_bm;
	
	ADCA.REFCTRL = ADC_REFSEL_INT1V_gc | ADC_BANDGAP_bm;	// note: CONVMODE (in CTRLB register) is not set, this is a possible ERROR, see warning below
	ADCA.CTRLB = ADC_RESOLUTION_8BIT_gc;
	ADCA.CTRLB |= 0b00010000;	// set CONVMODE to signed mode  
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	
	/* WARNING! WHEN DIFFERENTIAL INPUT IS USED, SIGNED MODE MUST BE USED (sec. 28.6 of Manual) */

	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_4X_gc;	
	ADCA.CH1.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_4X_gc;
	ADCA.CH2.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_4X_gc;
	
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc;				// Red sensor on ADC A channel 0
	ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc;				// Green sensor on ADC A channel 1
	ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc;				// Blue sensor on ADC A channel 2
	
	// Differential mode requires choosing a negative voltage input (VINN)
	// set VINN to internal ground on negative differential input	
	// Q: Why dont we just use single-ended mode then ?
	// A: Single-ended mode does not allow use of GAIN 

	ADCA.CH0.MUXCTRL |= 0b00000111;		// 0b00000111 = Internal ground (ref. Table 28-15 in Manual)
	ADCA.CH1.MUXCTRL |= 0b00000111;		
	ADCA.CH2.MUXCTRL |= 0b00000111;		
	
	// Get baseline RGB sensor values
	rsenbase = gsenbase = bsenbase = 0xff;
	for (uint8_t i = 0; i < 255; i++)
	{
		ADCA.CTRLA = ADC_CH0START_bm | ADC_CH1START_bm | ADC_CH2START_bm | ADC_ENABLE_bm;
		while (ADCA.CH0.INTFLAGS==0);
		if (ADCA.CH0.RESL < rsenbase) rsenbase = ADCA.CH0.RESL;
		while (ADCA.CH1.INTFLAGS==0);
		if (ADCA.CH1.RESL < gsenbase) gsenbase = ADCA.CH1.RESL;
		while (ADCA.CH2.INTFLAGS==0);
		if (ADCA.CH2.RESL < bsenbase) bsenbase = ADCA.CH2.RESL;		
	}
}

uint8_t get_red_sensor()
{
	uint8_t cursen = 0;
	for (uint8_t i = 0; i < 87; i++)
	{
		ADCA.CH0.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH0.INTFLAGS==0);
		if (ADCA.CH0.RESL < rsenbase) rsenbase = ADCA.CH0.RESL;
		if (ADCA.CH0.RESL > cursen) cursen = ADCA.CH0.RESL;
	}
	return cursen - rsenbase;
}

uint8_t get_green_sensor()
{
	uint8_t cursen = 0;
	for (uint8_t i = 0; i < 87; i++)
	{
		ADCA.CH1.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH1.INTFLAGS==0);
		if (ADCA.CH1.RESL < gsenbase) gsenbase = ADCA.CH1.RESL;
		if (ADCA.CH1.RESL > cursen) cursen = ADCA.CH1.RESL;
	}		
	return ADCA.CH1.RESL - gsenbase;
}

uint8_t get_blue_sensor()
{
	uint8_t cursen = 0;
	for (uint8_t i = 0; i < 87; i++)
	{
		ADCA.CH2.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH2.INTFLAGS==0);
		if (ADCA.CH2.RESL < bsenbase) bsenbase = ADCA.CH2.RESL;
		if (ADCA.CH2.RESL > cursen) cursen = ADCA.CH2.RESL;
	}		
	return cursen - bsenbase;
}

void rgb_sensor_enable() 
{ 
	ADCA.CTRLA |= ADC_ENABLE_bm; 
}

void rgb_sensor_disable() 
{ 
	ADCA.CTRLA &= ~ADC_ENABLE_bm; 
}

#include "rgb_sensor.h"

void rgb_sensor_init()
{
	RGB_SENSOR_PORT.DIRCLR = RGB_SENSOR_R_PIN_bm | RGB_SENSOR_G_PIN_bm | RGB_SENSOR_B_PIN_bm;
	
	ADCA.CTRLA = ADC_ENABLE_bm;
	ADCA.REFCTRL = ADC_REFSEL_INT1V_gc | ADC_BANDGAP_bm;
	ADCA.CTRLB = ADC_RESOLUTION_8BIT_gc | ADC_CONMODE_bm;
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	/* When differential input is used, signed mode must be used. (sec. 28.6 of Manual) */
	ADCA.CALL = PRODSIGNATURES_ADCACAL0;
	ADCA.CALH = PRODSIGNATURES_ADCACAL1;

	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_4X_gc;	//Probably should turn the gain back up to 4X when we put the shells on.
	ADCA.CH1.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_4X_gc;	//Probably should turn the gain back up to 4X when we put the shells on.
	ADCA.CH2.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_4X_gc;	//Probably should turn the gain back up to 4X when we put the shells on.
	
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc;	// Red sensor on ADC A channel 0
	ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc;	// Green sensor on ADC A channel 1
	ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc;	// Blue sensor on ADC A channel 2

	ADCA.CH0.MUXCTRL |= ADC_CH_MUXNEG_INTGND_MODE4_gc;
	ADCA.CH1.MUXCTRL |= ADC_CH_MUXNEG_INTGND_MODE4_gc;
	ADCA.CH2.MUXCTRL |= ADC_CH_MUXNEG_INTGND_MODE4_gc;
	
}

int8_t get_red_sensor()
{
	int8_t cursen = 0;
	ADCA.CH0.CTRL |= ADC_CH_START_bm;
	while (ADCA.CH0.INTFLAGS==0);
	cursen = ADCA.CH0.RESL;
	return cursen;
}

int8_t get_green_sensor()
{
	int8_t cursen = 0;
	ADCA.CH1.CTRL |= ADC_CH_START_bm;
	while (ADCA.CH1.INTFLAGS==0);
	cursen = ADCA.CH1.RESL;
	return cursen;
}

int8_t get_blue_sensor()
{
	int8_t cursen = 0;
	ADCA.CH2.CTRL |= ADC_CH_START_bm;
	while (ADCA.CH2.INTFLAGS==0);
	cursen = ADCA.CH2.RESL;
	return cursen;
}

#include "RGB_sensor.h"

	///* SET INPUT PINS AS INPUTS */
	//IR_SENSOR_PORT.DIRCLR = ALL_IR_SENSOR_PINS_bm;
	//
//
	//ADCB.REFCTRL = /*ADC_BANDGAP_bm |*/ ADC_REFSEL_AREFB_gc;
	//ADCB.CTRLB = ADC_RESOLUTION_8BIT_gc | ADC_CONMODE_bm;
	//ADCB.PRESCALER = ADC_PRESCALER_DIV512_gc;
	//ADCB.CH0.CTRL = ADC_CH_INPUTMODE_DIFF_gc;	// differential input. requires signed mode (see sec. 28.6 in manual)
	//ADCB.CH0.MUXCTRL = ADC_CH_MUXNEG_PIN0_gc;	// use VREF_IN for the negative input (0.54 V)
	//ADCB.CTRLA = ADC_ENABLE_bm;
	////ADCB.CALL = PRODSIGNATURES_ADCBCAL0;
	////ADCB.CALH = PRODSIGNATURES_ADCBCAL1;
//
	///* FIND AND RECORD THE ZERO-OFFSET OF EACH IR DIRECTION */
	//IR_SENSOR_PORT.DIRSET = ALL_IR_SENSOR_PINS_bm;		// set the IR sense pins as OUTPUT
	//IR_SENSOR_PORT.OUTCLR = ALL_IR_SENSOR_PINS_bm;		// put a low voltage on these pins (typically, this will be about 15 mV)
//
	//for(uint8_t i=0; i<6; i++)
	//{
		//ADCB.CH0.MUXCTRL &= MUX_SENSOR_CLR; //clear previous sensor selection
		//ADCB.CH0.MUXCTRL |= mux_sensor_selectors[i];
		//busy_delay_us(250);
		//ADCB.CH0.CTRL |= ADC_CH_START_bm;
		//busy_delay_us(250);
		//while (ADCB.CH0.INTFLAGS==0);		// wait for 'complete flag' to be set
		//ADC_offset[i] = ADCB.CH0.RES*(-1);
	//}
	//IR_SENSOR_PORT.DIRCLR = ALL_IR_SENSOR_PINS_bm;
	////printf("Offsets: [0: %hhd, 1: %hhd, 2: %hhd, 3: %hhd, 4: %hhd, 5: %hhd\r\n",ADC_offset[0],ADC_offset[1],ADC_offset[2],ADC_offset[3],ADC_offset[4],ADC_offset[5]);
	//
	//////the commands below set the ir_emitters to output.
	//PORTC.DIRSET = (PIN3_bm | PIN7_bm);
	//PORTD.DIRCLR =  PIN3_bm;
	//PORTE.DIRSET = (PIN3_bm | PIN7_bm);
	//PORTF.DIRCLR =  PIN3_bm;

void rgb_sensor_init()
{
	RGB_SENSOR_PORT.DIRCLR = RGB_SENSOR_R_PIN_bm | RGB_SENSOR_G_PIN_bm | RGB_SENSOR_B_PIN_bm;
	
	ADCA.CTRLA = ADC_ENABLE_bm;
	ADCA.REFCTRL = ADC_REFSEL_INT1V_gc | ADC_BANDGAP_bm;
	ADCA.CTRLB = ADC_RESOLUTION_8BIT_gc | ADC_CONMODE_bm;
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	
	/* When differential input is used, signed mode must be used. (sec. 28.6 of Manual) */

	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_2X_gc;	//Probably should turn the gain back up to 4X when we put the shells on.
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

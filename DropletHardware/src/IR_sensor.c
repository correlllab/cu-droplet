#include "IR_sensor.h"


const uint8_t mux_sensor_selectors[6] = {MUX_IR_SENSOR_0, MUX_IR_SENSOR_1, MUX_IR_SENSOR_2, MUX_IR_SENSOR_3, MUX_IR_SENSOR_4, MUX_IR_SENSOR_5};
int8_t ADC_offset[6];		// this value will typically be about -72 if using 1.00 V reference and ADCB0 as VINN

// IR sensors use ADCB channel 0, all the time

void IR_sensor_init()
{
	/* SET INPUT PINS AS INPUTS */
	IR_SENSOR_PORT.DIRCLR = IR_SENSOR_0_PIN_bm | IR_SENSOR_1_PIN_bm | IR_SENSOR_2_PIN_bm | IR_SENSOR_3_PIN_bm | IR_SENSOR_4_PIN_bm | IR_SENSOR_5_PIN_bm;
	
	ADCB.CTRLB = ADC_RESOLUTION_8BIT_gc;		// use 8 bit resolution
	ADCB.CTRLB |= ADC_CONMODE_bm;				// switch to signed mode
	ADCB.PRESCALER = ADC_PRESCALER_DIV512_gc;
	ADCB.CH0.CTRL = ADC_CH_INPUTMODE_DIFF_gc;	// differential input. requires signed mode (see sec. 28.6 in manual)
	ADCB.CH0.MUXCTRL = ADC_CH_MUXNEG_PIN0_gc;	// use VREF_IN for the negative input (0.54 V)
	ADCB.CTRLA = ADC_ENABLE_bm;

	/* FIND AND RECORD THE ZERO-OFFSET OF EACH IR DIRECTION */

	PORTB.DIRSET = 0b11111100;		// set the IR sense pins as OUTPUT
	PORTB.OUTSET = 0b00000000;		// put a low voltage on these pins (typically, this will be about 15 mV)

	for(uint8_t i=0; i<6; i++)
	{
		ADCB.CH0.MUXCTRL &= MUX_SENSOR_CLR; //clear previous sensor selection		
		ADCB.CH0.MUXCTRL |= mux_sensor_selectors[i];
		ADCB.CTRLA |= ADC_CH0START_bm;
		while (ADCB.CH0.INTFLAGS==0);		// wait for 'complete flag' to be set
		ADC_offset[i] = ADCB.CH0.RES*(-1);
	}

	PORTB.DIRCLR = 0xFF;			// return the IR sense pins back to inputs
	//printf("Offsets: [0: %i, 1: %i, 2: %i, 3: %i, 4: %i, 5: %i\r\n",ADC_offset[0],ADC_offset[1],ADC_offset[2],ADC_offset[3],ADC_offset[4],ADC_offset[5]);
}

/*
* This measurement will always output a number between 0 and about 200. Ambient light levels are typically around 20.
* The range of outputs that could be used for actual measurements will be limited to about 20 to 200 (only 180 significant values)
*/

uint8_t get_IR_sensor(uint8_t sensor_num)
{
	int8_t meas[3]; //we're going to take three measurements, and then calculate their median.
		
	ADCB.CH0.MUXCTRL &= MUX_SENSOR_CLR; //clear previous sensor selection
	ADCB.CH0.MUXCTRL |= mux_sensor_selectors[sensor_num];
	
	for (uint8_t i = 0; i < 3; i++) 
	{
		ADCB.CTRLA |= ADC_CH0START_bm;
		while (ADCB.CH0.INTFLAGS==0);		// wait for measurement to complete
		meas[i] = ADCB.CH0.RES;
	}		
	
	return (uint8_t)(find_median(meas)+ADC_offset[sensor_num]);
}

// Finds the median of 3 numbers by finding the max, finding the min, and returning the other value
int8_t find_median(int8_t* meas)
{
	uint8_t mini, maxi, medi;
	int8_t min = -128, max = 127;
	for (uint8_t i = 0; i < 3; i++)
	{
		if (meas[i] < max)
		{
			max = meas[i]; 
			maxi = i;
		}
		if (meas[i] > min)
		{
			min = meas[i];
			mini = i;
		}
	}
	for (medi = 0; medi < 3; medi++)
	{
		if (medi != maxi & medi != mini) return meas[medi];
	}
}
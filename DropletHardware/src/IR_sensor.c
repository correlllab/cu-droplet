#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "sp_driver.h"
#include "stddef.h"				// required to use "offsetof(_,_)" for production row access
#include "RGB_LED.h"			// currently uses some color flashes for debugging

#include "IR_sensor.h"
#include "IRcom.h"

#define IR_SENSOR_0_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN5_gc		// IR0 sensor on PB5
#define IR_SENSOR_1_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN6_gc		// IR1 sensor on PB6
#define IR_SENSOR_2_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN7_gc		// IR2 sensor on PB7
#define IR_SENSOR_3_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN4_gc		// IR3 sensor on PB4
#define IR_SENSOR_4_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN2_gc		// IR4 sensor on PB2
#define IR_SENSOR_5_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN3_gc		// IR5 sensor on PB3

uint8_t ADCAcalibration0, ADCAcalibration1; //(for ADCA, the legs and the color sensors)
uint8_t ADCBcalibration0, ADCBcalibration1;


int8_t ADC_offset[6];		// this value will typically be about -72 if using 1.00 V reference and ADCB0 as VINN

int8_t find_median(int8_t* meas); // Helper function for getting the middle of the 3 measurements

void IR_sensor_init()
{
	// IR sensors use ADCB channel 0, all the time

	/* SET INPUT PINS AS INPUTS */
	
	IR_SENSOR_PORT.DIRCLR = IR_SENSOR_0_PIN_bm | IR_SENSOR_1_PIN_bm | IR_SENSOR_2_PIN_bm | IR_SENSOR_3_PIN_bm | IR_SENSOR_4_PIN_bm | IR_SENSOR_5_PIN_bm;
	
	// 28.16.3 REFCTRL – Reference Control register
	//
	// Bit 1 – BANDGAP: Bandgap enable
	// Setting this bit enables the Bandgap for ADC measurement. Note that if any other functions are
	// using the Bandgap already, this bit does not need to be set when the internal 1.00V reference is
	// used in ADC or DAC, or if the Brown-out Detector is enabled.
	//
	// Bits 6:4 – REFSEL[2:0]: ADC Reference Selection
	// These bits selects the reference for the ADC

	//ADCB.REFCTRL = ADC_REFSEL_VCC_gc;								// Vcc/1.6
	//ADCB.REFCTRL = 0b01000000;									// Vcc/2
	//ADCB.REFCTRL = 0b00100000;									// AREFA = 0.71 V
	//ADCB.REFCTRL = 0b00110000;									// AREFB = 0.54 V

	// 28.16.2 CTRLB – ADC Control Register B
	//
	// Bit 7 – IMPMODE: Gain Stage Impedance Mode
	// This bit controls the impedance mode of the gain stage. See GAIN setting with ADC Channel
	// Register description for more information.
	//
	// Bit 6:5 – CURRLIMIT[1:0]: Current Limitation
	// These bits can be used to limit the maximum current consumption of the ADC. Setting these bits
	// will also reduce the maximum sampling rate. The available settings is shown in Table 28-3 on
	// page 367. The indicated current limitations are nominal values, refer to device datasheet for
	// actual current limitation for each setting.
	//
	// Bit 4 – CONVMODE: ADC Conversion ModePlot
	// This bit controls whether the ADC will work in signed or unsigned mode. By default this bit is
	// cleared and the ADC is configured for unsigned mode. When this bit is set the ADC is configured
	// for signed mode.
	//
	// Bit 3 – FREERUN: ADC Free Running Mode
	// When the bit is set to one, the ADC is in free running mode and ADC channels defined in the
	// EVCTRL register are swept repeatedly.
	//
	// Bit 2:1 – RESOLUTION[1:0]: ADC Conversion Result Resolution
	// These bits define whether the ADC completes the conversion at 12- or 8-bit result. They also
	// define whether the 12-bit result is left or right oriented in the 16-bit result registers. See Table
	// 28-4 on page 367 for possible settings.

	ADCB.CTRLB = ADC_RESOLUTION_8BIT_gc;		// use 8 bit resolution
	ADCB.CTRLB |= ADC_CONMODE_bm;		// switch to signed mode
	
	ADCB.PRESCALER = ADC_PRESCALER_DIV512_gc;
	
	/* WARNING! WHEN DIFFERENTIAL INPUT IS USED, SIGNED MODE MUST BE USED (sec. 28.6 of Manual) */

	//ADCB.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;				// can use either signed or unsigned mode
	ADCB.CH0.CTRL = ADC_CH_INPUTMODE_DIFF_gc;						// requires selecting signed mode
	//ADCB.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc;				// requires selecting signed mode
	
	/* SELECT MUXNEG */
	
	// selecting the negative input sets where the zero of the signed output is
	// note that this is NOT a negative voltage, no negative voltages should ever be applied the the XMEGA
	
	ADCB.CH0.MUXCTRL = ADC_CH_MUXNEG_PIN0_gc;		// use VREF_IN for the negative input (0.54 V)


	/* READ & LOAD CALIBRATION (PRODUCTION SIGNATURE ROW) */	//TODO (unlikely will make any difference)
	
	ADCBcalibration0 = SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, ADCBCAL0 ));
	ADCBcalibration1 = SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, ADCBCAL1 ));
	ADCAcalibration0 = SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, ADCACAL0 ));
	ADCAcalibration1 = SP_ReadCalibrationByte(offsetof( NVM_PROD_SIGNATURES_t, ADCACAL1 ));

	// for Droplet3, the correct values are:
	//	ADCACAL0	68
	//	ADCACAL1	4
	//	ADCBCAL0	68
	//	ADCBCAL1	4

	// SHOULD WE WRITE THEM???
	
	//ADCB.CALL = ADCBcalibration0;
	//ADCB.CALH = ADCBcalibration1;
	//ADCB.CALL = 68;
	//ADCB.CALH = 4;
	
	ADCB.CTRLA = ADC_ENABLE_bm;

	/* FIND AND RECORD THE ZERO-OFFSET OF EACH IR DIRECTION */

	PORTB.DIRSET = 0b11111100;		// set the IR sense pins as OUTPUT
	PORTB.OUTSET = 0b00000000;		// put a low voltage on these pins (typically, this will be about 15 mV)

	ADCB.CH0.MUXCTRL |= IR_SENSOR_0_FOR_ADCB_MUXPOS;
	//_delay_ms(1);	// may have to wait a bit before the new multiplexer (MUX) connection is made in hardware?		TODO ???
	ADCB.CTRLA |= ADC_CH0START_bm;
	while (ADCB.CH0.INTFLAGS==0){};		// wait for 'complete flag' to be set
	ADCB.CH0.INTFLAGS = 1;				// clear the complete flag
	ADC_offset[0] = ADCB.CH0.RES*(-1);

	ADCB.CH0.MUXCTRL &= ~0b01111000;	// clear out the old value
	ADCB.CH0.MUXCTRL |= IR_SENSOR_1_FOR_ADCB_MUXPOS;
	//_delay_ms(1);	// may have to wait a bit before the new multiplexer (MUX) connection is made in hardware?		TODO ???
	ADCB.CTRLA |= ADC_CH0START_bm;
	while (ADCB.CH0.INTFLAGS==0){};		// wait for 'complete flag' to be set
	ADCB.CH0.INTFLAGS = 1;				// clear the complete flag
	ADC_offset[1] = ADCB.CH0.RES*(-1);

	ADCB.CH0.MUXCTRL &= ~0b01111000;	// clear out the old value
	ADCB.CH0.MUXCTRL |= IR_SENSOR_2_FOR_ADCB_MUXPOS;
	//_delay_ms(1);	// may have to wait a bit before the new multiplexer (MUX) connection is made in hardware?		TODO ???
	ADCB.CTRLA |= ADC_CH0START_bm;
	while (ADCB.CH0.INTFLAGS==0){};		// wait for 'complete flag' to be set
	ADCB.CH0.INTFLAGS = 1;				// clear the complete flag
	ADC_offset[2] = ADCB.CH0.RES*(-1);

	ADCB.CH0.MUXCTRL &= ~0b01111000;	// clear out the old value
	ADCB.CH0.MUXCTRL |= IR_SENSOR_3_FOR_ADCB_MUXPOS;
	//_delay_ms(1);	// may have to wait a bit before the new multiplexer (MUX) connection is made in hardware?		TODO ???
	ADCB.CTRLA |= ADC_CH0START_bm;
	while (ADCB.CH0.INTFLAGS==0){};		// wait for 'complete flag' to be set
	ADCB.CH0.INTFLAGS = 1;				// clear the complete flag
	ADC_offset[3] = ADCB.CH0.RES*(-1);

	ADCB.CH0.MUXCTRL &= ~0b01111000;	// clear out the old value
	ADCB.CH0.MUXCTRL |= IR_SENSOR_4_FOR_ADCB_MUXPOS;
	//_delay_ms(1);	// may have to wait a bit before the new multiplexer (MUX) connection is made in hardware?		TODO ???
	ADCB.CTRLA |= ADC_CH0START_bm;
	while (ADCB.CH0.INTFLAGS==0){};		// wait for 'complete flag' to be set
	ADCB.CH0.INTFLAGS = 1;				// clear the complete flag
	ADC_offset[4] = ADCB.CH0.RES*(-1);

	ADCB.CH0.MUXCTRL &= ~0b01111000;	// clear out the old value
	ADCB.CH0.MUXCTRL |= IR_SENSOR_5_FOR_ADCB_MUXPOS;
	//_delay_ms(1);	// may have to wait a bit before the new multiplexer (MUX) connection is made in hardware?		TODO ???
	ADCB.CTRLA |= ADC_CH0START_bm;
	while (ADCB.CH0.INTFLAGS==0){};		// wait for 'complete flag' to be set
	ADCB.CH0.INTFLAGS = 1;				// clear the complete flag
	ADC_offset[5] = ADCB.CH0.RES*(-1);
/*
	printf("ADC offset 0: %i\r\n",ADC_offset[0]);
	printf("ADC offset 1: %i\r\n",ADC_offset[1]);
	printf("ADC offset 2: %i\r\n",ADC_offset[2]);
	printf("ADC offset 3: %i\r\n",ADC_offset[3]);
	printf("ADC offset 4: %i\r\n",ADC_offset[4]);
	printf("ADC offset 5: %i\r\n",ADC_offset[5]);
*/
	PORTB.DIRCLR = 0xFF;			// return the IR sense pins back to inputs
	
	//printf("Offsets: [0: %i, 1: %i, 2: %i, 3: %i, 4: %i, 5: %i\r\n",ADC_offset[0],ADC_offset[1],ADC_offset[2],ADC_offset[3],ADC_offset[4],ADC_offset[5]);
}


uint8_t get_IR_sensor(uint8_t sensor_num)
{
	// this routine uses ADCB channel 0, all the time
	
	int8_t meas[3];
	int16_t average;
	int8_t scaled_median, scaled_average;
	
	//printf("\r\nget IR %i sen, CH0\r\n",sensor_num);

	switch(sensor_num)
	{
		case 0:
			ADCB.CH0.MUXCTRL &= ~0b01111000;	// clear out the old value
			ADCB.CH0.MUXCTRL |= IR_SENSOR_0_FOR_ADCB_MUXPOS;
			break;
		case 1:
			ADCB.CH0.MUXCTRL &= ~0b01111000;	// clear out the old value
			ADCB.CH0.MUXCTRL |= IR_SENSOR_1_FOR_ADCB_MUXPOS;
			break;
		case 2:
			ADCB.CH0.MUXCTRL &= ~0b01111000;	// clear out the old value
			ADCB.CH0.MUXCTRL |= IR_SENSOR_2_FOR_ADCB_MUXPOS;
			break;
		case 3:
			ADCB.CH0.MUXCTRL &= ~0b01111000;	// clear out the old value
			ADCB.CH0.MUXCTRL |= IR_SENSOR_3_FOR_ADCB_MUXPOS;
			break;
		case 4:
			ADCB.CH0.MUXCTRL &= ~0b01111000;	// clear out the old value
			ADCB.CH0.MUXCTRL |= IR_SENSOR_4_FOR_ADCB_MUXPOS;
			break;
		case 5:
			ADCB.CH0.MUXCTRL &= ~0b01111000;	// clear out the old value
			ADCB.CH0.MUXCTRL |= IR_SENSOR_5_FOR_ADCB_MUXPOS;
			break;
		default:
			return 0;
	}

	for (uint8_t i = 0; i < 3; i++)
	{
		ADCB.CTRLA |= ADC_CH0START_bm;
		while (ADCB.CH0.INTFLAGS==0){};		// wait for 'complete flag' to be set
		ADCB.CH0.INTFLAGS = 1;				// clear the complete flag 
		meas[i] = ADCB.CH0.RES;
		//printf("meas1: %i\r\n",meas1);
	}		

	average = ((int16_t)meas[0] + (int16_t)meas[1] + (int16_t)meas[2]) / 3;
	scaled_average = (uint8_t)(average+ADC_offset[sensor_num]);
	scaled_median = (uint8_t)(find_median(meas)+ADC_offset[sensor_num]);
	//printf("measA: %i, measB: %i, measC: %i\r\n",meas[0],meas[1],meas[2]);
	//printf("avg: %i, scale_avg: %u, scale_med: %u\r\n",average, scaled_average, scaled_median);
	return scaled_median;

	// notes on how this works:
	// the usual range of outputs for the 8 bit signed ADC is -127 to 128
	// right now the range of outputs we can actually get is limited by the negative input voltage to the ADC (VINN)
	// since we use ADCB pin0, with a constant voltage of 0.54 V, this sets the 0 of our scale (-127 to 128)
	// at 0.54 V, this also means the GND evaluates to about -73.  This means that values below -73 are inaccessible to us,
	// since we will never have an ADC input that is below GND.  That makes the new range -73 to 128.
	// when we shift this range to get only unsigned values, the new range becomes 0 to 201.
	// Conclusion: this measurement will always output a number between 0 and abt 200.
	// furthermore, experimentally, the ambient room light levels are typically around 20 (unsigned shift already applied, so 0 = GND)
	// and so the range of outputs that could be used for actual measurements will be limited to about 20 to 200 (only 180 significant values)
	// this range could be improved slightly by increasing the VINN voltage.  If ADCA pin0 is used, VINN becomes 0.71 V and the range is shifted upwards.

	return scaled_average;
}

void IR_sensor_enable() 
{ 
	ADCB.CTRLA |= ADC_ENABLE_bm; 
}

void IR_sensor_disable() 
{ 
	ADCB.CTRLA &= ~ADC_ENABLE_bm; 
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
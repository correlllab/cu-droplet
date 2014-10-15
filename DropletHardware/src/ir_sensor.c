#include "ir_sensor.h"

const uint8_t mux_sensor_selectors[6] = {MUX_IR_SENSOR_0, MUX_IR_SENSOR_1, MUX_IR_SENSOR_2, MUX_IR_SENSOR_3, MUX_IR_SENSOR_4, MUX_IR_SENSOR_5};
int8_t ADC_offset[6];		// this value will typically be about -72 if using 1.00 V reference and ADCB0 as VINN

USART_t* channel[] = {
	&USARTC0,  //   -- Channel 0
	&USARTC1,  //   -- Channel 1
	&USARTD0,  //   -- Channel 2
	&USARTE0,  //   -- Channel 3
	&USARTE1,  //   -- Channel 4
	&USARTF0   //   -- Channel 5
};

// IR sensors use ADCB channel 0, all the time
void ir_sensor_init()
{
	/* SET INPUT PINS AS INPUTS */
	IR_SENSOR_PORT.DIRCLR = ALL_IR_SENSOR_PINS_bm;

	ADCB.REFCTRL = ADC_REFSEL_AREFB_gc;
	ADCB.CTRLB = ADC_RESOLUTION_8BIT_gc | ADC_CONMODE_bm; //8bit resolution, and sets it to signed mode.
	ADCB.PRESCALER = ADC_PRESCALER_DIV512_gc;
	ADCB.CH0.CTRL = ADC_CH_INPUTMODE_DIFF_gc;	// differential input. requires signed mode (see sec. 28.6 in manual)
	ADCB.CH0.MUXCTRL = ADC_CH_MUXNEG_PIN0_gc;	// use VREF_IN for the negative input (0.54 V)
	ADCB.CALL = PRODSIGNATURES_ADCBCAL0;
	ADCB.CALH = PRODSIGNATURES_ADCBCAL1;
	ADCB.CTRLA = ADC_ENABLE_bm;	

	/* FIND AND RECORD THE ZERO-OFFSET OF EACH IR DIRECTION */
	IR_SENSOR_PORT.DIRSET = ALL_IR_SENSOR_PINS_bm;		// set the IR sense pins as OUTPUT
	IR_SENSOR_PORT.OUTCLR = ALL_IR_SENSOR_PINS_bm;		// put a low voltage on these pins (typically, this will be about 15 mV)

	for(uint8_t i=0; i<6; i++)
	{
		ADCB.CH0.MUXCTRL &= MUX_SENSOR_CLR; //clear previous sensor selection
		ADCB.CH0.MUXCTRL |= mux_sensor_selectors[i];
		busy_delay_us(250);		
		ADCB.CH0.CTRL |= ADC_CH_START_bm;
		busy_delay_us(250);				
		while (ADCB.CH0.INTFLAGS==0);		// wait for 'complete flag' to be set		
		ADC_offset[i] = ADCB.CH0.RES*(-1);
	}
	IR_SENSOR_PORT.DIRCLR = ALL_IR_SENSOR_PINS_bm;
	printf("Offsets: [0: %hhd, 1: %hhd, 2: %hhd, 3: %hhd, 4: %hhd, 5: %hhd\r\n",ADC_offset[0],ADC_offset[1],ADC_offset[2],ADC_offset[3],ADC_offset[4],ADC_offset[5]);
		
	////the commands below set the ir_emitters to output.
	PORTC.DIRSET = (PIN3_bm | PIN7_bm);
	PORTD.DIRCLR =  PIN3_bm;
	PORTE.DIRSET = (PIN3_bm | PIN7_bm);
	PORTF.DIRCLR =  PIN3_bm;	
	//PORTF.DIRSET = ALL_EMITTERS_CARWAV_bm;	//set carrier wave pins to output.
}

/*
* This measurement will always output a number between 0 and about 200. Ambient light levels are typically around 20.
* The range of outputs that could be used for actual measurements will be limited to about 20 to 200 (only 180 significant values)
*/

uint8_t get_ir_sensor(uint8_t sensor_num)
{
	int8_t meas; //we're going to take three measurements, and then calculate their median.
		
	ADCB.CH0.MUXCTRL &= MUX_SENSOR_CLR; //clear previous sensor selection
	ADCB.CH0.MUXCTRL |= mux_sensor_selectors[sensor_num];
	busy_delay_us(250);
	ADCB.CH0.CTRL |= ADC_CH_START_bm;
	busy_delay_us(250);			
	while (ADCB.CH0.INTFLAGS==0);		// wait for measurement to complete
	meas = ADCB.CH0.RESL;
	//return (uint8_t)(meas+ADC_offset[sensor_num]);
	//printf("\t%hhd %hhd %hhd\r\n",meas[0], meas[1], meas[2]);
	return (uint8_t)(meas+ADC_offset[sensor_num]);
}
	
uint8_t check_collisions(){
	uint8_t baseline_meas[6];
	uint8_t channelCtrlBVals[6];
	uint8_t measured_vals[6];
	uint8_t dirs=0;
	
	for(uint8_t i=0;i<6;i++)
	{

		channelCtrlBVals[i] = channel[i]->CTRLB;
		channel[i]->CTRLB=0;
	}	
	for(uint8_t i=0;i<6;i++)
	{	
		busy_delay_us(50);
		baseline_meas[i] = get_ir_sensor(i);
	}
	TCF2.CTRLB &= ~ALL_EMITTERS_CARWAV_bm;	//disable carrier wave output
	PORTF.OUTSET = ALL_EMITTERS_CARWAV_bm;	// set carrier wave pins high.
	PORTF.DIRSET = ALL_EMITTERS_CARWAV_bm;
	PORTC.DIRSET = (PIN3_bm | PIN7_bm);
	PORTD.DIRSET =  PIN3_bm;
	PORTE.DIRSET = (PIN3_bm | PIN7_bm);
	PORTF.DIRSET =  PIN3_bm;
		
	PORTC.OUTCLR = (PIN3_bm | PIN7_bm);
	PORTD.OUTCLR = PIN3_bm;
	PORTE.OUTCLR = (PIN3_bm | PIN7_bm);
	PORTF.OUTCLR = PIN3_bm;

	//busy_delay_us(250);
	//ADCB.CTRLA |= ADC_FLUSH_bm;
	//delay_ms(1000);
	for(uint8_t i=0;i<6;i++){
		busy_delay_us(250);		
		measured_vals[i] = get_ir_sensor(i);
		if((measured_vals[i]-baseline_meas[i])>10){
			dirs = dirs|(1<<i);
		}
		//busy_delay_us(50);
	}		

	PORTC.OUTTGL = (PIN3_bm | PIN7_bm);
	PORTD.OUTTGL =  PIN3_bm;
	PORTE.OUTTGL = (PIN3_bm | PIN7_bm);
	PORTF.OUTTGL =  PIN3_bm;
	for(uint8_t i=0;i<6;i++) channel[i]->CTRLB = channelCtrlBVals[i];
	TCF2.CTRLB |= ALL_EMITTERS_CARWAV_bm; //reenable carrier wave output
	
	return dirs;
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
		if ((medi != maxi) && (medi != mini)) return meas[medi];
	}
}
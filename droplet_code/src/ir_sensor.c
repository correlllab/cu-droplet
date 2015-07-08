#include "ir_sensor.h"

#ifdef AUDIO_DROPLET
	ADC_CH_t* ir_sense_channels[6] = {&(ADCA.CH0), &(ADCA.CH1), &(ADCA.CH2), &(ADCB.CH0), &(ADCB.CH1), &(ADCB.CH2)};
#else
	const uint8_t mux_sensor_selectors[6] = {MUX_IR_SENSOR_0, MUX_IR_SENSOR_1, MUX_IR_SENSOR_2, MUX_IR_SENSOR_3, MUX_IR_SENSOR_4, MUX_IR_SENSOR_5};
#endif

// IR sensors use ADCB channel 0, all the time
void ir_sensor_init()
{
	#ifdef AUDIO_DROPLET
		/* SET INPUT PINS AS INPUTS */
		PORTA.DIRCLR = PIN5_bm | PIN6_bm | PIN7_bm;
		PORTB.DIRCLR = PIN4_bm | PIN2_bm | PIN3_bm;
	
		//
		//IR SENSORS ACTING WEIRD? TRY COMMENTING OUT THE BELOW.
		//BELOW RECOMMENDED BY note on pg 153 of the manual
		//
		PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTA.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTB.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTB.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTB.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
	
		ADCA.REFCTRL = ADC_REFSEL_INT1V_gc;
		ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm;
		ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
		ADCA.CALL = PRODSIGNATURES_ADCACAL0;
		ADCA.CALH = PRODSIGNATURES_ADCACAL1;
	
		ADCB.REFCTRL = ADC_REFSEL_INT1V_gc;
		ADCB.CTRLB = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; //12bit resolution, and sets it to signed mode.
		ADCB.PRESCALER = ADC_PRESCALER_DIV512_gc;
		ADCB.CALL = PRODSIGNATURES_ADCBCAL0;
		ADCB.CALH = PRODSIGNATURES_ADCBCAL1;
	
		ADCA.CH0.CTRL = ADC_CH_INPUTMODE_DIFF_gc;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCA.CH0.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE3_gc | ADC_CH_MUXPOS_PIN5_gc;	// use VREF_IN for the negative input (0.54 V)
		ADCA.CH1.CTRL = ADC_CH_INPUTMODE_DIFF_gc;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCA.CH1.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE3_gc | ADC_CH_MUXPOS_PIN6_gc;	// use VREF_IN for the negative input (0.54 V)
		ADCA.CH2.CTRL = ADC_CH_INPUTMODE_DIFF_gc;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCA.CH2.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE3_gc | ADC_CH_MUXPOS_PIN7_gc;	// use VREF_IN for the negative input (0.54 V)
	
		ADCB.CH0.CTRL = ADC_CH_INPUTMODE_DIFF_gc;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCB.CH0.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE3_gc | ADC_CH_MUXPOS_PIN4_gc;	// use VREF_IN for the negative input (0.54 V)
		ADCB.CH1.CTRL = ADC_CH_INPUTMODE_DIFF_gc;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCB.CH1.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE3_gc | ADC_CH_MUXPOS_PIN2_gc;	// use VREF_IN for the negative input (0.54 V)
		ADCB.CH2.CTRL = ADC_CH_INPUTMODE_DIFF_gc;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCB.CH2.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE3_gc | ADC_CH_MUXPOS_PIN3_gc;	// use VREF_IN for the negative input (0.54 V)

		ADCA.CTRLA = ADC_ENABLE_bm;
		ADCB.CTRLA = ADC_ENABLE_bm;
	#else
		/* SET INPUT PINS AS INPUTS */
		IR_SENSOR_PORT.DIRCLR = ALL_IR_SENSOR_PINS_bm;

		ADCB.REFCTRL = ADC_REFSEL_INT1V_gc;
		ADCB.CTRLB = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; //12bit resolution, and sets it to signed mode.
		ADCB.PRESCALER = ADC_PRESCALER_DIV512_gc;
		ADCB.CH0.CTRL = ADC_CH_INPUTMODE_DIFF_gc;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCB.CH0.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE3_gc;	// use VREF_IN for the negative input (0.54 V)
		ADCB.CALL = PRODSIGNATURES_ADCBCAL0;
		ADCB.CALH = PRODSIGNATURES_ADCBCAL1;
		ADCB.CTRLA = ADC_ENABLE_bm;
	#endif

	delay_us(5);
	
	for(uint8_t dir=0; dir<6; dir++) ir_sense_baseline[dir] = 0;
	
	delay_ms(5);
	get_ir_sensor(0);
	uint8_t min_val, val;
	for(uint8_t dir=0; dir<6; dir++)
	{
		min_val=255;
		for(uint8_t meas_count=0; meas_count<5; meas_count++)
		{
			val = get_ir_sensor(dir);
			if(val<min_val) min_val = val;
		}
		ir_sense_baseline[dir] = min_val;
	}
	//printf("\r\n");
}

uint8_t get_ir_sensor(uint8_t sensor_num)
{	
	int16_t meas[IR_MEAS_COUNT];
	
	#ifdef AUDIO_DROPLET
	
		for(uint8_t meas_count=0; meas_count<IR_MEAS_COUNT; meas_count++)
		{
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
				ir_sense_channels[sensor_num]->CTRL |= ADC_CH_START_bm;
				while (ir_sense_channels[sensor_num]->INTFLAGS==0){};		// wait for measurement to complete
			}
			meas[meas_count] = ((((int16_t)(ir_sense_channels[sensor_num]->RESH))<<8)|((int16_t)(ir_sense_channels[sensor_num]->RESL)))>>3;	
			ir_sense_channels[sensor_num]->INTFLAGS=1; // clear the complete flag		
		}
	
	#else
	
		ADCB.CH0.MUXCTRL &= MUX_SENSOR_CLR; //clear previous sensor selection
		ADCB.CH0.MUXCTRL |= mux_sensor_selectors[sensor_num];
	
		for(uint8_t meas_count=0; meas_count<IR_MEAS_COUNT; meas_count++)
		{
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
				ADCB.CH0.CTRL |= ADC_CH_START_bm;
				while (ADCB.CH0.INTFLAGS==0){};		// wait for measurement to complete
			}
			meas[meas_count] = ((((int16_t)ADCB.CH0.RESH)<<8)|((int16_t)ADCB.CH0.RESL))>>2;
			ADCB.CH0.INTFLAGS=1; // clear the complete flag
		}
	#endif
	
	int16_t median = meas_find_median(&(meas[2]), IR_MEAS_COUNT-2);
	//printf("\t");
	//for(uint8_t i=0;i<IR_MEAS_COUNT;i++) printf("%u: %3d\t",i, meas[i]);
	//printf("\r\n");	
	if(median<ir_sense_baseline[sensor_num])	return 0;
	else										return (median-ir_sense_baseline[sensor_num]);
}

uint8_t check_collisions(){
	int16_t baseline_meas[6];
	uint8_t channelCtrlBVals[6];
	volatile int16_t measured_vals[6];
	uint8_t dirs=0;
	//wait_for_ir(ALL_DIRS);
	for(uint8_t i=0;i<6;i++) ir_rxtx[i].status = IR_STATUS_BUSY_bm;	
	uint16_t curr_power = get_all_ir_powers();
	set_all_ir_powers(256);
	for(uint8_t i=0;i<6;i++)
	{
		busy_delay_us(50);
		//get_ir_sensor(i);
		baseline_meas[i] = get_ir_sensor(i);
	}
	TCF2.CTRLB &= ~ALL_EMITTERS_CARWAV_bm;	//disable carrier wave output
	PORTF.OUTSET = ALL_EMITTERS_CARWAV_bm;
	for(uint8_t i=0;i<6;i++)
	{
		channelCtrlBVals[i] = channel[i]->CTRLB;
		channel[i]->CTRLB=0;
	}	
	PORTC.DIRSET = (PIN3_bm | PIN7_bm);
	PORTD.DIRSET =  PIN3_bm;
	PORTE.DIRSET = (PIN3_bm | PIN7_bm);
	PORTF.DIRSET =  PIN3_bm;
	PORTC.OUTCLR = (PIN3_bm | PIN7_bm);
	PORTD.OUTCLR = PIN3_bm;
	PORTE.OUTCLR = (PIN3_bm | PIN7_bm);
	PORTF.OUTCLR = PIN3_bm;

	busy_delay_us(250);
	ADCA.CTRLA |= ADC_FLUSH_bm;
	ADCB.CTRLA |= ADC_FLUSH_bm;
	//delay_ms(250);
	for(uint8_t i=0;i<6;i++)
	{
		busy_delay_us(250);
		//get_ir_sensor(i);
		measured_vals[i] = get_ir_sensor(i);
		//printf("%1hu:%d\t", i, measured_vals[i]);		
		//int16_t temp = measured_vals[i]-baseline_meas[i];
		//printf("\t%3d", temp);
		if((measured_vals[i]-baseline_meas[i])>16){
			dirs = dirs|(1<<i);
		}
	}
	//printf("\r\n");
	PORTF.OUTCLR = ALL_EMITTERS_CARWAV_bm;
	for(uint8_t i=0;i<6;i++) channel[i]->CTRLB = channelCtrlBVals[i];
	TCF2.CTRLB |= ALL_EMITTERS_CARWAV_bm; //reenable carrier wave output
	set_all_ir_powers(curr_power);
	for(uint8_t i=0;i<6;i++) ir_rxtx[i].status = 0;		
	return dirs;
}	

// Finds the median of 3 numbers by finding the max, finding the min, and returning the other value
// WARNING! This function modifies the array!
int16_t meas_find_median(int16_t* meas, uint8_t arr_len)
{
	if(arr_len==1) return meas[0];
	else if(arr_len==2) return (meas[0]+meas[1])/2;
	
	for(uint8_t i=0; i<arr_len ; i++)
	{
		for(uint8_t j=i+1 ; j<arr_len ; j++)
		{
			if(meas[j] < meas[i])
			{
				int16_t temp = meas[i];
				meas[i] = meas[j];
				meas[j] = temp;
			}
		}
	}
	if(arr_len%2==0) return (meas[arr_len/2-1]+meas[arr_len/2])/2;
	else return meas[arr_len/2];
}
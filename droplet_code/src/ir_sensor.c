#include "ir_sensor.h"

#ifdef AUDIO_DROPLET
	ADC_CH_t* ir_sense_channels[6]  = {&(ADCA.CH0), &(ADCA.CH1), &(ADCA.CH2), &(ADCB.CH0), &(ADCB.CH1), &(ADCB.CH2)};
#else
	const uint8_t mux_sensor_selectors[6] = {MUX_IR_SENSOR_0, MUX_IR_SENSOR_1, MUX_IR_SENSOR_2, MUX_IR_SENSOR_3, MUX_IR_SENSOR_4, MUX_IR_SENSOR_5};
#endif

static int16_t  ir_sense_baseline[6];

// IR sensors use ADCB channel 0, all the time
void irSensorInit(){
	#ifdef AUDIO_DROPLET
		/* SET INPUT PINS AS INPUTS */
		PORTA.DIRCLR = PIN5_bm | PIN6_bm | PIN7_bm;
		PORTB.DIRCLR = PIN4_bm | PIN2_bm | PIN3_bm;
	
		//git 
		//IR SENSORS ACTING WEIRD? TRY COMMENTING OUT THE BELOW.
		//BELOW RECOMMENDED BY note on pg 153 of the manual
		//
		PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTA.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTB.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTB.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTB.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
	
		ADCA.REFCTRL = ADC_REFSEL_AREFA_gc;
		ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm/* | ADC_FREERUN_bm*/;
		ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
		ADCA.CALL = PRODSIGNATURES_ADCACAL0;
		ADCA.CALH = PRODSIGNATURES_ADCACAL1;
	
		ADCB.REFCTRL = ADC_REFSEL_AREFA_gc;
		ADCB.CTRLB = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm/* | ADC_FREERUN_bm*/; //12bit resolution, and sets it to signed mode.	
		ADCB.PRESCALER = ADC_PRESCALER_DIV512_gc;
		ADCB.CALL = PRODSIGNATURES_ADCBCAL0;
		ADCB.CALH = PRODSIGNATURES_ADCBCAL1;
	
		ADCA.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN2_bm;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCA.CH0.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE4_gc | ADC_CH_MUXPOS_PIN5_gc;	// use VREF_IN for the negative input (0.54 V)
		ADCA.CH1.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN2_bm;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCA.CH1.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE4_gc | ADC_CH_MUXPOS_PIN6_gc;	// use VREF_IN for the negative input (0.54 V)
		ADCA.CH2.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN2_bm;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCA.CH2.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE4_gc | ADC_CH_MUXPOS_PIN7_gc;	// use VREF_IN for the negative input (0.54 V)

		ADCB.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN2_bm;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCB.CH0.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE4_gc | ADC_CH_MUXPOS_PIN4_gc;	// use VREF_IN for the negative input (0.54 V)
		ADCB.CH1.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN2_bm;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCB.CH1.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE4_gc | ADC_CH_MUXPOS_PIN2_gc;	// use VREF_IN for the negative input (0.54 V)
		ADCB.CH2.CTRL =ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN2_bm;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCB.CH2.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE4_gc | ADC_CH_MUXPOS_PIN3_gc;	// use VREF_IN for the negative input (0.54 V)


		ADCA.CTRLA = ADC_ENABLE_bm;
		ADCB.CTRLA = ADC_ENABLE_bm;
		
		//ADCA.EVCTRL = ADC_EVSEL_1234_gc | ADC_EVACT_CH012_gc | ADC_SWEEP_012_gc;
		//ADCB.EVCTRL = ADC_EVSEL_567_gc | ADC_EVACT_CH012_gc | ADC_SWEEP_012_gc;
	#else
		/* SET INPUT PINS AS INPUTS */
		IR_SENSOR_PORT.DIRCLR = ALL_IR_SENSOR_PINS_bm;
		
		PORTB.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTB.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTB.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTB.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTB.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
		PORTB.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;

		ADCB.REFCTRL = ADC_REFSEL_AREFA_gc;
		ADCB.CTRLB = ADC_RESOLUTION_12BIT_gc | ADC_CONMODE_bm; //12bit resolution, and sets it to signed mode.
		ADCB.PRESCALER = ADC_PRESCALER_DIV512_gc;
		ADCB.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN2_bm;	// differential input. requires signed mode (see sec. 28.6 in manual)
		ADCB.CH0.MUXCTRL = ADC_CH_MUXNEG_INTGND_MODE4_gc;	// use VREF_IN for the negative input (0.54 V)
		ADCB.CALL = PRODSIGNATURES_ADCBCAL0;
		ADCB.CALH = PRODSIGNATURES_ADCBCAL1;
		ADCB.CTRLA = ADC_ENABLE_bm;
	#endif
	
	for(uint8_t dir=0;dir<6;dir++){
		ir_sense_baseline[dir]=0;
	}
	scheduleTask(1000,initIrBaselines,NULL);
	schedulePeriodicTask(5407, updateIrBaselines, NULL);
}

void initIrBaselines(){
	getIrSensors(ir_sense_baseline, 13);
}

void updateIrBaselines(){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(irIsBusy(ALL_DIRS)){
			return;
		}
		hpIrBlock_bm=0x3F;
	}
	int16_t prevBaselines[6];
	for(uint8_t dir=0; dir<6; dir++){
		prevBaselines[dir] = ir_sense_baseline[dir]; //zeroing the baseline array.
		ir_sense_baseline[dir] = 0;
	}
	getIrSensors(ir_sense_baseline, 13);
	for(uint8_t dir=0;dir<6;dir++){
		ir_sense_baseline[dir] = (ir_sense_baseline[dir]+prevBaselines[dir])/2;
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		hpIrBlock_bm = 0;
	}
}

void getIrSensors(int16_t* output_arr, uint8_t meas_per_ch){			
	int16_t meas[6][meas_per_ch];	
	#ifdef AUDIO_DROPLET
		for(uint8_t meas_count=0;meas_count<meas_per_ch;meas_count++){
			for(uint8_t dir=0;dir<6;dir++){	
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
					ir_sense_channels[dir]->CTRL |= ADC_CH_START_bm;
					while(ir_sense_channels[dir]->INTFLAGS==0);
					meas[dir][meas_count] = (ir_sense_channels[dir]->RES);
					ir_sense_channels[dir]->INTFLAGS=1;
				}
			}
		}
	#else
		for(uint8_t dir=0;dir<6;dir++){
			ADCB.CH0.MUXCTRL &= MUX_SENSOR_CLR; //clear previous sensor selection
			ADCB.CH0.MUXCTRL |= mux_sensor_selectors[dir];			
			for(uint8_t meas_count=0; meas_count<meas_per_ch; meas_count++){
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE){		
					ADCB.CH0.CTRL |= ADC_CH_START_bm;
					while (ADCB.CH0.INTFLAGS==0){};		// wait for measurement to complete
					meas[dir][meas_count] = ADCB.CH0RES;
					ADCB.CH0.INTFLAGS=1; // clear the complete flag					
				}
			}			
		}
	#endif	
	
	
	for(uint8_t dir=0;dir<6;dir++){
		if(meas_per_ch>2){
			int16_t median = measFindMedian(&(meas[dir][2]),meas_per_ch-2);
			//printf("%d ",median);
			output_arr[dir] = median-ir_sense_baseline[dir];
		}			
		else if(meas_per_ch==2)
			output_arr[dir] = measFindMedian(&(meas[dir][1]),meas_per_ch-1)-ir_sense_baseline[dir];
		else
			output_arr[dir] = meas[dir][0];
	}
	//for(uint8_t i=0;i<6;i++) printf("%d ", output_arr[i]);
	//printf("\r\n");	
}

uint8_t checkCollisions(){
	int16_t meas[6];
	uint8_t dirs = 0;
	checkCollisionValues(meas);
	for(uint8_t i=0;i<6;i++){
		dirs |=  (((meas[i]+ir_sense_baseline[i])>=IR_SENSE_MAX)<<i);
	}
	return dirs;
}

void checkCollisionValues(int16_t meas[6]){
	int16_t baseline_meas[6];
	int16_t measured_vals[6];
	//uint8_t dirs=0;
	if(irIsBusy(ALL_DIRS)){
		printf_P(PSTR("IR Hardware busy. Can't check collisions.\r\n"));
		return;
	}
	for(uint8_t i=0;i<6;i++) ir_rxtx[i].status = IR_STATUS_BUSY_bm;	
	uint16_t curr_power = getAllirPowers();
	setAllirPowers(256);
	getIrSensors(baseline_meas, 5);
	//printf("Coll    base: ");
	//for(uint8_t i=0;i<6;i++) printf("%4d ", baseline_meas[i]);
	//printf("\r\n");
	for(uint8_t i=0;i<6;i++) irLedOn(i);
	delay_us(250);	
	getIrSensors(measured_vals, 5);
	//printf("Coll results: ");
	//for(uint8_t i=0;i<6;i++) printf("%4d ", measured_vals[i]);
	//printf("\r\n");
	for(uint8_t i=0;i<6;i++) irLedOff(i);
	for(uint8_t i=0;i<6;i++){
		meas[i] = (measured_vals[i]-baseline_meas[i]);
	}
	setAllirPowers(curr_power);
	for(uint8_t i=0;i<6;i++) ir_rxtx[i].status = 0;		
}	
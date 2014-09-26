#include "IR_sensor.h"

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
void IR_sensor_init()
{
	/* SET INPUT PINS AS INPUTS */
	IR_SENSOR_PORT.DIRCLR = ALL_IR_SENSOR_PINS_bm;
	

	ADCB.REFCTRL = /*ADC_BANDGAP_bm |*/ ADC_REFSEL_AREFB_gc;
	ADCB.CTRLB = ADC_RESOLUTION_8BIT_gc | ADC_CONMODE_bm;
	ADCB.PRESCALER = ADC_PRESCALER_DIV512_gc;
	ADCB.CH0.CTRL = ADC_CH_INPUTMODE_DIFF_gc;	// differential input. requires signed mode (see sec. 28.6 in manual)
	ADCB.CH0.MUXCTRL = ADC_CH_MUXNEG_PIN0_gc;	// use VREF_IN for the negative input (0.54 V)
	ADCB.CTRLA = ADC_ENABLE_bm;
	//ADCB.CALL = PRODSIGNATURES_ADCBCAL0;
	//ADCB.CALH = PRODSIGNATURES_ADCBCAL1;

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
	//printf("Offsets: [0: %hhd, 1: %hhd, 2: %hhd, 3: %hhd, 4: %hhd, 5: %hhd\r\n",ADC_offset[0],ADC_offset[1],ADC_offset[2],ADC_offset[3],ADC_offset[4],ADC_offset[5]);
		
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

uint8_t get_IR_sensor(uint8_t sensor_num)
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

//void check_collisions(){
	//uint8_t meas[6];
	//set_all_ir_powers(256);
	//for(uint8_t i=0;i<6;i++) printf("%3hhd ",ir_bounce_meas(i));
	//printf("\r\n");
//}

//void ir_blast(uint16_t duration){
	//for(uint8_t i=0;i<6;i++) channelCtrlBVals[i] = channel[i]->CTRLB;	
	//
	//TCF2.CTRLB &= ~ALL_EMITTERS_CARWAV_bm; //disable carrier wave output
	//PORTF.DIRSET = ALL_EMITTERS_CARWAV_bm;			 //enable user output on these pins.
	//PORTF.OUT |= ALL_EMITTERS_CARWAV_bm;			// high signal on these pins
//
	//for(uint8_t i=0;i<6;i++) channel[i]->CTRLB=0;
	//
	//PORTC.DIRSET = (PIN3_bm | PIN7_bm);
	//PORTD.DIRSET =  PIN3_bm;
	//PORTE.DIRSET = (PIN3_bm | PIN7_bm);
	//PORTF.DIRSET =  PIN3_bm;
	//
	//PORTC.OUT &= ~(PIN3_bm | PIN7_bm);
	//PORTD.OUT &= ~PIN3_bm;
	//PORTE.OUT &= ~(PIN3_bm | PIN7_bm);
	//PORTF.OUT &= ~PIN3_bm;
	//
	//ir_blast_on=1;
	//schedule_task(duration, end_ir_blast, 0);
//}
//
//void end_ir_blast(){
	//if(!ir_blast_on) return;
	//PORTC.OUT |= (PIN3_bm | PIN7_bm);
	//PORTD.OUT |=  PIN3_bm;
	//PORTE.OUT |= (PIN3_bm | PIN7_bm);
	//PORTF.OUT |=  PIN3_bm;
	//for(uint8_t i=0;i<6;i++) channel[i]->CTRLB = channelCtrlBVals[i];	
	//TCF2.CTRLB |= ALL_EMITTERS_CARWAV_bm; //disable carrier wave output
	//PORTF.OUT &= ~ALL_EMITTERS_CARWAV_bm; // high signal on these pins
	//ir_blast_on=0;
//}

	
void check_collisions(){
	uint8_t baseline_meas[6];
	uint8_t channelCtrlBVals[6];
	uint8_t measured_vals[6];
	for(uint8_t i=0;i<6;i++)
	{

		channelCtrlBVals[i] = channel[i]->CTRLB;
		channel[i]->CTRLB=0;
	}	
	for(uint8_t i=0;i<6;i++)
	{	
		busy_delay_us(50);
		baseline_meas[i] = get_IR_sensor(i);
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
	delay_ms(10000);
	for(uint8_t i=0;i<6;i++){
		busy_delay_us(250);		
		measured_vals[i]=get_IR_sensor(i);
		//busy_delay_us(50);
	}		

	PORTC.OUTTGL = (PIN3_bm | PIN7_bm);
	PORTD.OUTTGL =  PIN3_bm;
	PORTE.OUTTGL = (PIN3_bm | PIN7_bm);
	PORTF.OUTTGL =  PIN3_bm;
	for(uint8_t i=0;i<6;i++) channel[i]->CTRLB = channelCtrlBVals[i];
	TCF2.CTRLB |= ALL_EMITTERS_CARWAV_bm; //reenable carrier wave output

	for(uint8_t i=0;i<6;i++){
		printf("%3hhu ", (uint8_t)(measured_vals[i]-baseline_meas[i]));
	}		
	printf("\r\n");
}	
	
int8_t ir_bounce_meas(uint8_t dir){
	uint8_t carrier_wave_bm;
	uint8_t TX_pin_bm;

	PORT_t * UART;
	USART_t* USART;
	int16_t baseline_meas = get_IR_sensor(dir);

	switch(dir)
	{
		case 0:	// WORKS!
			carrier_wave_bm = PIN0_bm; TX_pin_bm = PIN3_bm; UART = &PORTC; break;
			case 1:	// WORKS!
			carrier_wave_bm = PIN1_bm; TX_pin_bm = PIN7_bm; UART = &PORTC; break;
			case 2:	// WORKS!
			carrier_wave_bm = PIN4_bm; TX_pin_bm = PIN3_bm; UART = &PORTD; break;
			case 3:	// WORKS!
			carrier_wave_bm = PIN5_bm; TX_pin_bm = PIN3_bm; UART = &PORTE; break;
			case 4:	// WORKS!
			carrier_wave_bm = PIN7_bm; TX_pin_bm = PIN7_bm; UART = &PORTE; break;
			case 5:	// WORKS!
			carrier_wave_bm = PIN6_bm; TX_pin_bm = PIN3_bm; UART = &PORTF; break;
	}

	uint8_t USART_CTRLB = channel[dir]->CTRLB;		// record the current state of the USART

	TCF2.CTRLB &= ~carrier_wave_bm;			// disable carrier wave output
	PORTF.DIRSET = carrier_wave_bm;			// enable user output on this pin
	PORTF.OUT |= carrier_wave_bm;			// high signal on this pin

	channel[dir]->CTRLB = 0;				// disable USART
	UART->DIRSET = TX_pin_bm;			// enable user output on this pin
	UART->OUT &= ~TX_pin_bm;			// low signal on TX pin		(IR LED is ON when this pin is LOW, these pins were inverted in software during initialization)

	busy_delay_us(250);
	//delay_ms(30);
	// IR LIGHT IS ON NOW
	int16_t measured_val = get_IR_sensor(dir);

	UART->OUT |= TX_pin_bm;			// high signal on TX pin (turns IR blast OFF)

	channel[dir]->CTRLB = USART_CTRLB;	// re-enable USART (restore settings as it was before)
	PORTF.OUT &= ~carrier_wave_bm;			// low signal on the carrier wave pin, don't really know why we do this? probably not necessary
	TCF2.CTRLB |= carrier_wave_bm;			// re-enable carrier wave output

	return (int8_t)(measured_val-baseline_meas);
	
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
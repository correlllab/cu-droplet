#include "rgb_sensor.h"
#define RGB_SENSE_ADDR 0x29

static const char RGB_SENSE_POWERON_FAILURE[] PROGMEM = "RGB sense power-on failed %u.\r\n";
#ifndef AUDIO_DROPLET
static int16_t r_baseline, g_baseline, b_baseline;
#endif

//typedef enum{
	//R, G, B
//} Colors;

typedef union{
	uint32_t i;
	float f;
} u;

void rgbSensorInit()
{
	#ifdef AUDIO_DROPLET
		uint8_t power_on_sequence[8] = {0x80, 0x01,  // Write 0x01 to ENABLE register, activating the device's oscillator.
										0x8F, 0x01,  // Write 0x01 to CONTROL register, setting the gain to x4.
										0x81, 0xD5,	 // Write 0xD5 to ATIME register, setting the integration time to 2.4ms*(256-ATIME)
										0x80, 0x03};  // Write 0x03 to ENABLE register, activating the ADC (and leaving the oscillator on);
		char callerDescr[9] = "RGB Init\0";
		uint8_t result = twiWriteWrapper(RGB_SENSE_ADDR, &(power_on_sequence[0]), 2, callerDescr);
		if(!result)	printf_P(RGB_SENSE_POWERON_FAILURE,1);
		result = twiWriteWrapper(RGB_SENSE_ADDR, &(power_on_sequence[2]), 2, callerDescr);
		if(!result)	printf_P(RGB_SENSE_POWERON_FAILURE,2);
		result = twiWriteWrapper(RGB_SENSE_ADDR, &(power_on_sequence[4]), 2, callerDescr);
		if(!result)	printf_P(RGB_SENSE_POWERON_FAILURE,3);
		result = twiWriteWrapper(RGB_SENSE_ADDR, &(power_on_sequence[6]), 2, callerDescr);
		if(!result)	printf_P(RGB_SENSE_POWERON_FAILURE,4);
	#else		
		RGB_SENSOR_PORT.DIRCLR = RGB_SENSOR_R_PIN_bm | RGB_SENSOR_G_PIN_bm | RGB_SENSOR_B_PIN_bm;

		ADCA.REFCTRL = ADC_REFSEL_AREFA_gc;
		ADCA.CTRLB = ADC_RESOLUTION_LEFT12BIT_gc | ADC_CONMODE_bm;
		ADCA.PRESCALER = ADC_PRESCALER_DIV256_gc;
		/* When differential input is used, signed mode must be used. (sec. 28.6 of Manual) */

		ADCA.CH0.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_1X_gc;	//Probably should turn the gain back up to 4X when we put the shells on.
		ADCA.CH1.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_1X_gc;	//Probably should turn the gain back up to 4X when we put the shells on.
		ADCA.CH2.CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | ADC_CH_GAIN_2X_gc;	//Probably should turn the gain back up to 4X when we put the shells on.
	
		ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN5_gc | ADC_CH_MUXNEG_INTGND_MODE4_gc;	// Red sensor on ADC A channel 0
		ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN6_gc | ADC_CH_MUXNEG_INTGND_MODE4_gc;	// Green sensor on ADC A channel 1
		ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN7_gc | ADC_CH_MUXNEG_INTGND_MODE4_gc;	// Blue sensor on ADC A channel 2
	
		ADCA.CALL = PRODSIGNATURES_ADCACAL0;
		ADCA.CALH = PRODSIGNATURES_ADCACAL1;

		ADCA.CTRLA = ADC_ENABLE_bm;
	
		//read_color_settings();
	
		delay_us(50);
		const int8_t num_samples = 3;
		getRedSensor(); getBlueSensor(); getGreenSensor();
		delayMS(10);
		int16_t r_avg=0, g_avg=0, b_avg=0;
		for(uint8_t i=0; i<num_samples; i++)
		{
			r_avg+=getRedSensor();
			g_avg+=getGreenSensor();
			b_avg+=getBlueSensor();
			delayMS(10);
			//printf("\r\n");
		}
		r_baseline= r_avg/num_samples;
		g_baseline= g_avg/num_samples;
		b_baseline= b_avg/num_samples;
		//printf("Baselines:\r\n%3d  %3d  %3d\r\n", r_baseline, g_baseline, b_baseline);
		r_baseline = 0;
		g_baseline = 0;
		b_baseline = 0;	
	#endif		
}

#ifndef AUDIO_DROPLET

int16_t getRedSensor(){
	int16_t meas[RGB_MEAS_COUNT];
	int16_t red_val;
	//printf("R: ");
	for(uint8_t meas_count=0; meas_count<RGB_MEAS_COUNT; meas_count++){
		ADCA.CH0.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH0.INTFLAGS==0){};		// wait for measurement to complete
		meas[meas_count] = ((((int16_t)(ADCA.CH0.RESH))<<8)|((int16_t)ADCA.CH0.RESL))>>4;	
		ADCA.CH0.INTFLAGS=1; // clear the complete flag				
		//printf("%6d ", meas[meas_count]);
	}
	//printf("\r\n");
	red_val=measFindMedian(&meas[2], RGB_MEAS_COUNT-2);
	return red_val;
}

int16_t getGreenSensor(){
	int16_t meas[RGB_MEAS_COUNT];
	int16_t green_val;		
	//printf("G: ");
	for(uint8_t meas_count=0; meas_count<RGB_MEAS_COUNT; meas_count++)	{
		ADCA.CH1.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH1.INTFLAGS==0){};		// wait for measurement to complete
		meas[meas_count] = ((((int16_t)(ADCA.CH1.RESH))<<8)|((int16_t)ADCA.CH1.RESL))>>4;		
		ADCA.CH1.INTFLAGS=1; // clear the complete flag	
		//printf("%6d ", meas[meas_count]);
	}
	//printf("\r\n");
	green_val=measFindMedian(&meas[2], RGB_MEAS_COUNT-2);

	return green_val;
}

int16_t getBlueSensor(){
	int16_t meas[RGB_MEAS_COUNT];
	int16_t blue_val;
	//printf("B: ");
	for(uint8_t meas_count=0; meas_count<RGB_MEAS_COUNT; meas_count++)	{
		ADCA.CH2.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH2.INTFLAGS==0){};		// wait for measurement to complete
		meas[meas_count] = ((((int16_t)(ADCA.CH2.RESH))<<8)|((int16_t)ADCA.CH2.RESL))>>4;
		ADCA.CH2.INTFLAGS=1; // clear the complete flag		
		//printf("%6d ", meas[meas_count]);
	}		
	//printf("\r\n");
	blue_val=measFindMedian(&meas[2], RGB_MEAS_COUNT-2);
		
	return blue_val;
}

#endif


void getRGB(int16_t *r, int16_t *g, int16_t *b)
{
	#ifdef AUDIO_DROPLET
		
		uint8_t write_sequence = 0xB4;
		char callerDescr[8] = "Get RGB\0";
		uint8_t result = twiWriteReadWrapper(RGB_SENSE_ADDR, &write_sequence, 1, 8, callerDescr);
		int16_t* temp_values = (int16_t*)(twi->readData);
		if(result)
		{
			//*c=temp_values[0];
			*r=temp_values[1];
			*g=temp_values[2];
			*b=temp_values[3];
			//printf("% 5d % 5d % 5d % 5d\r\n", temp_values[0], *r, *g, *b);
		}
		else printf_P(PSTR("Read failed.\r\n"));
	#else
		int16_t rTemp,gTemp,bTemp;
	
		rTemp = getRedSensor();
		gTemp = getGreenSensor();
		bTemp = getBlueSensor();		
		rTemp = rTemp - r_baseline;
		gTemp = gTemp - g_baseline;
		bTemp = bTemp - b_baseline;
		//if(rTemp<0)	rTemp=0;
		//if(gTemp<0)	gTemp=0;
		//if(bTemp<0)	bTemp=0;
		if(r!=NULL) *r = rTemp;
		if(g!=NULL) *g = gTemp;
		if(b!=NULL) *b = bTemp;
	#endif
}

// Finds the median of arr_len numbers by finding the max, finding the min, and returning the other value
// WARNING! This function modifies the array!
int16_t measFindMedian(int16_t* meas, uint8_t arr_len){
	if(arr_len==1) return meas[0];
	else if(arr_len==2) return (meas[0]+meas[1])/2;
	
	for(uint8_t i=0; i<arr_len ; i++){
		for(uint8_t j=i+1 ; j<arr_len ; j++){
			if(meas[j] < meas[i]){
				int16_t temp = meas[i];
				meas[i] = meas[j];
				meas[j] = temp;
			}
		}
	}
	if(arr_len%2==0) return (meas[arr_len/2-1]+meas[arr_len/2])/2;
	else return meas[arr_len/2];
}
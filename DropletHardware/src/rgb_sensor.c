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

void get_rgb_sensors(int8_t* r, int8_t* g, int8_t* b)
{
	uint8_t led_r = get_red_led();
	uint8_t led_g = get_green_led();
	uint8_t led_b = get_blue_led();
	
	set_rgb(0,0,0);
	delay_ms(LED_OFF_DELAY_MS);
	
	if(r!=NULL) *r = get_red_sensor();
	if(g!=NULL) *g = get_green_sensor();
	if(b!=NULL) *b = get_blue_sensor();
		
	set_rgb(led_r, led_g, led_b);
}

int8_t get_red_sensor()
{
	uint8_t meas[RGB_MEAS_COUNT];
	
	for(uint8_t meas_count=0; meas_count<RGB_MEAS_COUNT; meas_count++)
	{
		ADCA.CH0.INTFLAGS=1; // clear the complete flag
		ADCA.CH0.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH0.INTFLAGS==0){};		// wait for measurement to complete
		meas[meas_count] = ADCA.CH0.RESL;
	}
	
	return find_median(meas, RGB_MEAS_COUNT);
}

int8_t get_green_sensor()
{
	uint8_t meas[RGB_MEAS_COUNT];
		
	for(uint8_t meas_count=0; meas_count<RGB_MEAS_COUNT; meas_count++)
	{
		ADCA.CH1.INTFLAGS=1; // clear the complete flag
		ADCA.CH1.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH1.INTFLAGS==0){};		// wait for measurement to complete
		meas[meas_count] = ADCA.CH1.RESL;
	}
		
	return find_median(meas, RGB_MEAS_COUNT);
}

int8_t get_blue_sensor()
{
	uint8_t meas[RGB_MEAS_COUNT];
		
	for(uint8_t meas_count=0; meas_count<RGB_MEAS_COUNT; meas_count++)
	{
		ADCA.CH2.INTFLAGS=1; // clear the complete flag
		ADCA.CH2.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH2.INTFLAGS==0){};		// wait for measurement to complete
		meas[meas_count] = ADCA.CH2.RESL;
	}
		
	return find_median(meas, RGB_MEAS_COUNT);
}

//// Finds the median of 3 numbers by finding the max, finding the min, and returning the other value
//int8_t find_median(int8_t* meas)
//{
	//uint8_t mini, maxi, medi;
	//int8_t min = -128, max = 127;
	//for (uint8_t i = 0; i < 3; i++)
	//{
		//if (meas[i] < max)
		//{
			//max = meas[i];
			//maxi = i;
		//}
		//if (meas[i] > min)
		//{
			//min = meas[i];
			//mini = i;
		//}
	//}
	//for (medi = 0; medi < 3; medi++)
	//{
		//if ((medi != maxi) && (medi != mini)) return meas[medi];
	//}
//}
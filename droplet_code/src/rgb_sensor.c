#include "rgb_sensor.h"
#define RGB_SENSE_ADDR 0x29



void rgb_sensor_init()
{
	#ifdef AUDIO_DROPLET
		uint8_t power_on_sequence[8] = {0x80, 0x01,  // Write 0x01 to ENABLE register, activating the device's oscillator.
										0x8F, 0x01,  // Write 0x01 to CONTROL register, setting the gain to x4.
										0x81, 0xD5,	 // Write 0xD5 to ATIME register, setting the integration time to 2.4ms*(256-ATIME)
										0x80, 0x03};  // Write 0x03 to ENABLE register, activating the ADC (and leaving the oscillator on);

		uint8_t result = TWI_MasterWrite(RGB_SENSE_ADDR, &(power_on_sequence[0]), 2);
		if(!result)	printf_P(RGB_SENSE_POWERON_FAILURE,1);
		delay_ms(5);
		result = TWI_MasterWrite(RGB_SENSE_ADDR, &(power_on_sequence[2]), 2);
		if(!result)	printf_P(RGB_SENSE_POWERON_FAILURE,2);
		delay_ms(5);
		result = TWI_MasterWrite(RGB_SENSE_ADDR, &(power_on_sequence[4]), 2);
		if(!result)	printf_P(RGB_SENSE_POWERON_FAILURE,3);
		delay_ms(5);
		result = TWI_MasterWrite(RGB_SENSE_ADDR, &(power_on_sequence[6]), 2);
		if(!result)	printf_P(RGB_SENSE_POWERON_FAILURE,4);
		delay_ms(5);
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
		get_red_sensor(); get_blue_sensor(); get_green_sensor();
		delay_ms(10);
		int16_t r_avg=0, g_avg=0, b_avg=0;
		for(uint8_t i=0; i<num_samples; i++)
		{
			r_avg+=get_red_sensor();
			g_avg+=get_green_sensor();
			b_avg+=get_blue_sensor();
			delay_ms(10);
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

int16_t get_red_sensor(){
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
	red_val=meas_find_median(&meas[2], RGB_MEAS_COUNT-2);
	return red_val;
}

int16_t get_green_sensor(){
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
	green_val=meas_find_median(&meas[2], RGB_MEAS_COUNT-2);

	return green_val;
}

int16_t get_blue_sensor(){
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
	blue_val=meas_find_median(&meas[2], RGB_MEAS_COUNT-2);
		
	return blue_val;
}

#endif

void read_color_settings()
{
	#ifndef AUDIO_DROPLET
		//printf("Reading Color Calib Matrix:\r\n");
		u dat;
		for(uint8_t i=0;i<3;i++)
		{
			for(uint8_t j=0;j<3;j++)
			{
				dat.i =((uint32_t)EEPROM_read_byte(0x60 + 12*i + 4*j + 0))<<24;
				dat.i|=((uint32_t)EEPROM_read_byte(0x60 + 12*i + 4*j + 1))<<16;
				dat.i|=((uint32_t)EEPROM_read_byte(0x60 + 12*i + 4*j + 2))<<8;
				dat.i|=((uint32_t)EEPROM_read_byte(0x60 + 12*i + 4*j + 3))<<0;
				calib_matrix[i][j]=dat.f;
				//printf("\t%f\t",dat.f);	
			}
			//printf("\r\n");		
		}
		//printf("\r\n");
	#else
		printf_P(PSTR("ERROR: Audio droplets don't use color_settings.\r\n"));
	#endif		
}

void get_rgb(int16_t *r, int16_t *g, int16_t *b)
{
	#ifdef AUDIO_DROPLET
		uint8_t write_sequence = 0xB4;
		uint8_t result = TWI_MasterWriteRead(RGB_SENSE_ADDR, &write_sequence, 1, 8);
		uint16_t* temp_values = (uint16_t*)(twi->readData);
		if(result)
		{
			//*c=temp_values[0];
			*r=(int16_t)temp_values[1];
			*g=(int16_t)temp_values[2];
			*b=(int16_t)temp_values[3];
		}
		else printf_P(PSTR("Read failed.\r\n"));
	#else
		int16_t rTemp,gTemp,bTemp;
	
		rTemp = get_red_sensor();
		gTemp = get_green_sensor();
		bTemp = get_blue_sensor();		
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
#include "rgb_sensor.h"

void rgb_sensor_init()
{
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
	
	read_color_settings();
	
	delay_ms(50);
	const int8_t num_samples = 3;
	int16_t r_avg=0, g_avg=0, b_avg=0;
	int16_t rCal, gCal, bCal;
	for(uint8_t i=0; i<num_samples; i++)
	{
		r_avg+=get_red_sensor();
		g_avg+=get_green_sensor();
		b_avg+=get_blue_sensor();
		delay_ms(50);
	}
	r_baseline= r_avg/num_samples;
	g_baseline= g_avg/num_samples;
	b_baseline= b_avg/num_samples;
	printf("Baselines:\r\n%3d  %3d  %3d\r\n", r_baseline, g_baseline, b_baseline);
}

// Still not convinced that we should have the conditional, instead of just telling people
// that if their lights are on they won't get good values, here.
void get_rgb_sensors(uint8_t* r, uint8_t* g, uint8_t* b)
{
	uint8_t led_r = get_red_led();
	uint8_t led_g = get_green_led();
	uint8_t led_b = get_blue_led();
	
	if(led_r || led_g || led_b)
	{
		set_rgb(0,0,0);
		delay_ms(LED_OFF_DELAY_MS);
	} 
	
	//*r = (int8_t)((((rResH&0x08)<<4)|((rResH&0x01)<<6))|((rResL&0xFC)>>2));
	//*g = (int8_t)((((gResH&0x08)<<4)|((gResH&0x01)<<6))|((gResL&0xFC)>>2));	
	//*b = (int8_t)((((bResH&0x08)<<4)|((bResH&0x01)<<6))|((bResL&0xFC)>>2));	

	int16_t rTemp,gTemp,bTemp;

	rTemp = get_red_sensor();
	gTemp = get_green_sensor();		
	bTemp = get_blue_sensor();	
	
	rTemp=rTemp-r_baseline;
	gTemp=gTemp-g_baseline;
	bTemp=bTemp-b_baseline;
	if(rTemp<0) rTemp = 0;
	if(gTemp<0) gTemp = 0;
	if(bTemp<0) bTemp = 0;
	if(r!=NULL) *r=(uint8_t)rTemp;
	if(g!=NULL) *g=(uint8_t)gTemp;	
	if(b!=NULL) *b=(uint8_t)bTemp;	
		
	if(led_r || led_g || led_b) set_rgb(led_r, led_g, led_b);
}

void get_cal_rgb(uint8_t* r, uint8_t* g, uint8_t* b)
{
	uint8_t tmpR, tmpG, tmpB;
	get_rgb_sensors(&tmpR, &tmpG, &tmpB);
	
	float calcR, calcG, calcB;
	
	calcR = calib_matrix[R][R]*(tmpR*1.) + calib_matrix[R][G]*(tmpG*1.) + calib_matrix[R][B]*(tmpB*1.);
	calcG = calib_matrix[G][R]*(tmpR*1.) + calib_matrix[G][G]*(tmpG*1.) + calib_matrix[G][B]*(tmpB*1.);
	calcB = calib_matrix[B][R]*(tmpR*1.) + calib_matrix[B][G]*(tmpG*1.) + calib_matrix[B][B]*(tmpB*1.);
	
	if(calcR>255)		*r = 255;
	else if(calcR<0)	*r = 0;
	else				*r = (uint8_t)calcR;
	
	if(calcG>255)		*g = 255;
	else if(calcG<0)	*g = 0;
	else				*g = (uint8_t)calcG;
	
	if(calcB>255)		*b = 255;
	else if(calcB<0)	*b = 0;
	else				*b = (uint8_t)calcB;
	
	printf("%3hu %3hu %3hu\t->\t%3hu %3hu %3hu\r\n",tmpR,tmpG,tmpB,*r,*g,*b);
}

int16_t get_red_sensor()
{
	int16_t meas[RGB_MEAS_COUNT];
		
	for(uint8_t meas_count=0; meas_count<RGB_MEAS_COUNT; meas_count++)
	{
		ADCA.CH0.INTFLAGS=1; // clear the complete flag
		ADCA.CH0.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH0.INTFLAGS==0){};		// wait for measurement to complete
		meas[meas_count] = ((((int16_t)(ADCA.CH0.RESH))<<8)|((int16_t)ADCA.CH0.RESL))>>4;			
	}
	//printf("%d\t", meas[RGB_MEAS_COUNT-1]);	
	
	return find_median(&meas[2], RGB_MEAS_COUNT-2);
}

int16_t get_green_sensor()
{
	int16_t meas[RGB_MEAS_COUNT];
		
	for(uint8_t meas_count=0; meas_count<RGB_MEAS_COUNT; meas_count++)
	{
		ADCA.CH1.INTFLAGS=1; // clear the complete flag
		ADCA.CH1.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH1.INTFLAGS==0){};		// wait for measurement to complete
		meas[meas_count] = ((((int16_t)(ADCA.CH1.RESH))<<8)|((int16_t)ADCA.CH1.RESL))>>4;		
	}
	//printf("%d\t", meas[RGB_MEAS_COUNT-1]);			
		
	return find_median(&meas[2], RGB_MEAS_COUNT-2);
}

int16_t get_blue_sensor()
{
	int16_t meas[RGB_MEAS_COUNT];
	int16_t blue_val;
	for(uint8_t meas_count=0; meas_count<RGB_MEAS_COUNT; meas_count++)
	{
		ADCA.CH2.INTFLAGS=1; // clear the complete flag
		ADCA.CH2.CTRL |= ADC_CH_START_bm;
		while (ADCA.CH2.INTFLAGS==0){};		// wait for measurement to complete
		meas[meas_count] = ((((int16_t)(ADCA.CH2.RESH))<<8)|((int16_t)ADCA.CH2.RESL))>>4;
	}
	blue_val=find_median(&meas[2], RGB_MEAS_COUNT-2);
	return blue_val;
}

void read_color_settings()
{
	printf("Reading Color Calib Matrix:\r\n");
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
			printf("\t%f\t",dat.f);			
		}
		printf("\r\n");		
	}
	printf("\r\n");
}

void write_color_settings(float cm[3][3])
{
	printf("Writing Color Calib Matrix:\r\n");
	u dat;
	for(uint8_t i=0;i<3;i++)
	{
		for(uint8_t j=0;j<3;j++)
		{
			dat.f=cm[i][j];
			printf("\t%f\t",dat.f);
			EEPROM_write_byte(0x60 + 12*i + 4*j + 0, (uint8_t)((dat.i>>24)&0xFF));
			EEPROM_write_byte(0x60 + 12*i + 4*j + 1, (uint8_t)((dat.i>>16)&0xFF));
			EEPROM_write_byte(0x60 + 12*i + 4*j + 2, (uint8_t)((dat.i>>8)&0xFF));
			EEPROM_write_byte(0x60 + 12*i + 4*j + 3, (uint8_t)((dat.i>>0)&0xFF));
		}
		printf("\r\n");
	}
	printf("\r\n");
}
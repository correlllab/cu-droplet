#include "rgb_led.h"

void rgb_led_init()
{
	// LED Pins as output
	LED_R_PORT.DIRSET = LED_R_PIN_bm;
	LED_G_PORT.DIRSET = LED_G_PIN_bm;
	LED_B_PORT.DIRSET = LED_B_PIN_bm;
	
	//---------RED-----------
	LED_R_TC.PER = 255;							// Set period (~500KHz -> abt. 500 ticks/ms) 
	LED_R_TC.CCB = 0;							// initially fill the CCB register with a value
	LED_R_TC.CTRLA |= TC_CLKSEL_DIV1_gc;		// Set clock and prescaler, 32MHz/64 = 500KHz ** 
	LED_R_TC.CTRLB |= TC_WGMODE_SS_gc;			// enable Single Slope PWM (Waveform Generation Mode)
	LED_R_TC.CTRLB |= TC1_CCBEN_bm;				// enable waveform output on OCnB (setting WGM operation overrides the port output register for this output pin)
	
	//---------GREEN-----------
	// Begin a timer-counter that will reach TOP (overflow) every 20ms (servo signal is 50 Hz)
	LED_G_TC.PER = 255;							// Set period (~500KHz -> abt. 500 ticks/ms) 
	LED_G_TC.CCA = 0;							// initially fill the CCB register with a value
	LED_G_TC.CTRLA |= TC_CLKSEL_DIV1_gc;		// Set clock and prescaler, 32MHz/64 = 500KHz ** 
	LED_G_TC.CTRLB |= TC_WGMODE_SS_gc;			// enable Single Slope PWM (Waveform Generation Mode)
	LED_G_TC.CTRLB |= TC1_CCAEN_bm;				// enable waveform output on OCnA (setting WGM operation overrides the port output register for this output pin)
	
	//---------BLUE-----------
	// Begin a timer-counter that will reach TOP (overflow) every 20ms (servo signal is 50 Hz)
	LED_B_TC.PER = 255;							// Set period (~500KHz -> abt. 500 ticks/ms) 
	LED_B_TC.CCA = 0;							// initially fill the CCB register with a value
	LED_B_TC.CTRLA |= TC_CLKSEL_DIV1_gc;		// Set clock and prescaler, 32MHz/64 = 500KHz ** 
	LED_B_TC.CTRLB |= TC_WGMODE_SS_gc;			// enable Single Slope PWM (Waveform Generation Mode)
	LED_B_TC.CTRLB |= TC1_CCAEN_bm;				// enable waveform output on OCnA (setting WGM operation overrides the port output register for this output pin)
}

void led_off()
{ 
	LED_B_TC.CCABUF = 0;
	LED_G_TC.CCABUF = 0;
	LED_R_TC.CCBBUF = 0; 
}

void set_rgb(uint8_t r, uint8_t g, uint8_t b) 
{ 
	set_red_led(r); 
	set_green_led(g); 
	set_blue_led(b);
}

/*
Hue should be between 0 and 360 (though it gets modulo'd with 360, so should be okay)
Saturation and value can be between 0 and 255, where 255 is brightest/most saturated,
and 0 is dimmest/least saturated.
*/
void set_hsv(uint16_t h, uint8_t s, uint8_t v)
{
	uint8_t r,g,b;
	hsv_to_rgb(h,s,v,&r,&g,&b);
	set_rgb(r,g,b);
}

//Adapted from cs.rit.edu/~ncs/color/t_convert.html
void hsv_to_rgb(uint16_t hue, uint8_t saturation, uint8_t val, uint8_t* red, uint8_t* green, uint8_t* blue)
{
	float h = (hue%360)*1.0;
	float s = saturation/255.0;
	float v = val/255.0;
	float f, p, q, t;
	float r=0, g=0, b=0;
	
	if (abs(s)<0.00001)
	{
		*red = *green = *blue = val;
	}
	
	h = h / 60.0;
	uint8_t i = (uint8_t)h;
	f = h - i;
	
	p = v*(1-s);
	q = v*(1-s*f);
	t = v*(1-s*(1-f));
	
	switch(i)
	{
		case 0:
			r = v;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = v;
			b = p;
			break;
		case 2:
			r = p;
			g = v;
			b = t;
			break;
		case 3:
			r = p;
			g = q;
			b = v;
			break;
		case 4:
			r = t;
			g = p;
			b = v;
			break;
		case 5:
			r = v;
			g = p;
			b = q;
			break;
		default:
			//printf("SHOULD NEVER SEE THIS. i: %hhu",i);
			break;
	}
	*red = (uint8_t)(r*255);
	*blue = (uint8_t)(b*255);
	*green = (uint8_t)(g*255);
}
#include "rgb_led.h"

void rgb_led_init()
{
	// LED Pins as output
	PORTE.DIRSET = LED_R_PIN_bm | LED_G_PIN_bm;
	PORTD.DIRSET = LED_B_PIN_bm;
	
	//-------RED&GREEN--------
	TCE1.PER = 255;							
	TCE1.CCA = 0;							
	TCE1.CCB = 0;							
	TCE1.CTRLA = TC_CLKSEL_DIV64_gc;		
	TCE1.CTRLB = TC_WGMODE_SS_gc | TC1_CCBEN_bm | TC1_CCAEN_bm;
	
	//---------BLUE-----------
	
	TCD1.PER = 255;
	TCD1.CCA = 0;
	TCD1.CTRLA = TC_CLKSEL_DIV64_gc;
	TCD1.CTRLB = TC_WGMODE_SS_gc | TC1_CCAEN_bm;	
}

void led_off()
{ 
	TCE1.CCABUF = 0;
	TCE1.CCBBUF = 0;
	TCD1.CCABUF = 0;	
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
	
	if (abs(s)<0.00001){
		*red = *green = *blue = val;
		return;
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
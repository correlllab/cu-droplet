// RGB_LED.c

#include <avr/io.h>
#include "RGB_LED.h"

void RGB_LED_init()
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


uint8_t get_red_led() { return LED_R_TC.CCB; }
void set_red_led(uint8_t saturation) { /*if (!(pc_command & COMMAND_DARK_bm))*/ LED_R_TC.CCBBUF = saturation; }
void force_set_red_led(uint8_t saturation) { LED_R_TC.CCBBUF = saturation; }

// Get and set intensity for green LED
uint8_t get_green_led() { return LED_G_TC.CCA; }
void set_green_led(uint8_t saturation) { /*if (!(pc_command & COMMAND_DARK_bm))*/ LED_G_TC.CCABUF = saturation; }
void force_set_green_led(uint8_t saturation) { LED_G_TC.CCABUF = saturation; }

// Get and set intensity for blue LED
uint8_t get_blue_led() { return LED_B_TC.CCA; }
void set_blue_led(uint8_t saturation) {/* if (!(pc_command & COMMAND_DARK_bm))*/ LED_B_TC.CCABUF = saturation; }
void force_set_blue_led(uint8_t saturation) { LED_B_TC.CCABUF = saturation; }

void led_off() { LED_B_TC.CCABUF = LED_G_TC.CCABUF = LED_R_TC.CCBBUF = 0; }
void set_rgb(uint8_t r, uint8_t g, uint8_t b) { set_red_led(r); set_green_led(g); set_blue_led(b); }


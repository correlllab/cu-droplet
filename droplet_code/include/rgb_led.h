#pragma once

#include "avr/io.h" //includes stdlib
#include <stdlib.h>

#define LED_R_PIN_bm			PIN5_bm
#define LED_G_PIN_bm			PIN4_bm
#define LED_B_PIN_bm			PIN4_bm

void rgb_led_init(); // Set RGB LED pins as output & initialize timers for PWM

// Get and set intensity for red LED
inline uint8_t get_red_led() { return TCE1.CCBBUF; }
inline void set_red_led(uint8_t saturation) { TCE1.CCBBUF = saturation; }

// Get and set intensity for green LED
inline uint8_t get_green_led() { return TCE1.CCABUF; }
inline void set_green_led(uint8_t saturation) { TCE1.CCABUF = saturation; }
	
// Get and set intensity for blue LED
inline uint8_t get_blue_led() { return TCD1.CCABUF; }
inline void set_blue_led(uint8_t saturation) { TCD1.CCABUF = saturation;	}


void led_off();
void set_rgb(uint8_t r, uint8_t g, uint8_t b);
void set_hsv(uint16_t h, uint8_t s, uint8_t v);
void hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b);
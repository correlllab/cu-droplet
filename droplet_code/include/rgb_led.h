#pragma once
#include "droplet_base.h"
#include <math.h>

#define LED_R_PIN_bm			PIN5_bm
#define LED_G_PIN_bm			PIN4_bm
#define LED_B_PIN_bm			PIN4_bm

void rgb_led_init(void); // Set RGB LED pins as output & initialize timers for PWM

// Get and set intensity for red LED
inline uint8_t get_red_led(void){ return TCE1.CCBBUF; }
inline void set_red_led(uint8_t saturation) { TCE1.CCBBUF = saturation; }

// Get and set intensity for green LED
inline uint8_t get_green_led(void){ return TCE1.CCABUF; }
inline void set_green_led(uint8_t saturation) { TCE1.CCABUF = saturation; }
	
// Get and set intensity for blue LED
inline uint8_t get_blue_led(void){ return TCD1.CCABUF; }
inline void set_blue_led(uint8_t saturation) { TCD1.CCABUF = saturation;	}


void led_off(void);
void set_rgb(uint8_t r, uint8_t g, uint8_t b);
void set_hsv(uint16_t h, uint8_t s, uint8_t v);
void hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b);

inline void warning_light_sequence(void){
	set_rgb(0,100,100); delay_ms(300); set_rgb(100,0,100); delay_ms(300); set_rgb(100,100,0); delay_ms(300); led_off();
	set_rgb(0,100,100); delay_ms(300); set_rgb(100,0,100); delay_ms(300); set_rgb(100,100,0); delay_ms(300); led_off();
	set_rgb(0,100,100); delay_ms(300); set_rgb(100,0,100); delay_ms(300); set_rgb(100,100,0); delay_ms(300); led_off();
}
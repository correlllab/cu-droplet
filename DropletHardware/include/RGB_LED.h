#ifndef __RGB_LED_H__
#define __RGB_LED_H__

#include <avr/io.h>

#define LED_R_PORT 				PORTE		// rationale: RED LED is on portE
#define LED_R_PIN_bm			PIN5_bm		// rationale: RED LED is on pin5
#define LED_R_TC				TCE1		// rationale: RED LED is on OC1x pins

#define LED_G_PORT 				PORTE		// rationale: GREEN LED is on portE
#define LED_G_PIN_bm			PIN4_bm		// rationale: GREEN LED is on pin4
#define LED_G_TC				TCE1		// rationale: GREEN LED is on OC1x pins

#define LED_B_PORT 				PORTD		// rationale: BLUE LED is on portD
#define LED_B_PIN_bm			PIN4_bm		// rationale: BLUE LED is on pin4
#define LED_B_TC				TCD1		// rationale: BLUE LED is on OC1x pins

// Set RGB LED pins as output & initialize timers for PWM
void RGB_LED_init(void);

inline uint8_t get_red_led() { return LED_R_TC.CCB; }
inline void set_red_led(uint8_t saturation) { LED_R_TC.CCBBUF = saturation; }
inline void force_set_red_led(uint8_t saturation) { LED_R_TC.CCBBUF = saturation; }

// Get and set intensity for green LED
inline uint8_t get_green_led() { return LED_G_TC.CCA; }
inline void set_green_led(uint8_t saturation) { LED_G_TC.CCABUF = saturation; }
inline void force_set_green_led(uint8_t saturation) { LED_G_TC.CCABUF = saturation; }
	
// Get and set intensity for blue LED
inline uint8_t get_blue_led() { return LED_B_TC.CCA; }
inline void set_blue_led(uint8_t saturation) { LED_B_TC.CCABUF = saturation; }
inline void force_set_blue_led(uint8_t saturation) { LED_B_TC.CCABUF = saturation; }

void led_off();
void set_rgb(uint8_t r, uint8_t g, uint8_t b);
void set_hsv(uint16_t h, uint8_t s, uint8_t v);
void hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b);

#endif

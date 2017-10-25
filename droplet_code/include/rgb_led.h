#pragma once
#include "droplet_base.h"
#include <math.h>

#define LED_R_PIN_bm			PIN5_bm
#define LED_G_PIN_bm			PIN4_bm
#define LED_B_PIN_bm			PIN4_bm

void rgbLEDinit(void); // Set RGB LED pins as output & initialize timers for PWM

// Get and set intensity for red LED
inline uint8_t getRedLED(void){ return TCE1.CCBBUF; }
inline void setRedLED(uint8_t saturation) { TCE1.CCBBUF = saturation; }

// Get and set intensity for green LED
inline uint8_t getGreenLED(void){ return TCE1.CCABUF; }
inline void setGreenLED(uint8_t saturation) { TCE1.CCABUF = saturation; }
	
// Get and set intensity for blue LED
inline uint8_t getBlueLED(void){ return TCD1.CCABUF; }
inline void setBlueLED(uint8_t saturation) { TCD1.CCABUF = saturation;	}


void ledOff(void);
void setRGB(uint8_t r, uint8_t g, uint8_t b);
void setHSV(uint16_t h, uint8_t s, uint8_t v);
void hsvToRGB(uint16_t h, uint8_t s, uint8_t v, uint8_t* r, uint8_t* g, uint8_t* b);

inline void warningLightSequence(void){
	setRGB(0,100,100); delayMS(300); setRGB(100,0,100); delayMS(300); setRGB(100,100,0); delayMS(300); ledOff();
	setRGB(0,100,100); delayMS(300); setRGB(100,0,100); delayMS(300); setRGB(100,100,0); delayMS(300); ledOff();
	setRGB(0,100,100); delayMS(300); setRGB(100,0,100); delayMS(300); setRGB(100,100,0); delayMS(300); ledOff();
}
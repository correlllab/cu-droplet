#ifndef IR_sensor_h
#define IR_sensor_h
#define F_CPU 32000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "sp_driver.h"
#include "stddef.h"				// required to use "offsetof(_,_)" for production row access
#include "RGB_LED.h"			// currently uses some color flashes for debugging
//#include "IRcom.h"


#define IR_SENSOR_0_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN5_gc		// IR0 sensor on PB5
#define IR_SENSOR_1_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN6_gc		// IR1 sensor on PB6
#define IR_SENSOR_2_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN7_gc		// IR2 sensor on PB7
#define IR_SENSOR_3_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN4_gc		// IR3 sensor on PB4
#define IR_SENSOR_4_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN2_gc		// IR4 sensor on PB2
#define IR_SENSOR_5_FOR_ADCB_MUXPOS		ADC_CH_MUXPOS_PIN3_gc		// IR5 sensor on PB3

#define IR_SENSOR_PORT PORTB

#define IR_SENSOR_0_PIN_bm		PIN5_bm
#define IR_SENSOR_1_PIN_bm		PIN6_bm
#define IR_SENSOR_2_PIN_bm		PIN7_bm
#define IR_SENSOR_3_PIN_bm		PIN4_bm
#define IR_SENSOR_4_PIN_bm		PIN2_bm
#define IR_SENSOR_5_PIN_bm		PIN3_bm

void IR_sensor_init();

uint8_t get_IR_sensor(uint8_t sensor_num);

int8_t find_median(int8_t* meas); // Helper function for getting the middle of the 3 measurements

inline void IR_sensor_enable(){ ADCB.CTRLA |= ADC_ENABLE_bm; }
inline void IR_sensor_disable(){ ADCB.CTRLA &= ~ADC_ENABLE_bm; }
	

#endif

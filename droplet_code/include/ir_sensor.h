/** \file *********************************************************************
 * Low level sensing functions using IR channels.
 * Note that there is no IR communication code in this file.
 *****************************************************************************/
#pragma once

#include "droplet_base.h"
#include "ir_comm.h"
#include "ir_led.h"

#define IR_SENSE_MAX 2047

/**
 * \brief Can be used to check if object(s) are within 1cm of this Droplet.
 *
 * \return A bit-mask with 1 set in the directions where objects are detected within 1 cm.
 * Direction macros are defined in droplet_init.h.
 *
 */
void irSensorInit(void);
void getIrSensors(int16_t* output_arr, uint8_t meas_per_ch);

//int16_t get_ir_sensor(uint8_t sensor_num, uint8_t ir_meas_count);
void checkCollisionValues(int16_t meas[6]);
uint8_t checkCollisions(void);
void initIrBaselines(void);
void updateIrBaselines(void);

#ifdef AUDIO_DROPLET
	inline void irSensorEnable(void){ ADCA.CTRLA |= ADC_ENABLE_bm; ADCB.CTRLA |= ADC_ENABLE_bm; }
	inline void irSensorDisable(void){ ADCA.CTRLA &= ~ADC_ENABLE_bm; ADCB.CTRLA &= ~ADC_ENABLE_bm; }
#else
	#define IR_SENSOR_PORT PORTB

	#define IR_SENSOR_0_PIN_bm		PIN5_bm
	#define IR_SENSOR_1_PIN_bm		PIN6_bm
	#define IR_SENSOR_2_PIN_bm		PIN7_bm
	#define IR_SENSOR_3_PIN_bm		PIN4_bm
	#define IR_SENSOR_4_PIN_bm		PIN2_bm
	#define IR_SENSOR_5_PIN_bm		PIN3_bm
	#define ALL_IR_SENSOR_PINS_bm (PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm)

	#define MUX_IR_SENSOR_0		ADC_CH_MUXPOS_PIN5_gc		// IR0 sensor on PB5
	#define MUX_IR_SENSOR_1		ADC_CH_MUXPOS_PIN6_gc		// IR1 sensor on PB6
	#define MUX_IR_SENSOR_2		ADC_CH_MUXPOS_PIN7_gc		// IR2 sensor on PB7
	#define MUX_IR_SENSOR_3		ADC_CH_MUXPOS_PIN4_gc		// IR3 sensor on PB4
	#define MUX_IR_SENSOR_4		ADC_CH_MUXPOS_PIN2_gc		// IR4 sensor on PB2
	#define MUX_IR_SENSOR_5		ADC_CH_MUXPOS_PIN3_gc		// IR5 sensor on PB3
	#define MUX_SENSOR_CLR		0b00000111

	inline void irSensorEnable(void){ ADCB.CTRLA |= ADC_ENABLE_bm; }
	inline void irSensorDisable(void){ ADCB.CTRLA &= ~ADC_ENABLE_bm; }
#endif
/** \file *********************************************************************
 * Low level sensing functions using IR channels.
 * Note that there is no IR communication code in this file.
 *****************************************************************************/
#ifndef ir_sensor_h
#define ir_sensor_h

#include <avr/io.h>
#include "scheduler.h"
#include "delay_x.h"
#include "droplet_init.h"
#include "ir_led.h"

//IR0_SEN	PORTA	PIN5
//IR1_SEN	PORTA	PIN6
//IR2_SEN	PORTA	PIN7
//IR3_SEN	PORTB	PIN4
//IR4_SEN	PORTB	PIN2
//IR5_SEN	PORTB	PIN3


#define IR_MEAS_COUNT 5

int16_t ir_sense_baseline[6];
/**
 * \brief Can be used to check if object(s) are within 1cm of this Droplet.
 *
 * \return A bit-mask with 1 set in the directions where objects are detected within 1 cm.
 * Direction macros are defined in droplet_init.h.
 *
 */
void ir_sensor_init();
uint8_t get_ir_sensor(uint8_t sensor_num);
uint8_t check_collisions();
int16_t meas_find_median(int16_t* meas, uint8_t arr_len); // Helper function for getting the middle of the 3 measurements
inline void ir_sensor_enable(){ ADCB.CTRLA |= ADC_ENABLE_bm; }
inline void ir_sensor_disable(){ ADCB.CTRLA &= ~ADC_ENABLE_bm; }

#endif
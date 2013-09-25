#include <avr/io.h>

#ifndef IR_sensor_h
#define IR_sensor_h

#define IR_SENSOR_PORT PORTB

#define IR_SENSOR_0_PIN_bm		PIN5_bm
#define IR_SENSOR_1_PIN_bm		PIN6_bm
#define IR_SENSOR_2_PIN_bm		PIN7_bm
#define IR_SENSOR_3_PIN_bm		PIN4_bm
#define IR_SENSOR_4_PIN_bm		PIN2_bm
#define IR_SENSOR_5_PIN_bm		PIN3_bm

void IR_sensor_init();

uint8_t get_IR_sensor(uint8_t sensor_num);

void IR_sensor_enable();

void IR_sensor_disable();



#endif

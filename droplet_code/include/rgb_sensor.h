#ifndef rgb_sensor_h
#define rgb_sensor_h

#include "avr/io.h" //includes stdlib OR DOES IT?
#include "rgb_led.h"
#include "scheduler.h"
#include "i2c.h"

#define RGB_SENSE_ADDR 0x29

void rgb_sensor_init();
void get_rgb(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c);

#endif
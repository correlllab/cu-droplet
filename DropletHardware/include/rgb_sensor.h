#ifndef rgb_sensor_h
#define rgb_sensor_h

#include "avr/io.h" //includes stdlib OR DOES IT?
#include <stdlib.h>
#include "rgb_led.h"

#define RGB_MEAS_COUNT 3
#define LED_OFF_DELAY_MS 150

#define RGB_SENSOR_PORT PORTA
#define RGB_SENSOR_R_PIN_bm PIN5_bm
#define RGB_SENSOR_G_PIN_bm PIN6_bm
#define RGB_SENSOR_B_PIN_bm PIN7_bm

uint8_t rsenbase;
uint8_t gsenbase;
uint8_t bsenbase;

void rgb_sensor_init();

void get_rgb_sensors(int8_t* r, int8_t* g, int8_t* b);
int8_t get_red_sensor();
int8_t get_green_sensor();
int8_t get_blue_sensor();

int8_t find_median(int8_t* meas, uint8_t arr_len);

#endif

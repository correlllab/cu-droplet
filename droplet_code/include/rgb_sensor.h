#ifndef rgb_sensor_h
#define rgb_sensor_h

#include "avr/io.h" //includes stdlib OR DOES IT?
#include "rgb_led.h"
#include "scheduler.h"

#define RGB_MEAS_COUNT 5
#define LED_OFF_DELAY_MS 150

#define RGB_SENSOR_PORT PORTA
#define RGB_SENSOR_R_PIN_bm PIN5_bm
#define RGB_SENSOR_G_PIN_bm PIN6_bm
#define RGB_SENSOR_B_PIN_bm PIN7_bm

uint8_t rsenbase;
uint8_t gsenbase;
uint8_t bsenbase;

void rgb_sensor_init();

void get_rgb_sensors(int16_t* r, int16_t* g, int16_t* b);
int16_t get_red_sensor();
int16_t get_green_sensor();
int16_t get_blue_sensor();

int16_t find_median(int16_t* meas, uint8_t arr_len);

#endif

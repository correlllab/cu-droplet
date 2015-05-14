#ifndef rgb_sensor_h
#define rgb_sensor_h

#include "avr/io.h" //includes stdlib OR DOES IT?
#include "rgb_led.h"
#include "scheduler.h"
#include "flash_api.h"

#define RGB_MEAS_COUNT 5
#define LED_OFF_DELAY_MS 150

#define RGB_SENSOR_PORT PORTA
#define RGB_SENSOR_R_PIN_bm PIN5_bm
#define RGB_SENSOR_G_PIN_bm PIN6_bm
#define RGB_SENSOR_B_PIN_bm PIN7_bm

void rgb_sensor_init();

int16_t r_baseline, g_baseline, b_baseline;
void get_rgb_sensors(int8_t* r, int8_t* g, int8_t* b);
//void get_cal_rgb(uint8_t* r, uint8_t* g, uint8_t* b);
int16_t get_red_sensor();
int16_t get_green_sensor();
int16_t get_blue_sensor();

void read_color_settings();
void write_color_settings();

typedef enum
{
	R, G, B
} Colors;

int16_t meas_find_median(int16_t* meas, uint8_t arr_len);

float calib_matrix[3][3];

typedef union
{
	uint32_t i;
	float f;
} u;

#endif

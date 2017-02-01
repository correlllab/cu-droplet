#pragma once

#include <avr/pgmspace.h>

#include "avr/io.h" 
#include "rgb_led.h"
#include "scheduler.h"
#include "i2c.h"

static const char RGB_SENSE_POWERON_FAILURE[] PROGMEM = "RGB sense power-on failed %hu.\r\n";

	void rgb_sensor_init();

#ifdef AUDIO_DROPLET

	#define RGB_SENSE_ADDR 0x29

#else

	#include "flash_api.h"

	#define RGB_MEAS_COUNT 5

	#define RGB_SENSOR_PORT PORTA
	#define RGB_SENSOR_R_PIN_bm PIN5_bm
	#define RGB_SENSOR_G_PIN_bm PIN6_bm
	#define RGB_SENSOR_B_PIN_bm PIN7_bm

	int16_t r_baseline, g_baseline, b_baseline;
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

	void get_rgb(int16_t *r, int16_t *g, int16_t *b);
#pragma once
#include "droplet_base.h"
#include "rgb_led.h"
#include "i2c.h"

void rgb_sensor_init(void);

#ifdef AUDIO_DROPLET

	#define RGB_SENSE_ADDR 0x29

#else

	#define RGB_MEAS_COUNT 5

	#define RGB_SENSOR_PORT PORTA
	#define RGB_SENSOR_R_PIN_bm PIN5_bm
	#define RGB_SENSOR_G_PIN_bm PIN6_bm
	#define RGB_SENSOR_B_PIN_bm PIN7_bm

	int16_t get_red_sensor(void);
	int16_t get_green_sensor(void);
	int16_t get_blue_sensor(void);

#endif

	void get_rgb(int16_t *r, int16_t *g, int16_t *b);

	int16_t meas_find_median(int16_t* meas, uint8_t arr_len); // Helper function for getting the middle of the 3 measurements
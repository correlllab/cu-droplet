#pragma once

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "droplet_init.h"
#include "pc_comm.h"
#include "rgb_led.h"
#include "range_algs.h"
#include "scheduler.h"
#include "speaker.h"
#include "rgb_sensor.h"

void handle_serial_command(char* command, uint16_t command_length);
void send_id();


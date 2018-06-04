#pragma once
#include "droplet_base.h"

#include "pc_comm.h"
#include "rgb_led.h"
#include "range_algs.h"
#include "scheduler.h"
#include "speaker.h"
#include "rgb_sensor.h"
#include "motor.h"
#include "ir_comm.h"
#include "reprogram.h"

void handleSerialCommand(char* command, uint16_t command_length);
void sendID(void);

void sendIrReprogMessage(void);
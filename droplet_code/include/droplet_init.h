#pragma once

//#define INIT_DEBUG_MODE

#ifdef INIT_DEBUG_MODE
#define INIT_DEBUG_PRINT(x) printf(x)
#else
#define INIT_DEBUG_PRINT(x)
#endif

#include "droplet_base.h"
#include "scheduler.h"
#include "pc_comm.h"
#include "rgb_led.h"
#include "rgb_sensor.h"
#include "power.h"
#include "i2c.h"
#include "range_algs.h"
#include "rgb_sensor.h"
#include "ir_led.h"
#include "ir_sensor.h"
#include "ir_comm.h"
#include "nvm.h"
#include "speaker.h"
#include "mic.h"
#include "motor.h"
#include "random.h"
#include "firefly_sync.h"
#include "serial_handler.h"
#include "matrix_utils.h"
#include "localization.h"
#include "reprogram.h"

extern void init(void);//_attribute__ ((section (".usrtxt")));
extern void loop(void);
extern void handleMsg(irMsg* msg_struct);
extern void handleMeas(Rnb* meas);
void initWrapper(void) __attribute__ ((section (".wrapper"),noinline));
void loopWrapper(void) __attribute__ ((section (".wrapper"),noinline));
void handleMsgWrapper(irMsg* msg_struct) __attribute__ ((section (".wrapper"),noinline));
void handleMeasWrapper(Rnb* meas) __attribute__ ((section (".wrapper"),noinline));

extern uint8_t userHandleCommand(char* commandWord, char* commandArgs);
extern void userMicInterrupt(void);


void startupLightSequence(void);
uint8_t getDropletOrd(id_t id);

extern const id_t OrderedBotIDs[121];

inline id_t getIdFromOrd(uint8_t ord){
	return pgm_read_word(&OrderedBotIDs[ord]);
}
#pragma once
#include "droplet_base.h"
#include "rgb_led.h"
#include "motor.h"



void powerInit(void); //just calls cap_monitor and leg_monitor init
void capMonitorInit(void);
void legMonitorInit(void);

uint8_t capStatus(void);			// Returns 0 if cap is within normal range ( 2.8V -- 5V ),
								//         1 if cap voltage is dangerously high ( > 5V )
								//         -1 if cap voltage is dangerously low ( < 2.8V )

int8_t legStatus(uint8_t leg);
uint8_t legsPowered(void);

#ifdef FIX_UNPOWERED_STATE
	volatile uint8_t failedLegChecks;
	volatile Task_t* legCheckTask;
	void stopLowPowerMoveTask(void);
	void checkLegsTask(void);
#endif

void enableLowPowerInterrupts(void);
void disableLowPowerInterrupts(void);

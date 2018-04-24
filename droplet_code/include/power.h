#pragma once
#include "droplet_base.h"
#include "rgb_led.h"
#include "delay_x.h"
#include "motor.h"

void powerInit(void); //just calls cap_monitor and leg_monitor init
void capMonitorInit(void);
void legMonitorInit(void);

void enableLowPowerInterrupt(void);
void disableLowPowerInterrupt(void);

uint8_t capStatus(void);			// Returns 0 if cap is within normal range ( 2.8V -- 5V ),
								//         1 if cap voltage is dangerously high ( > 5V )
								//         -1 if cap voltage is dangerously low ( < 2.8V )

int8_t legStatus(uint8_t leg);
uint8_t legsPowered(void);
uint8_t legsFloating(void);

#ifdef FIX_UNPOWERED_STATE
	volatile uint8_t failedLegChecks;
	volatile uint8_t unpoweredFixActive;
	void stopLowPowerMoveTask(void);
	void checkLegsTask(void);
#endif

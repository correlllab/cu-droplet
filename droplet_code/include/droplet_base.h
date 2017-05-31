#pragma once

#define AUDIO_DROPLET
//#define SYNCHRONIZED

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <util/crc16.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <string.h>

typedef uint16_t id_t;

//Any #defines needed by other Droplet files
#define DIR0		((uint8_t)0x01)
#define DIR1		((uint8_t)0x02)
#define DIR2		((uint8_t)0x04)
#define DIR3		((uint8_t)0x08)
#define DIR4		((uint8_t)0x10)
#define DIR5		((uint8_t)0x20)
#define ALL_DIRS	((uint8_t)0x3F)

#define DIR_NE		DIR0
#define DIR_E		DIR1
#define DIR_SE		DIR2
#define DIR_SW		DIR3
#define DIR_W		DIR4
#define DIR_NW		DIR5

id_t droplet_ID;
volatile uint16_t rtc_epoch;

// Returns the number of ms elapsed since the last reset. (Defined in droplet_base_asm.s)
uint32_t get_time(void);

// Returns this Droplet's unique 16-bit identifier. 0 will never be an identifier.
inline id_t get_droplet_id(void){ 
	return droplet_ID;
}

inline void* myMalloc(size_t size){
	void* tmp = NULL;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		tmp = malloc(size);
	}
	return tmp;
}

inline void myFree(void* ptr){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		free(ptr);
	}
}

inline void Config32MHzClock(void){
	// Set system clock to 32 MHz
	CCP = CCP_IOREG_gc;
	OSC.CTRL = OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP = CCP_IOREG_gc;
	CLK.CTRL = 0x01;
	//OSC.RC32KCAL = PRODSIGNATURES_RCOSC32K;
	// Set up real-time clock
	CLK.RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;
	//RTC.INTCTRL = RTC_OVFINTLVL_LO_gc;
	while (RTC.STATUS & RTC_SYNCBUSY_bm);	// wait for SYNCBUSY to clear
	
	RTC.PER = 0xFFFF;		//(2^16)-1 milliseconds is 65.535 seconds
	RTC.CTRL = RTC_PRESCALER_DIV1_gc;

	// reset RTC to 0, important for after a reboot:
	while(RTC.STATUS & RTC_SYNCBUSY_bm);	// wait for SYNCBUSY to clear
	RTC.CNT = 0;
}

inline static void delay_us(double __us){ _delay_us(__us); }

/**
 * \brief Resets the Droplet's program counter and clears all low-level system buffers.
 */
inline void droplet_reboot(void){
	CPU_CCP=CCP_IOREG_gc;
	RST.CTRL = 0x1;
}


/* 
 * Delay ms milliseconds
 * (the built-in _delay_ms only takes constant arguments, not variables)
 */
void delay_ms(uint16_t ms);
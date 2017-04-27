#include <avr/io.h>

.extern rtc_epoch
; Gets the time as an atomic operation
.global get_time
get_time:
	in		r0, _SFR_IO_ADDR(SREG)		; Save the state of the status register
	cli									; Disable interrupts
	lds		r22, RTC_CNT				; Load low byte into return value
	lds		r23, RTC_CNT+1
	lds		r24, rtc_epoch				; epoch 16-bit high word
	lds		r25, rtc_epoch+1
	out		_SFR_IO_ADDR(SREG), r0		; Restore status register (including re-enabling interrupts if necessary)
	ret

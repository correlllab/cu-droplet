#include <avr/io.h>

/*.extern run_tasks*/
/*.extern task_executing*/
.extern rtc_epoch
/*.global rtc_compare_isr

; RTC compare match interrupt vector
; Called when it's time for a scheduled task to run
; * Saves all the registers on the stack, including the CPU status register
; * Pushes the address of run_tasks() on the stack
; * Increments the num_executing_tasks variable
; * Returns from the interrupt handler
; Since run_tasks is on top of the stack, the IRS returns into the beginning of run_tasks
.global RTC_COMP_vect
RTC_COMP_vect:
rtc_compare_isr:
	push	r0							; Save all the registers on the stack
	push	r1
	push	r2
	push	r3
	push	r4
	push	r5
	push	r6
	push	r7
	push	r8
	push	r9
	push	r10
	push	r11
	push	r12
	push	r13
	push	r14
	push	r15
	push	r16
	push	r17
	push	r18
	push	r19
	push	r20
	push	r21
	push	r22
	push	r23
	push	r24
	push	r25
	push	r26
	push	r27
	push	r28
	push	r29
	push	r30
	push	r31
	in		r16, _SFR_IO_ADDR(SREG)		; Save the CPU status register
	push	r16
	ldi		ZL,	pm_lo8(run_tasks)		; Push the address of run_tasks on the stack
	ldi		ZH, pm_hi8(run_tasks)		; Returning from the ISR will return to this address
	ldi		r16, pm_hh8(run_tasks)
	push	ZL
	push	ZH
	push	r16
	lds		r24, task_executing	; Increment num_executing_tasks
	inc		r24
	sts		task_executing, r24
	reti								; reti returns from the ISR to the beginning of run_tasks

; Restores the registers that were saved in RTC_COMP ISR and decrements the num_executing_tasks counter
; After popping the registers, the top of the stack points to the instruction that would have been next if 
; the RTC_COMP interrupt had not occurred.  
; Returning from this subroutine returns program control to where it would have been if the RTC_COMP interrupt
; had not occurred
.global restore_registers
restore_registers:
	lds		r24, task_executing	; Decrement num_executing_tasks
	dec		r24
	sts		task_executing, r24
	pop		r16							; Restore CPU status register
	out		_SFR_IO_ADDR(SREG), r16
	pop		r31								; Restore all registers
	pop		r30
	pop		r29
	pop		r28
	pop		r27
	pop		r26
	pop		r25
	pop		r24
	pop		r23
	pop		r22
	pop		r21
	pop		r20
	pop		r19
	pop		r18
	pop		r17
	pop		r16
	pop		r15
	pop		r14
	pop		r13
	pop		r12
	pop		r11
	pop		r10
	pop		r9
	pop		r8
	pop		r7
	pop		r6
	pop		r5
	pop		r4
	pop		r3
	pop		r2
	pop		r1
	pop		r0
	ret								; The top value of the stack was the code address that
										; was pushed when the interrupt occurred
										; ret returns to where we were before the RTC_COMP interrupt happened

*/

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

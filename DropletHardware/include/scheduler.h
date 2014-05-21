#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>
//#include <stdlib.h>
#include <stdio.h>
#include "RGB_LED.h"

#define F_CPU 32000000UL

// A task is a function, possibly with an argument, to be called at a specific time
// scheduled_time is the 32-bit global time when the function should be called
// task_function is the function to call. Its prototype must be "void foo(void *arg)"
// arg is the argument to pass to task_function.  arg must be typecast to a void*
// next is a pointer to the next task to be executed, after this one
typedef struct task
{
	uint32_t scheduled_time;
	void (*task_function)(void*);
	void* arg;
	struct task *next;
} Task_t;

// Global task list
// Linked list of tasks, sorted by time until execution
Task_t *task_list;
volatile uint16_t rtc_epoch;
static uint8_t SCHEDULER_DEBUG_MODE = 0;
volatile uint8_t num_tasks, num_executing_tasks;

// Get the current 32-bit time, as measured in ms from the last reset
inline volatile extern uint32_t get_32bit_time() __attribute__((OS_task));

void scheduler_init();
void Config32MHzClock(void);
void set_current_time(uint16_t count);
void delay_ms(uint16_t ms);
static inline void delay_us(double __us){ _delay_us(__us); }
inline uint16_t get_16bit_time(){ return RTC.CNT; }

/* 
 * Adds a new task to the task queue
 * time is number of milliseconds from present until function is executed
 * function is a function pointer to execute
 * arg is the argument to supply to function
 * Example: schedule_task(1000, foo, (void*)55)
 * will call foo(55) in one second
 * Returns a pointer to the task that can be used to remove the task from the queue
 */
Task_t* schedule_task(uint32_t time, void (*function)(void*), void* arg);
void remove_task(Task_t* task); // Removes a task from the queue
void print_task_queue();

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
void run_tasks() __attribute__((OS_task));
void restore_registers() __attribute__((naked));


#endif
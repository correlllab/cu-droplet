#pragma once

#define F_CPU 32000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "rgb_led.h"


#define RTC_COMP_INT_LEVEL RTC_COMPINTLVL_LO_gc;

#define MAX_NUM_SCHEDULED_TASKS 10

#define MIN_TASK_TIME_IN_FUTURE 20

typedef union flex_function
{
	void (*arg_function)(void*);
	void (*noarg_function)();	
} flex_function;

// A task is a function, possibly with an argument, to be called at a specific time
// scheduled_time is the 32-bit global time when the function should be called
// task_function is the function to call. Its prototype must be "void foo(void *arg)"
// arg is the argument to pass to task_function.  arg must be typecast to a void*
// next is a pointer to the next task to be executed, after this one
typedef struct task
{
	uint32_t scheduled_time;
	uint32_t period;
	flex_function func;
	void* arg;
	struct task* next;
} Task_t;

// Global task list
// Linked list of tasks, sorted by time until execution
volatile Task_t *task_list;

volatile uint16_t rtc_epoch;
volatile uint8_t num_tasks, task_executing;

// Get the current 32-bit time, as measured in ms from the last reset
inline volatile extern uint32_t get_time() __attribute__((OS_task));

void scheduler_init();
void Config32MHzClock(void);
void delay_ms(uint16_t ms);
static inline void delay_us(double __us){ _delay_us(__us); }
void task_list_cleanup();

/* 
 * Adds a new task to the task queue
 * time is number of milliseconds from present until function is executed
 * function is a function pointer to execute
 * arg is the argument to supply to function
 * Example: schedule_task(1000, foo, (void*)55)
 * will call foo(55) in one second
 * Returns a pointer to the task that can be used to remove the task from the queue
 */
Task_t* schedule_task(uint32_t time, void (*function)(), void* arg);
// This function primarily calls the above, but always to run 10ms in the future, and then repeat with a certain period.
Task_t* schedule_periodic_task(uint32_t period, void (*function)(), void* arg);
void add_task_to_list(Task_t* task);
void remove_task(Task_t* task); // Removes a task from the queue
void print_task_queue();

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
void run_tasks() __attribute__((OS_task));
void restore_registers() __attribute__((naked));

/* To avoid issues caused by malloc, we're going to have some "fake" malloc functions.
 * We'll keep an array of MAX_NUM_SCHEDULED_TASKS Task_t structs, and fake_malloc()
 * will return pointers to those as appropriate.
 */
static volatile Task_t task_storage_arr[MAX_NUM_SCHEDULED_TASKS];
//static uint8_t curr_pointer;


//static inline Task_t* scheduler_malloc()
//{
	//if(num_tasks>=MAX_NUM_SCHEDULED_TASKS) return NULL;
	//
	//uint8_t tmp;
	//for(tmp=(curr_pointer+1)%MAX_NUM_SCHEDULED_TASKS ; tmp!=curr_pointer ; tmp=(tmp+1)%MAX_NUM_SCHEDULED_TASKS)
	//{
		////This code assumes that all tasks will have non-null function pointers.
		//if((task_storage_arr[tmp].func).noarg_function == NULL) break;
	//}
	//curr_pointer = tmp;
	//return &(task_storage_arr[curr_pointer]);
//}

static inline Task_t* scheduler_malloc()
{
	if(num_tasks>=MAX_NUM_SCHEDULED_TASKS) return NULL;

	for(uint8_t tmp=0 ; tmp<MAX_NUM_SCHEDULED_TASKS ; tmp++)
	{
		//This code assumes that all tasks will have non-null function pointers.
		if((task_storage_arr[tmp].func.noarg_function) == NULL)
		{
			return &(task_storage_arr[tmp]);
		}
	}
	
	printf_P(PSTR("No empty spot found in scheduler_malloc, but num_tasks wasn't greater than or equal max_tasks.\r\n"));
	return NULL;

}

//Returns '1' if the next task to run is scheduled for more than 255ms in the past. If this occurs, call task_list_cleanup.
inline uint8_t task_list_check()
{ 
	if(task_executing)	return 0;
	else				return (((int32_t)(get_time()-(task_list->scheduled_time)))>255); 
}

//static inline void scheduler_free(Task_t* tgt)
//{
	//for(uint8_t tmp=curr_pointer; tmp!=(curr_pointer+1)%MAX_NUM_SCHEDULED_TASKS; tmp = (tmp+(MAX_NUM_SCHEDULED_TASKS-1))%MAX_NUM_SCHEDULED_TASKS)
	//{
		//if(&(task_storage_arr[tmp])==tgt)
		//{
			//(task_storage_arr[tmp].func).noarg_function = NULL;
			//curr_pointer = ((tmp+(MAX_NUM_SCHEDULED_TASKS-1))%MAX_NUM_SCHEDULED_TASKS);
			//return;
		//}
	//}
	//printf("In scheduler_free, task_storage_arr[tmp] was never equal to tgt.\r\n");
//}

static inline void scheduler_free(Task_t* tgt)
{
	tgt->arg = 0;
	tgt->period = 0;
	(tgt->func).noarg_function = NULL;
	tgt->scheduled_time = 0;
	tgt->next = NULL;
}

//void task_list_checkup();

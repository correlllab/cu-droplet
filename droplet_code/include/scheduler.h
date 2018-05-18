#pragma once

#include "droplet_base.h"
#include "rgb_led.h"

#define RTC_COMP_INT_LEVEL RTC_COMPINTLVL_LO_gc;
#define MAX_NUM_SCHEDULED_TASKS 10
#define MIN_TASK_TIME_IN_FUTURE 10

typedef void (*arg_func_t)(void*);
typedef void (*noarg_func_t)(void);

typedef union flex_function_union
{
	arg_func_t arg_function;
	noarg_func_t noarg_func;
} FlexFunction __attribute__ ((__transparent_union__)) ;

// A task is a function, possibly with an argument, to be called at a specific time
// scheduled_time is the 32-bit global time when the function should be called
// task_function is the function to call. Its prototype must be "void foo(void *arg)"
// arg is the argument to pass to task_function.  arg must be typecast to a void*
// next is a pointer to the next task to be executed, after this one
typedef struct task{
	uint32_t scheduled_time;
	uint32_t period;
	FlexFunction func;
	void* arg;
	volatile struct task* next;
} Task_t;

// Global task list
// Linked list of tasks, sorted by time until execution
volatile Task_t *task_list;

volatile uint8_t num_tasks, task_executing;

void schedulerInit(void);
void taskListCleanup(void);

/* 
 * Adds a new task to the task queue
 * time is number of milliseconds from present until function is executed
 * function is a function pointer to execute
 * arg is the argument to supply to function
 * Example: schedule_task(1000, foo, (void*)55)
 * will call foo(55) in one second
 * Returns a pointer to the task that can be used to remove the task from the queue
 */
volatile Task_t* scheduleTask(uint32_t time, FlexFunction function, void* arg);
// This function primarily calls the above, but always to run 10ms in the future, and then repeat with a certain period.
volatile Task_t* schedulePeriodicTask(uint32_t period, FlexFunction function, void* arg);

void removeTask(volatile Task_t*); // Removes a task from the queue
void printTaskQueue(void);

//Returns '1' if the next task to run is scheduled for more than 255ms in the past. If this occurs, call task_list_cleanup.
inline uint8_t taskListCheck(void){ 
	if(task_executing)	return 0;
	else				return (((int32_t)(getTime()-(task_list->scheduled_time)))>3000); 
}
static volatile Task_t task_storage_arr[MAX_NUM_SCHEDULED_TASKS];

#define SAVE_CONTEXT()                                  \
    asm volatile (  "push   r0                      \n\t"   \
                    "in     r0, 0x003F				\n\t"   \
                    "cli                            \n\t"   \
                    "push   r0                      \n\t"   \
                    "push   r1                      \n\t"   \
                    "clr    r1                      \n\t"   \
                    "push   r2                      \n\t"   \
                    "push   r3                      \n\t"   \
                    "push   r4                      \n\t"   \
                    "push   r5                      \n\t"   \
                    "push   r6                      \n\t"   \
                    "push   r7                      \n\t"   \
                    "push   r8                      \n\t"   \
                    "push   r9                      \n\t"   \
                    "push   r10                     \n\t"   \
                    "push   r11                     \n\t"   \
                    "push   r12                     \n\t"   \
                    "push   r13                     \n\t"   \
                    "push   r14                     \n\t"   \
                    "push   r15                     \n\t"   \
                    "push   r16                     \n\t"   \
                    "push   r17                     \n\t"   \
                    "push   r18                     \n\t"   \
                    "push   r19                     \n\t"   \
                    "push   r20                     \n\t"   \
                    "push   r21                     \n\t"   \
                    "push   r22                     \n\t"   \
                    "push   r23                     \n\t"   \
                    "push   r24                     \n\t"   \
                    "push   r25                     \n\t"   \
                    "push   r26                     \n\t"   \
                    "push   r27                     \n\t"   \
                    "push   r28                     \n\t"   \
                    "push   r29                     \n\t"   \
                    "push   r30                     \n\t"   \
                    "push   r31                     \n\t"   \
                );
				
#define RESTORE_CONTEXT()                               \
    asm volatile (  "pop    r31                     \n\t"   \
                    "pop    r30                     \n\t"   \
                    "pop    r29                     \n\t"   \
                    "pop    r28                     \n\t"   \
                    "pop    r27                     \n\t"   \
                    "pop    r26                     \n\t"   \
                    "pop    r25                     \n\t"   \
                    "pop    r24                     \n\t"   \
                    "pop    r23                     \n\t"   \
                    "pop    r22                     \n\t"   \
                    "pop    r21                     \n\t"   \
                    "pop    r20                     \n\t"   \
                    "pop    r19                     \n\t"   \
                    "pop    r18                     \n\t"   \
                    "pop    r17                     \n\t"   \
                    "pop    r16                     \n\t"   \
                    "pop    r15                     \n\t"   \
                    "pop    r14                     \n\t"   \
                    "pop    r13                     \n\t"   \
                    "pop    r12                     \n\t"   \
                    "pop    r11                     \n\t"   \
                    "pop    r10                     \n\t"   \
                    "pop    r9                      \n\t"   \
                    "pop    r8                      \n\t"   \
                    "pop    r7                      \n\t"   \
                    "pop    r6                      \n\t"   \
                    "pop    r5                      \n\t"   \
                    "pop    r4                      \n\t"   \
                    "pop    r3                      \n\t"   \
                    "pop    r2                      \n\t"   \
                    "pop    r1                      \n\t"   \
                    "pop    r0                      \n\t"   \
                    "out    0x003F, r0            \n\t"   \
                    "pop    r0                      \n\t"   \
                );
				
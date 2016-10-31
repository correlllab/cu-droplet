#include "scheduler.h"

static void add_task_to_list(volatile Task_t* task);
static int8_t run_tasks();

void scheduler_init(){
	task_list = NULL;
	num_tasks = 0;
	task_executing = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){  // Disable interrupts during initialization
		// Set up real-time clock
		rtc_epoch = 0;
		CLK.RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;
		RTC.INTCTRL = RTC_OVFINTLVL_HI_gc;		// High level overflow interrupt to increment the epoch counter
		while (RTC.STATUS & RTC_SYNCBUSY_bm);
		RTC.PER = 0xFFFF;
		while (RTC.STATUS & RTC_SYNCBUSY_bm);
		RTC.CTRL = RTC_PRESCALER_DIV1_gc;
		while (RTC.STATUS & RTC_SYNCBUSY_bm);
		RTC.CNT = 0;
	}
}

void Config32MHzClock(void){
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

// Delay ms milliseconds
// (the built-in _delay_ms only takes constant arguments, not variables)
void delay_ms(uint16_t ms){
	uint32_t cur_time, end_time;
	cli(); cur_time = get_time(); sei();
	end_time = cur_time + ms;
	while (1){
		cli();
		if (get_time() >= end_time){
			sei();
			return;
		}
		sei();
		delay_us(10);
	}
}

//This function checks for errors or inconsistencies in the task list, and attempts to correct them.
//Is this still needed?

void task_list_cleanup(){
	printf_P(PSTR("\tAttempting to restore task_list (by dropping all non-periodic tasks.\r\n\tIf you only see this message rarely, don't worry too much.\r\n"));

	volatile Task_t* cur_task = task_list;
	volatile Task_t* prev_task;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		RTC.INTCTRL &= ~RTC_COMP_INT_LEVEL;		
		while (cur_task != NULL){
			if(cur_task->period==0){
				prev_task = cur_task;
				cur_task = prev_task->next;
				remove_task(prev_task);
			}else{
				cur_task->scheduled_time=get_time()+cur_task->period+50;
				cur_task = cur_task->next;				
			}
		}
	}
}

// Adds a new task to the task queue
// time is number of milliseconds until function is executed
// function is a function pointer to execute
// arg is the argument to supply to function
volatile Task_t* schedule_task(uint32_t time, void (*function)(), void* arg){
	volatile Task_t* new_task;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		new_task = myMalloc(sizeof(Task_t));
		if (new_task == NULL){
			return NULL;
		}
		time+=MIN_TASK_TIME_IN_FUTURE*(time<MIN_TASK_TIME_IN_FUTURE);	
		new_task->scheduled_time = time + get_time();
		new_task->arg = arg;
		new_task->func.noarg_function = function;
		new_task->period = 0;
		new_task->next = NULL; 		
	}
	add_task_to_list(new_task);
	//printf("Task (%X->%X) scheduled for %lu\t[%hhu]\r\n", new_task, (new_task->func).noarg_function, new_task->scheduled_time, num_tasks);

	return new_task;
}

volatile Task_t* schedule_periodic_task(uint32_t period, void (*function)(), void* arg){
	period+=MIN_TASK_TIME_IN_FUTURE*(period<MIN_TASK_TIME_IN_FUTURE);	
	volatile Task_t* new_task = schedule_task(period, function, arg);
	new_task->period=period;
	return new_task;
}

static void add_task_to_list(volatile Task_t* task){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		// Find the new task's proper spot in the list of tasks
		// task_list is a linked list sorted by scheduled_time, smallest first
		task->next = task_list;
		
		// If the new task is the next to be executed, put it at the front of the list
		if (task_list == NULL || task->scheduled_time <= task_list->scheduled_time){
			task_list = task;
			// If scheduled_time is in the current epoch, set the RTC compare interrupt
			if(task_executing==0){			
				if (task->scheduled_time <= ((((uint32_t)rtc_epoch) << 16) | (uint32_t)RTC.PER)){
					while (RTC.STATUS & RTC_SYNCBUSY_bm);
					RTC.COMP = ((uint16_t)(task_list->scheduled_time))|0x8;
					RTC.INTCTRL |= RTC_COMP_INT_LEVEL;
				}else{
					RTC.INTCTRL &= ~RTC_COMP_INT_LEVEL;					
				}
			}
		}else{
			// If the new task is not the next to be executed, iterate through the task_list,
			// find its position in the linked list, and insert it there.
			volatile Task_t* tmp_task_ptr = task_list;
			while (tmp_task_ptr->next != NULL && task->scheduled_time > (tmp_task_ptr->next)->scheduled_time){
				if(tmp_task_ptr->next==tmp_task_ptr){
					//set_rgb(255, 50, 0);
					printf_P(PSTR("ERROR! Task list has self-reference.\r\n"));
					printf_P(PSTR("New Task %p (%p) scheduled at %lu with period %lu, %lu current\r\n"), task, (task->func).noarg_function, task->scheduled_time, task->period, get_time());
					print_task_queue();
					return;				
				}
				tmp_task_ptr = tmp_task_ptr->next;
			}
			//set_rgb(r, g, b);
			task->next = tmp_task_ptr->next;
			tmp_task_ptr->next = task;
		}
//
		////Check if the front of the task_lsit should have already been run, and set the RTC.COMP for that if neessary.
		//if (task_list != NULL && task_list->scheduled_time < get_time()) //
		//{
			//while (RTC.STATUS & RTC_SYNCBUSY_bm);
			//RTC.COMP = RTC.CNT+5;
			//RTC.INTCTRL |= RTC_COMP_INT_LEVEL;
		//}

		num_tasks++;
		
		//task_list_checkup();
	}
}

// Remove a task from the task queue
void remove_task(volatile Task_t* task){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if(task_list==task)	{
			task_list=task->next;
			num_tasks--;
		}else{
			volatile Task_t* tmp_task = task_list;
			while (tmp_task->next != NULL && tmp_task->next != task) tmp_task = tmp_task->next;
			if (tmp_task->next != NULL){
				tmp_task->next = task->next;
				num_tasks--;
			}
		}
		myFree((Task_t*)task);		
	}
}

void print_task_queue(){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){  // Disable interrupts during printing
		volatile Task_t* cur_task = task_list;
		
		printf_P(PSTR("Task Queue (%hu tasks, %hu executing):\r\n"), num_tasks, task_executing);
		
		// Iterate through the list of tasks, printing name, function, and scheduled time of each
		while (cur_task != NULL){
			printf_P(PSTR("\tTask %p (%p) scheduled at %lu with period %lu, %lu current\r\n"), cur_task, (cur_task->func).noarg_function, cur_task->scheduled_time, cur_task->period, get_time());
			if(cur_task==cur_task->next) break;
			cur_task = cur_task->next;
		}
	}
}

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
static int8_t run_tasks(){
	volatile Task_t* cur_task;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){ // Disable interrupts
		// Run all tasks that are scheduled to execute in the next 2ms
		// (The RTC compare register takes 2 RTC clock cycles to update)
		while (task_list != NULL && task_list->scheduled_time <= get_time() + 2){
			cur_task = task_list;
			task_list = cur_task->next;

			if(cur_task->arg==NULL){
				NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE){ // Enable interrupts during tasks
					(cur_task->func).noarg_function(); // run the task
				}
			}else{
				NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE){ // Enable interrupts during tasks
					(cur_task->func).arg_function(cur_task->arg); // run the task
				}
			}
			
			if(cur_task->period>0){
				cur_task->scheduled_time+=cur_task->period;
				cur_task->next=NULL;
				num_tasks--;
				add_task_to_list(cur_task);
			}else{		
				myFree((Task_t*)cur_task);
				cur_task = NULL;
				num_tasks--;
			}	
		}
		// If the next task to be executed is in the current epoch, set the RTC compare register and interrupt		
		if (task_list != NULL && task_list->scheduled_time <= ((((uint32_t)rtc_epoch) << 16) | (uint32_t)RTC.PER)){	
			while (RTC.STATUS & RTC_SYNCBUSY_bm);
			RTC.COMP = ((uint16_t)(task_list->scheduled_time))|0x8;
			RTC.INTCTRL |= RTC_COMP_INT_LEVEL;
		}else{
			RTC.INTCTRL &= ~RTC_COMP_INT_LEVEL;
		}
	}
	return 0;
}

ISR(RTC_COMP_vect){
	SAVE_CONTEXT();	
	task_executing=1;
	/*int8_t result =*/ run_tasks();
	task_executing=0;
	//if(result<0)
		//task_list_cleanup();		
	RESTORE_CONTEXT();	
}

// Increment rtc_epoch on RTC overflow
// Must be atomic so no reads of get_time() occur between RTC overflow and updating of epoch
ISR( RTC_OVF_vect ){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){ // Disable interrupts
		rtc_epoch++;
		// If the next task to run is in the current epoch, update the RTC compare value and interrupt
		if (task_list != NULL && task_list->scheduled_time < ((((uint32_t)rtc_epoch) << 16) | (uint32_t)RTC.PER)){
			if(!task_executing){
				if(task_list->scheduled_time < get_time()){
					//printf("In overflow, tasks need to have been executed!\r\n");
					//print_task_queue();
				}else{		
					while (RTC.STATUS & RTC_SYNCBUSY_bm);
					RTC.COMP = ((uint16_t)(task_list->scheduled_time))|0x8;
					RTC.INTCTRL |= RTC_COMP_INT_LEVEL;
				}
			}
		}else{
			printf("Next task not in current epoch. Task executing: %hu. Next task scheduled time: %lu. Time: %lu.\r\n", task_executing, task_list->scheduled_time, get_time());
		}
	}
}
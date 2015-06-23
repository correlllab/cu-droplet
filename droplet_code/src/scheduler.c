#include "scheduler.h"

/* To avoid issues caused by malloc, we're going to have some "fake" malloc functions.
 * We'll keep an array of MAX_NUM_SCHEDULED_TASKS Task_t structs, and fake_malloc()
 * will return pointers to those as appropriate.
 */
Task_t task_storage_arr[MAX_NUM_SCHEDULED_TASKS];
uint8_t curr_pointer;

inline Task_t* scheduler_malloc()
{
	if(num_tasks>=MAX_NUM_SCHEDULED_TASKS) return NULL;
	
	uint8_t tmp;
	for(tmp=(curr_pointer+1)%MAX_NUM_SCHEDULED_TASKS ; tmp!=curr_pointer ; tmp=(tmp+1)%MAX_NUM_SCHEDULED_TASKS)
	{
		//This code assumes that all tasks will have non-null function pointers.
		if(task_storage_arr[tmp].task_function == NULL) break;
	}
	curr_pointer = tmp;
	return &(task_storage_arr[curr_pointer]);
}

inline void scheduler_free(Task_t* tgt)
{
	for(uint8_t tmp=(curr_pointer); ; tmp = (tmp+(MAX_NUM_SCHEDULED_TASKS-1))%MAX_NUM_SCHEDULED_TASKS)
	{
		if(&(task_storage_arr[tmp])==tgt)
		{
			task_storage_arr[tmp].task_function = NULL;
			curr_pointer = ((tmp+(MAX_NUM_SCHEDULED_TASKS-1))%MAX_NUM_SCHEDULED_TASKS);
			return;
		}
	}
}

void scheduler_init()
{
	task_list = NULL;
	num_tasks = 0;
	num_executing_tasks = 0;
	curr_pointer = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)  // Disable interrupts during initialization
	{
		// Clear current task list, if necessary
		// (Should only be necessary if scheduler is re-initialized at runtime)
		
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

void Config32MHzClock(void)
{
	// Set system clock to 32 MHz
	CCP = CCP_IOREG_gc;
	OSC.CTRL = OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));
	CCP = CCP_IOREG_gc;
	CLK.CTRL = 0x01;
	
	// Set up real-time clock
	CLK.RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;	// per Dustin: RTCSRC is a 1 kHz oscillator, needs to be verified
	//RTC.INTCTRL = RTC_OVFINTLVL_LO_gc;
	while (RTC.STATUS & RTC_SYNCBUSY_bm);	// wait for SYNCBUSY to clear
	
	RTC.PER = 0xFFFF;		//	0xFFFF == 0b1111111111111111 = (2^16)-1
	// (2^16)-1 milliseconds is 65.535 seconds

	RTC.CTRL = RTC_PRESCALER_DIV1_gc;

	// reset RTC to 0, important for after a reboot:
	while(RTC.STATUS & RTC_SYNCBUSY_bm);	// wait for SYNCBUSY to clear
	
	RTC.CNT = 0;
}

// Delay ms milliseconds
// (the built-in _delay_ms only takes constant arguments, not variables)
void delay_ms(uint16_t ms)
{
	uint32_t cur_time, end_time;
	cli(); cur_time = get_time(); sei();
	end_time = cur_time + ms;
	while (1)
	{
		cli();
		if (get_time() >= end_time)
		{
			sei();
			return;
		}
		sei();
		delay_us(10);
	}
}

//This function checks for errors or inconsistencies in the task list, and attempts to correct them.
void task_list_cleanup()
{
	printf("\tPerforming task_list_cleanup!\r\n");
	print_task_queue();
	Task_t* cur_task = task_list;
	Task_t* tmp_task_ptr;
	Task_t* task_ptr_arr[MAX_NUM_SCHEDULED_TASKS];
	uint8_t num_periodic_tasks = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		while (cur_task != NULL)
		{
			if(cur_task->period==0)
			{
				tmp_task_ptr = cur_task->next;
				remove_task(cur_task);
				cur_task = tmp_task_ptr;
			}
			else
			{
				cur_task->scheduled_time=get_time()+cur_task->period+50; //TODO: Maybe remove this +50? We've added it to see if it fixes a bug.
				task_ptr_arr[num_periodic_tasks] = cur_task;
				cur_task = cur_task->next;
				task_ptr_arr[num_periodic_tasks]->next=NULL;
				num_periodic_tasks++;					
			}
			num_tasks=0;
		}
		task_list=NULL; //Now, the task list has been cleared out, but only non-periodic tasks have had their memory purged.
		for(uint8_t i=0;i<num_periodic_tasks;i++)
		{
			add_task_to_list(task_ptr_arr[i]);
		}
	}
}

// Adds a new task to the task queue
// time is number of milliseconds until function is executed
// function is a function pointer to execute
// arg is the argument to supply to function
Task_t* schedule_task(volatile uint32_t time, void (*function)(void*), void* arg)
{
	Task_t* new_task;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		new_task = (Task_t*)scheduler_malloc();
		if (new_task == NULL) return NULL;
	
		new_task->scheduled_time = time + get_time();
		if(0xFFFF&get_time()<2) new_task->scheduled_time += 10;
		new_task->arg = arg;
		new_task->task_function = function;
		new_task->period = 0;
		new_task->next = NULL; 		
	}
	add_task_to_list(new_task);
	if(new_task->next==new_task){
		printf("ERROR! New task has self-reference.\r\n");
		print_task_queue();
		return;				
	}	

	return new_task;
}

Task_t* schedule_periodic_task(uint32_t period, void (*function)(void*), void* arg)
{
	Task_t* new_task = schedule_task(get_time()+period, function, arg);
	new_task->period=period;
	return new_task;
}

void add_task_to_list(Task_t* task)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		// Find the new task's proper spot in the list of tasks
		// task_list is a linked list sorted by scheduled_time, smallest first
		task->next = task_list;
		
		// If the new task is the next to be executed, put it at the front of the list
		if (task_list == NULL || task->scheduled_time <= task_list->scheduled_time)
		{
			task_list = task;
			// If scheduled_time is in the current epoch, set the RTC compare interrupt
			if (task->scheduled_time <= ((((uint32_t)rtc_epoch) << 16) | (uint32_t)RTC.PER))
			{
				while (RTC.STATUS & RTC_SYNCBUSY_bm);
				RTC.COMP = (uint16_t)(0xFFFF&task->scheduled_time);
				RTC.INTCTRL |= RTC_COMPINTLVL_MED_gc;
			}
			else
			{
				RTC.INTCTRL &= ~RTC_COMPINTLVL_MED_gc;
			}
		}
		// If the new task is not the next to be executed, iterate through the task_list,
		// find its position in the linked list, and insert it there.
		else
		{
			uint8_t g = get_green_led();
			uint8_t r = get_red_led();
			uint8_t b = get_blue_led();
			set_rgb(255, 50, 0);
			Task_t* tmp_task_ptr = task_list;
			while (tmp_task_ptr->next != NULL && task->scheduled_time > (tmp_task_ptr->next)->scheduled_time)
			{
				if(tmp_task_ptr->next==tmp_task_ptr){
					printf("ERROR! Task list has self-reference.\r\n");
					printf("New Task %p (%p) scheduled at %lu with period %lu, %lu current\r\n", task, task->task_function, task->scheduled_time, task->period, get_time());
					print_task_queue();
					return;				
				}
				tmp_task_ptr = tmp_task_ptr->next;
			}
			set_rgb(r, g, b);
			task->next = tmp_task_ptr->next;
			tmp_task_ptr->next = task;
		}
//
		////Check if the front of the task_lsit should have already been run, and set the RTC.COMP for that if neessary.
		//if (task_list != NULL && task_list->scheduled_time < get_time()) //
		//{
			//while (RTC.STATUS & RTC_SYNCBUSY_bm);
			//RTC.COMP = RTC.CNT+5;
			//RTC.INTCTRL |= RTC_COMPINTLVL_MED_gc;
		//}

		num_tasks++;
	}
}

// Remove a task from the task queue
void remove_task(Task_t* task)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(task_list==NULL) return;
		if(task_list==task)
		{
			task_list=task->next;
			scheduler_free(task);
			task = NULL;
			num_tasks--;
		}
		else
		{
			Task_t* tmp_task = task_list;
			while (tmp_task->next != NULL && tmp_task->next != task) tmp_task = tmp_task->next;
			if (tmp_task->next != NULL)
			{
				tmp_task->next = task->next;
				scheduler_free(task);
				task = NULL;
				num_tasks--;
			}
		}
	}

}

void print_task_queue()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)  // Disable interrupts during printing
	{
		Task_t* cur_task = task_list;
		
		printf("Task Queue (%hhu tasks, %hhu executing):\r\n", num_tasks, num_executing_tasks);
		
		// Iterate through the list of tasks, printing name, function, and scheduled time of each
		while (cur_task != NULL)
		{
			printf("\tTask %p (%p) scheduled at %lu with period %lu, %lu current\r\n", cur_task, cur_task->task_function, cur_task->period, cur_task->scheduled_time, get_time());
			if(cur_task==cur_task->next) break;
			cur_task = cur_task->next;
		}
	}
}

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
void run_tasks()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) // Disable interrupts
	{
		// Run all tasks that are scheduled to execute in the next 2ms
		// (The RTC compare register takes 2 RTC clock cycles to update)
		while (task_list != NULL && task_list->scheduled_time <= get_time() + 2)
		{
			Task_t* cur_task = task_list;
			task_list = cur_task->next;
			NONATOMIC_BLOCK(NONATOMIC_FORCEOFF) // Enable interrupts during tasks
			{
				cur_task->task_function(cur_task->arg); // run the task
			}
			if(cur_task->period>0)
			{
				uint32_t THE_TIME = get_time();
				cur_task->scheduled_time=THE_TIME+cur_task->period;
				//(cur_task->scheduled_time)=(get_time()+(0xFFFF&((uint32_t)cur_task->period)));						
				//printf("??%lu\r\n",cur_task->scheduled_time);
				//if(cur_task->scheduled_time>0x01000000){
					//print_task_queue();
					//printf("ERROR! Scheduled time waaay in the future.\r\n");
				//}
		
				cur_task->next=NULL;
				num_tasks--;
				add_task_to_list(cur_task);
			}
			else
			{
				scheduler_free(cur_task);
				cur_task = NULL;
				num_tasks--;
			}
		}
		//If the next task to be executed was in the past, do something???
		if (task_list != NULL && task_list_check()) task_list_cleanup();
		// If the next task to be executed is in the current epoch, set the RTC compare register and interrupt		
		else if (task_list != NULL && task_list->scheduled_time <= ((((uint32_t)rtc_epoch) << 16) | (uint32_t)RTC.PER))
		{	
			while (RTC.STATUS & RTC_SYNCBUSY_bm);
			RTC.COMP = (uint16_t)(task_list->scheduled_time);
			RTC.INTCTRL |= RTC_COMPINTLVL_MED_gc;
		}
		else
		{
			RTC.INTCTRL &= ~RTC_COMPINTLVL_MED_gc;
		}
	}
	
	// Jump to the code that restores the registers to the state they were in
	// before the RTC interrupt.  Program control will return to where it was before the interrupt
	// on return from restore_registers
	asm("jmp restore_registers");	 // must include scheduler_asm.c in the project
}

// Increment rtc_epoch on RTC overflow
// Must be atomic so no reads of get_time() occur between RTC overflow and updating of epoch
ISR( RTC_OVF_vect )
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) // Disable interrupts
	{
		rtc_epoch++;
		//printf("RTC Overflow. Current time %u\n", get_time());
		//print_task_queue();

		// If the next task to run is in the current epoch, update the RTC compare value and interrupt
		if (task_list != NULL && task_list->scheduled_time < ((((uint32_t)rtc_epoch) << 16) | (uint32_t)RTC.PER))
		{
			//The next task should have already happened! Lets do it now.
			if(task_list_check()) task_list_cleanup();
			// updating RTC.COMP takes 2 RTC clock cycles, so only update the compare value and
			// interrupt if the scheduled_time is more than 2ms away
			else if (task_list->scheduled_time > get_time() + 2)
			{
				while (RTC.STATUS & RTC_SYNCBUSY_bm);
				RTC.COMP = (uint16_t)(task_list->scheduled_time);
				RTC.INTCTRL |= RTC_COMPINTLVL_MED_gc;
				return; // return from ISR
			}
			// If we get here, that means there's a task to execute in less than 2ms.  Jump to the ISR
			// to handle that.
			else
			{
				//RTC.INTFLAGS |= RTC_COMPIF_bm;
				asm("jmp rtc_compare_isr"); // must include scheduler_asm.S in the project
				return;
			}
		}
	}
}
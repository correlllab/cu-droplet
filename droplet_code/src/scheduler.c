#include "scheduler.h"

static volatile Task_t* scheduler_malloc()
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
	
	return (volatile Task_t*)0xFFFF;

}

static void scheduler_free(volatile Task_t* tgt)
{
	if((tgt<task_storage_arr)||(tgt>(&(task_storage_arr[MAX_NUM_SCHEDULED_TASKS]))))
	{
		printf_P(PSTR("ERROR: In scheduler_free, tgt (%X) was outside valid Task* range.\r\n"),tgt);
		set_rgb(0,0,255);
		delay_ms(60000);
	}
	tgt->arg = 0;
	tgt->period = 0;
	(tgt->func).noarg_function = NULL;
	tgt->scheduled_time = 0;
	tgt->next = NULL;
}

void scheduler_init()
{
	task_list = NULL;
	num_tasks = 0;
	task_executing = 0;
	for(uint8_t i=0; i<MAX_NUM_SCHEDULED_TASKS; i++) scheduler_free(&task_storage_arr[i]);
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)  // Disable interrupts during initialization
	{
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
	//OSC.RC32KCAL = PRODSIGNATURES_RCOSC32K;
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
	printf_P(PSTR("\tAttempting to restore task_list (by dropping all non-periodic tasks.\r\n\tIf you only see this message rarely, don't worry too much.\r\n"));

	volatile Task_t* cur_task = task_list;
	volatile Task_t* task_ptr_arr[MAX_NUM_SCHEDULED_TASKS];
	uint8_t num_periodic_tasks = 0;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		RTC.INTCTRL &= ~RTC_COMP_INT_LEVEL;		
		while (cur_task != NULL)
		{
			if(cur_task->period==0)
			{
				cur_task = cur_task->next;
			}
			else
			{
				cur_task->scheduled_time=get_time()+cur_task->period+50;
				task_ptr_arr[num_periodic_tasks] = cur_task;
				cur_task = cur_task->next;
				task_ptr_arr[num_periodic_tasks]->next=NULL;
				num_periodic_tasks++;					
			}
		}
		uint8_t task_is_periodic = 0;
		for(uint8_t i=0;i<MAX_NUM_SCHEDULED_TASKS;i++)
		{
			for(uint8_t j=0;j<num_periodic_tasks;j++)
			{
				if(&(task_storage_arr[i])==task_ptr_arr[j])
				{
					//printf_P(PSTR("\tSaving task %X because it is periodic.\r\n"),&(task_storage_arr[i]));
					task_is_periodic = 1;
					break;
				}
			}	
			if(!task_is_periodic)
			{
				//printf_P(PSTR("\tClearing memory of task %X.\r\n"), &(task_storage_arr[i]));
				remove_task(&(task_storage_arr[i]));
			}
			task_is_periodic = 0;
		}
		num_tasks = 0;
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
volatile Task_t* schedule_task(uint32_t time, void (*function)(), void* arg)
{	
	volatile Task_t* new_task;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		new_task = scheduler_malloc();
		if (new_task == NULL) return NULL;
		else if(new_task == ((volatile Task_t*)0xFFFF)) 
		{
			printf_P(PSTR("ERROR: No empty spot found in scheduler_malloc, but num_tasks wasn't greater than or equal max_tasks.\r\n"));
			task_list_cleanup();
		}
		else if((new_task<task_storage_arr)||(new_task>(&(task_storage_arr[MAX_NUM_SCHEDULED_TASKS-1]))))
		{
			printf_P(PSTR("ERROR: scheduler_malloc returned a new_task pointer outside of the task storage array.\r\n"));
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

volatile Task_t* schedule_periodic_task(uint32_t period, void (*function)(), void* arg)
{
	period+=MIN_TASK_TIME_IN_FUTURE*(period<MIN_TASK_TIME_IN_FUTURE);	
	volatile Task_t* new_task = schedule_task(period, function, arg);
	new_task->period=period;
	return new_task;
}

void add_task_to_list(volatile Task_t* task)
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
			if(task_executing==0)
			{			
				if (task->scheduled_time <= ((((uint32_t)rtc_epoch) << 16) | (uint32_t)RTC.PER))
				{
					while (RTC.STATUS & RTC_SYNCBUSY_bm);
					RTC.COMP = ((uint16_t)(task_list->scheduled_time))|0x8;
					RTC.INTCTRL |= RTC_COMP_INT_LEVEL;
				}
				else
				{
					RTC.INTCTRL &= ~RTC_COMP_INT_LEVEL;					
				}
			}
		}
		// If the new task is not the next to be executed, iterate through the task_list,
		// find its position in the linked list, and insert it there.
		else
		{
			//uint8_t g = get_green_led();
			//uint8_t r = get_red_led();
			//uint8_t b = get_blue_led();
			//set_rgb(255, 50, 0);
			volatile Task_t* tmp_task_ptr = task_list;
			while (tmp_task_ptr->next != NULL && task->scheduled_time > (tmp_task_ptr->next)->scheduled_time)
			{
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
void remove_task(volatile Task_t* task)
{
	if((task<task_storage_arr)||(task>(&(task_storage_arr[MAX_NUM_SCHEDULED_TASKS-1]))))
	{
		printf("ERROR: Asked to remove_task for task pointer outside the bounds of task_storage_arr.\r\n");
		return;
	}	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if(task_list==task)
		{
			task_list=task->next;
			num_tasks--;
		}
		else 
		{
			volatile Task_t* tmp_task = task_list;
			while (tmp_task->next != NULL && tmp_task->next != task) tmp_task = tmp_task->next;
			if (tmp_task->next != NULL)
			{
				tmp_task->next = task->next;
				num_tasks--;
			}
		}
		scheduler_free(task);		
	}
}

void print_task_queue()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)  // Disable interrupts during printing
	{
		volatile Task_t* cur_task = task_list;
		
		printf_P(PSTR("Task Queue (%hhu tasks, %hhu executing):\r\n"), num_tasks, task_executing);
		
		// Iterate through the list of tasks, printing name, function, and scheduled time of each
		while (cur_task != NULL)
		{
			printf_P(PSTR("\tTask %p (%p) scheduled at %lu with period %lu, %lu current\r\n"), cur_task, (cur_task->func).noarg_function, cur_task->scheduled_time, cur_task->period, get_time());
			if(cur_task==cur_task->next) break;
			cur_task = cur_task->next;
		}
	}
}

// TO BE CALLED FROM INTERRUPT HANDLER ONLY
// DO NOT CALL
int8_t run_tasks()
{
	volatile Task_t* cur_task;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) // Disable interrupts
	{
		// Run all tasks that are scheduled to execute in the next 2ms
		// (The RTC compare register takes 2 RTC clock cycles to update)
		while (task_list != NULL && task_list->scheduled_time <= get_time() + 2)
		{
			uint8_t num_slots_used = 0;
			for(uint8_t i=0;i<MAX_NUM_SCHEDULED_TASKS;i++)
			{
				if(((uint16_t)(task_storage_arr[i].func.noarg_function))!=0)
				{
					num_slots_used++;
					volatile Task_t* next_ptr = task_storage_arr[i].next;
					if((next_ptr!=0)&&((next_ptr<task_storage_arr)||(next_ptr>(&(task_storage_arr[MAX_NUM_SCHEDULED_TASKS-1])))))
					{
						printf_P(PSTR("Pre-call, task has next_ptr pointing outside of array.\r\n"));
						printf("\t%X\r\n",((uint16_t)(&(task_storage_arr[i]))));
						delay_ms(10);
					}
				}
			}
			//printf_P(PSTR("\tCalling %X. Tasks: %2hu. Slots Used: %2hu.\r\n"),cur_task->func.noarg_function, num_tasks, num_slots_used);
			if(num_slots_used!=num_tasks)
			{
				printf_P(PSTR("ERROR: Pre-call, task storage consistency check failure.\r\n"));
				return -1;
			}			
			cur_task = task_list;
			task_list = cur_task->next;

			if(cur_task->arg==NULL)
			{
				NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE) // Enable interrupts during tasks
				{
					(cur_task->func).noarg_function(); // run the task
				}
			}
			else
			{
				NONATOMIC_BLOCK(NONATOMIC_RESTORESTATE) // Enable interrupts during tasks
				{
					(cur_task->func).arg_function(cur_task->arg); // run the task
				}
			}
			
			if(cur_task->period>0)
			{
				cur_task->scheduled_time+=cur_task->period;
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
			
			num_slots_used = 0;
			for(uint8_t i=0;i<MAX_NUM_SCHEDULED_TASKS;i++)
			{
				if(((uint16_t)(task_storage_arr[i].func.noarg_function))!=0)
				{
					num_slots_used++;
					volatile Task_t* next_ptr = task_storage_arr[i].next;
					if((next_ptr!=0)&&((next_ptr<task_storage_arr)||(next_ptr>(&(task_storage_arr[MAX_NUM_SCHEDULED_TASKS-1])))))
					{
						printf_P(PSTR("Post-call, task %X has next_ptr pointing outside of array.\r\n"),task_storage_arr[i]);
						delay_ms(10);
					}
				}
			}
			//printf_P(PSTR("\tReturned %X. Tasks: %2hu. Slots Used: %2hu.\r\n"),cur_task->func.noarg_function, num_tasks, num_slots_used);
			if(num_slots_used!=num_tasks)
			{
				printf_P(PSTR("ERROR: Post-return, task storage consistency check failure.\r\n"));
				return -1;
			}			
		}
		// If the next task to be executed is in the current epoch, set the RTC compare register and interrupt		
		if (task_list != NULL && task_list->scheduled_time <= ((((uint32_t)rtc_epoch) << 16) | (uint32_t)RTC.PER))
		{	
			while (RTC.STATUS & RTC_SYNCBUSY_bm);
			RTC.COMP = ((uint16_t)(task_list->scheduled_time))|0x8;
			RTC.INTCTRL |= RTC_COMP_INT_LEVEL;
		}
		else
		{
			RTC.INTCTRL &= ~RTC_COMP_INT_LEVEL;
		}
	}
	return 0;
}

//volatile static uint16_t seen_tasks[MAX_NUM_SCHEDULED_TASKS];
//volatile static Task_t* checkup_task_ptr;

//void task_list_checkup()
//{
	//uint16_t task_mem_start = (uint16_t)(&(task_storage_arr[0]));
	//uint16_t task_mem_end = task_mem_start+MAX_NUM_SCHEDULED_TASKS*sizeof(Task_t);
	//uint16_t seen_tasks[MAX_NUM_SCHEDULED_TASKS];
	//for(uint8_t i=0;i<MAX_NUM_SCHEDULED_TASKS;i++) seen_tasks[i]=0;
	//uint8_t num_tasks_seen=0;
	//uint8_t error_occurred=0;
	//uint8_t num_slots_full=0;
	//checkup_task_ptr = task_list;
	//while(checkup_task_ptr!=NULL)
	//{
		//seen_tasks[num_tasks_seen] = (uint16_t)checkup_task_ptr;
		//num_tasks_seen++;		
		//
		//if(checkup_task_ptr<task_mem_start||checkup_task_ptr>task_mem_end){ 
			//printf_P(PSTR("TASK LIST ERROR\tAddress out of bounds?\r\n")); 
			//error_occurred=1; 
		//}
		//if(checkup_task_ptr->scheduled_time>0x01000000){			
			//printf_P(PSTR("TASK LIST ERROR\tScheduled time very large.\r\n")); 
			//error_occurred=1; 
		//}
		//if(checkup_task_ptr->period>0x01000000){
			//printf_P(PSTR("TASK LIST ERROR\tPeriod very large.\r\n")); 
			//error_occurred=1;
		//}
		//if((checkup_task_ptr->func).noarg_function==NULL){						
			//printf_P(PSTR("TASK LIST ERROR\tFunction handle is 0.\r\n")); 
		//}
		//
		//uint8_t repeated_task = 0;
		//uint8_t i;
		//for(i=0;i<num_tasks_seen;i++){
			//if(checkup_task_ptr->next==seen_tasks[i]){
				//repeated_task = 1;
				//error_occurred = 1;
				//break;
			//}
		//}
		//if(repeated_task)
		//{
			//printf_P(PSTR("Task list has a loop in it.\r\n"));
		//}
		//else
		//{
			//checkup_task_ptr = checkup_task_ptr->next;
		//}	
	//}
	////for(uint8_t i=0;i<MAX_NUM_SCHEDULED_TASKS;i++) if(((uint16_t)(task_storage_arr[i].func.noarg_function))!=0) num_slots_full++;
	////if(num_slots_full!=num_tasks)
	////{
		////printf("TASK LIST ERROR\tTask Storage Arr consistency error.\r\n");
		////error_occurred = 1;	
	////}
 	//if(error_occurred)
	//{
		//printf_P(PSTR("Attempting to print task queue.\r\n"));
		//print_task_queue();
	//}
//}

ISR(RTC_COMP_vect)
{
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
ISR( RTC_OVF_vect )
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) // Disable interrupts
	{
		rtc_epoch++;
		// If the next task to run is in the current epoch, update the RTC compare value and interrupt
		if (task_list != NULL && task_list->scheduled_time < ((((uint32_t)rtc_epoch) << 16) | (uint32_t)RTC.PER))
		{
			if(!task_executing)
			{
				if(task_list->scheduled_time < get_time())
				{
					//printf("In overflow, tasks need to have been executed!\r\n");
					//print_task_queue();
				}
				else				
				{		
					while (RTC.STATUS & RTC_SYNCBUSY_bm);
					RTC.COMP = ((uint16_t)(task_list->scheduled_time))|0x8;
					RTC.INTCTRL |= RTC_COMP_INT_LEVEL;
				}
			}
						////The next task should have already happened! Lets do it now.
			//if(task_list_check()){
				//printf("From overflow_ISR (%ld): ", get_time()-(task_list->scheduled_time));				
				//task_list_cleanup();
			//}
			//// updating RTC.COMP takes 2 RTC clock cycles, so only update the compare value and
			//// interrupt if the scheduled_time is more than 2ms away
			//else if (task_list->scheduled_time > get_time() + 2)
			//{
			//if(task_executing==0)
			//{				

			//}
			//}
			//// If we get here, that means there's a task to execute in less than 2ms.  Jump to the ISR
			//// to handle that.
			//else
			//{
				////RTC.INTFLAGS |= RTC_COMPIF_bm;
				//asm("jmp rtc_compare_isr"); // must include scheduler_asm.S in the project
			//}
		}
		else
		{
			printf("Next task not in current epoch. Task executing: %hu. Next task scheduled time: %lu. Time: %lu.\r\n", task_executing, task_list->scheduled_time, get_time());
		}
	}
}
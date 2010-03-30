#include "process_scheduler.h"
#include <stdlib.h>

process_t* kernel_process = 0;
process_s_t* current_entry = 0;

void initialize_process_scheduler(process_t * kproc) 
{
	kernel_process = kproc;

	current_entry = malloc(sizeof(process_s_t));
	memset(current_entry, 0, sizeof(process_s_t));
	current_entry->process = kproc;
	current_entry->quant = DEFAULT_PROCESS_QUANT;
	current_entry->next = current_entry; //Loop back onto itself (Think of the list as a circle =) )
}

static inline void switch_process(process_s_t * cur) {

}

void process_scheduler_tick() 
{
	current_entry->quant--;

	if (current_entry->quant < 1) 
	{
		//Process context switching
		if (current_entry->next == current_entry) 
		{
			//There is only one proc don't waste time switching context
			current_entry->quant = DEFAULT_PROCESS_QUANT;
		} else 
		{
			switch_process(current_entry);	
		}

	}
}

void list_processes_output(process_t* proc) 
{
	printf("Process %i name %s pagedir 0x%x ", proc->pid, proc->name, proc->page_dir);
	if (p_killable(proc)) {
		printf("the process is killable ");	
	} else {
		printf("the process cannot be killed ");
	}

	printf("\n");
}

void list_processes() 
{
	printf("Listing active processes\n");
	process_s_t* entry = current_entry;
	list_processes_output(entry->process);
	entry = entry->next;
	
	while(entry != current_entry) {
		list_processes_output(entry->process);
		entry = entry->next;
	} 

	printf("Done listing processes\n");
}

uint32 current_pid() {
	return current_entry->process->pid;
}

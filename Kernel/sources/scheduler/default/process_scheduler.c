#include "process_scheduler.h"
#include <panic/panic.h>
#include <stdlib.h>
#include <common.h>

struct process_entry_t
{
	process_t* process_pointer;
	unsigned int ticks_tell_die;

	struct process_entry_t* next;
};

typedef struct process_entry_t scheduler_proc;

scheduler_proc* list_root = 0;
scheduler_proc* list_current = 0;

void scheduler_init(process_t* kp)
{
	//Create and set new_process to all 0's
	scheduler_proc* new_process = malloc(sizeof(scheduler_proc));
	memset(new_process, 0, sizeof(scheduler_proc));

	//Set its process pointer to the kernels processing path
	new_process->process_pointer = kp;
	
	//Loop backs
	new_process->next = new_process;

	//Set the root and current set of the list to new_process
	list_root = new_process;
	list_current = new_process;
}

void scheduler_on_tick()
{
	if (list_current->ticks_tell_die == 0)
	{
		process_t* proc = list_current->process_pointer;

		//Swap to the next process
		list_current = list_current->next;

		process_t* oproc = list_current->process_pointer;

		//Give me a nano!
		list_current->ticks_tell_die = _STD_NANO_;

		//Swap to the new process
		switch_process(proc, oproc);
	} else
	{
		//One tick closer to being context switched
		list_current->ticks_tell_die--;
	}
}


//To anybody calling this function, remember to re-enable interrupts where applicable
void scheduler_add(process_t* op)
{
	disable_interrupts();

	//Create and set new_process to all 0's
	scheduler_proc* new_process = malloc(sizeof(scheduler_proc));
	memset(new_process, 0, sizeof(scheduler_proc));

	new_process->process_pointer = op;

	unsigned int iterator = 0;

	scheduler_proc* iterator_process = list_root;

	//Loop to find the last entry in the list
	for(;;)
	{
		if (iterator_process->next == list_root)
		{
			break;
		}
		else
		{
			iterator_process = iterator_process->next;
		}
	}

	iterator_process->next = new_process;

	new_process->next = list_root;

}

process_t* get_current_process()
{
	if (list_current == 0) return 0;

	return list_current->process_pointer;
}
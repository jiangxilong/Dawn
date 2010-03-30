#include "scheduler.h"
#include <process/process.h>

void scheduler_tick() {
	process_scheduler_tick();

	if (current_pid() == KERNEL_PID) {
		thread_schedule_tick(); //Schedule a kernel thread tick if the kernel process is in context
	}
}

#include "clock.h"
#include "pit.h"

unsigned long clock_ticks = 0;

void pit_callback()
{
	clock_ticks++;
}

void initialize_system_clock()
{
	set_pit_callback(pit_callback);
	init_pit(1000);
}

unsigned long get_clock_ticks()
{
	return clock_ticks;
}
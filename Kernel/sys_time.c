#include "sys_time.h"

//The time that has passed since January first of the year 2000
uint64 system_time = 0;

void Init_SysTime() 
{
	cmos_time_struct LTime;
	
	fill_cmos_tstruct(&LTime);
}

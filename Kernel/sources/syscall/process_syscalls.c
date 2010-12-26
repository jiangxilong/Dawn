#include <types/memory.h>
#include <process/process.h>
#include <scheduler/process_scheduler.h>

unsigned char syscallProcessValid(unsigned int pid)
{
	//Iterate untill found the correct PID
	process_t* iterator = 0;
	unsigned int number = 0;

	for (;;)
	{
		iterator = schedulerReturnProcess(number);

		if (iterator == 0) return 0;
		if (iterator->m_ID == pid) break;

		number = number + 1;
	}

	return 1;
}

int syscallGetPid(unsigned int iter)
{
	if (schedulerReturnProcess(iter) == 0) return -1;
	return schedulerReturnProcess(iter)->m_ID;
}

unsigned long syscallGetProcessingTime(unsigned int pid)
{

	//Iterate untill found the correct PID
	process_t* iterator = 0;
	unsigned int number = 0;

	for (;;)
	{
		iterator = schedulerReturnProcess(number);

		if (iterator == 0) return 0;
		if (iterator->m_ID == pid) break;

		number = number + 1;
	}

	//Return the processing time
	return iterator->m_processingTime;
}

void syscallGetName(char* StrLocation, unsigned int pid)
{	
	//Iterate untill found the correct PID
	process_t* iterator = 0;
	unsigned int number = 0;

	for (;;)
	{
		iterator = schedulerReturnProcess(number);

		if (iterator == 0)
		{
			strcpy(StrLocation, "Unknown");
			return;
		}
		if (iterator->m_ID == pid) break;

		number = number + 1;
	}

	//Copy the name into the StrLocation
	strcpy(StrLocation, iterator->m_Name);
}

void syscallKillCurrentProcess()
{
	schedulerKillCurrentProcess();
}

void syscallRequestExit(int returnValue)
{
	getCurrentProcess()->m_returnValue = returnValue;
	schedulerKillCurrentProcess();
}
#include <system/run.h>
#include <syscall/syscall.h>
DEFN_SYSCALL1(request_run_nproc, 21, char*);

void systemRunNewProcess(const char* Filename)
{
	syscall_request_run_nproc(Filename);
}

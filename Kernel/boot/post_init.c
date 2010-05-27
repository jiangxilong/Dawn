#include "post_init.h"
#include <heap/heap.h>
#include <stdio.h>
#include <system/reboot.h>
#include <common.h>
#include <timers/clock.h>
#include <input/input.h>
#include <input/mouse.h>
#include <terminal/terminal.h>
#include <process/process.h>

extern heap_t kernel_heap;
extern uint32 end; //The end of the kernel

typedef int (*cmdfunc) (void * arg);

typedef struct 
{
	char * str;
	cmdfunc function;
} cmd_list;

cmd_list cmds[256];

int ls_func (void * null) 
{
	print_directory(init_vfs(), 1);

	return 1;
}

int reboot_f(void * null) 
{
	reboot();
}

int help_f(void * null) 
{
	printf("Known functions\n");
	printf("help - this\n");
	printf("reboot - reboot the PC (May not function as desired in specific emulators\n");
	printf("ls - list every file & directory in the VFS recursively\n");
	printf("print_allocmap - print the heaps current allocation map and how much free memory there is\n");
	printf("End of known functions\n");
}

int mem_map (void * null) 
{
	//Call the heap list_chunks function
	list_chunks(&kernel_heap);

	//Calculate free pages in bytes
	extern uint32 phys_mm_slock;
	uint32 pmmt = phys_mm_slock;
	uint32 freeram = 0;

	while (pmmt > PHYS_MM_STACK_ADDR) 
	{
		pmmt -= sizeof(uint32);
		freeram += 4096; //1Kb of ram free
	}
	    
	uint32 freemb = freeram / 1024; //Bytes to Kb
	freemb = freemb / 1024; //Kb to  Mb
	printf("Unmapped RAM %u (MBs)\n", freemb);    

	return 1;
}

int cpuid_out (void * null) 
{

	if (cpuid_supported())
	{
		printf("CPUID Supported\n");
		printf("CPU Vendor: %s\n", cpuid_getvendor());
		printf("CPU Features: 0x%x\n", cpuid_features());
	}
	else
	{
		printf("CPUID not supported\n");
	}

	return 1;
}

int crash_me(void* null)
{
	uint32* FX = 0xABCDEF12;
	*FX = 3;
}

char CBuffer[1024];
int cptr = 0;

void exec_cb() 
{
	if (cptr <= 0) 
	{
		cptr = 0;
		return;
	}

	uint8 done = 0;
	CBuffer[cptr] = '\0';

	int i = 0;
	for (i = 0; i < 256; i++) 
	{
		if (strcmp(CBuffer, cmds[i].str) == 0) 
		{
			printf("\n");
			cmds[i].function(0);
			done = 1;
		}	
	}

	if (done == 0)
	{
		printf("\nError, Function %s not found\n", CBuffer);
	}

	cptr = 0;
}

void kboard_callback(uint8 cb) 
{
	disable_interrupts();

	if (cb == '\r') //Carriage return
	{ 
		exec_cb(); 
	} 
	else 
	{

		if (cb == '\b') 
		{ 
			cptr--; 
			if (cptr >= 0) 
			{ 
				CBuffer[cptr] = ' '; 
			} 
			if (cptr >= 0)
			{ 
				putc('\b'); 
				putc(' ');
				putc('\b'); 
			} 
		}
		else 
		{  
			CBuffer[cptr] = cb;
			cptr++;
			putc(cb);
		}

	}

	return;
}

void input_callback(uint32 dev, uint32 dat, void* flags)
{
	uint32 kflags = *(uint32*)flags;
	kboard_callback(keyboard_chlookup_asci(dat, kflags));
}

void mouse_callback(uint32 dev, uint32 dat, void* flags)
{
	mouse_input_t* i = (mouse_input_t*) flags;
}

void Entry(void* Arg)
{
	printf("Entry\n");
	return;
}

void Exit()
{
	printf("Exit\n");
	return;
}

void post_init() 
{

    cmds[0].str = "list_dirs";
    cmds[0].function = ls_func;

    cmds[1].str = "print_allocmap";
    cmds[1].function = mem_map;

    cmds[2].str = "reboot";
    cmds[2].function = (cmdfunc) reboot_f;

    cmds[3].str = "help";
    cmds[3].function = (cmdfunc) help_f;

    cmds[4].str = "cpuid";
    cmds[4].function = cpuid_out;

    cmds[4].str = "crash_me";
    cmds[4].function = crash_me;


    register_input_listener(DEVICE_KEYBOARD, input_callback);
    register_input_listener(DEVICE_MOUSE, mouse_callback);
    int CPS = CLOCKS_PER_SECOND;

    scheduler_init(init_kproc());
    enable_interrupts(); //Reenable interrupts shut off by copy_page_dir

    for (;;)
    {
	unsigned long long next_sec = clock() + CLOCKS_PER_SECOND;
	while (clock() < next_sec);
	unsigned int pcx = kgetcx();
	unsigned int pcy = kgetcy();
	kmovecy(0);
	kmovecx(0);
	printf("%u Ticks per second.\n%u Ticks since boot.\n", CPS, clock());
	kmovecy(pcy);
	kmovecx(pcx);
    }
}

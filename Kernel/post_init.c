#include "post_init.h"
#include <heap/heap.h>
#include <stdio.h>

extern heap_t kernel_heap;
extern uint32 end; //The end of the kernel

typedef int (*cmdfunc) (void * arg);

typedef struct {
	char * str;
	cmdfunc function;
} cmd_list;

cmd_list cmds[256];

int ls_func (void * null) 
{
	print_directory(init_vfs(), 1);

	return 1;
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

void kboard_callback(uint8 cb) {

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
}


void post_init() {
    cmds[0].str = "ls";
    cmds[0].function = ls_func;
    cmds[1].str = "print_allocmap";
    cmds[1].function = mem_map;

    init_keyboard();
    set_keyboard_callback(&kboard_callback);
}
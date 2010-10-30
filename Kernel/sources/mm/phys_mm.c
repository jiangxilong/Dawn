#include <mm/phys_mm.h>
#include <mm/virt_mm.h>
#include <debug/debug.h>
#include <multiboot.h>
#include <panic/panic.h>
#include <types/memory.h>

MEM_LOC used_mem_end = 0;

MEM_LOC phys_mm_slock = PHYS_MM_STACK_ADDR;
MEM_LOC phys_mm_smax = PHYS_MM_STACK_ADDR;

extern uint32 paging_enabled;

void init_phys_mm(MEM_LOC start) 
{
	DEBUG_PRINT("Debug Message: Used memory end 0x");
	DEBUG_PRINTX(start);
	DEBUG_PRINT("\n");

	used_mem_end = (start + 0x1000) & PAGE_MASK; //This ensures that the used_mem_end address is on a page-aligned boundry (Which it has to be if I wish to identity map from 0 to used_mem_end)
}

MEM_LOC allocateKernelFrame() 
{
	if (paging_enabled == 0) 
	{
		used_mem_end += 4096; //Add 4096 bytes (4kb) to used_mem_end address
		return used_mem_end - 4096; //Return the old address
		//The reason why this works is that all memory up to used_mem_end is identity mapped when paging is enabled
		//This meens that when paging is enabled the address will be mapped directly to the physical address (0x1000 will still access 0x1000 in memory for example) 
	} 
	else 
	{

		//Paging is enabled	
		if (phys_mm_slock == PHYS_MM_STACK_ADDR)
		{
			DEBUG_PRINT("Out of physical frames of memory\n");
			return 0; //Frame 0 should never be free
		}

		// Pop off the stack.
		phys_mm_slock -= sizeof (uint32);

		uint32 * stack = (uint32 *)phys_mm_slock;

		return *stack;
	}
}

MEM_LOC allocateFrame() 
{
	if (paging_enabled == 0) 
	{
		used_mem_end += 4096; //Add 4096 bytes (4kb) to used_mem_end address
		return used_mem_end - 4096; //Return the old address
		//The reason why this works is that all memory up to used_mem_end is identity mapped when paging is enabled
		//This meens that when paging is enabled the address will be mapped directly to the physical address (0x1000 will still access 0x1000 in memory for example) 
	} 
	else 
	{

		//Paging is enabled	
		if (phys_mm_slock == PHYS_MM_STACK_ADDR)
		{
			DEBUG_PRINT("Out of physical frames of memory\n");
			return 0; //Frame 0 should never be free
		}

		// Pop off the stack.
		phys_mm_slock -= sizeof (uint32);

		uint32 * stack = (uint32 *)phys_mm_slock;

		if (get_current_process() != 0)
		{
			used_list_add(get_current_process(), *stack);
		}

		return *stack;
	}
}

MEM_LOC allocateFrameForProcess(process_t* req_process) 
{
	if (paging_enabled == 0) 
	{
		used_mem_end += 4096; //Add 4096 bytes (4kb) to used_mem_end address
		return used_mem_end - 4096; //Return the old address
		//The reason why this works is that all memory up to used_mem_end is identity mapped when paging is enabled
		//This meens that when paging is enabled the address will be mapped directly to the physical address (0x1000 will still access 0x1000 in memory for example) 
	} 
	else 
	{

		//Paging is enabled	
		if (phys_mm_slock == PHYS_MM_STACK_ADDR)
		{
			DEBUG_PRINT("Out of physical frames of memory\n");
			return 0; //Frame 0 should never be free
		}

		// Pop off the stack.
		phys_mm_slock -= sizeof (uint32);

		uint32 * stack = (uint32 *)phys_mm_slock;

		if (req_process != 0)
		{
			used_list_add(req_process, *stack);
		}

		return *stack;
	}
}

void freeFrame(MEM_LOC frame) 
{
	if (paging_enabled == 0) 
	{
		return; //If paging isn't enabled we are not going to be able to free a frame of virtual memory and the stacks location is virtual (Cannot be accessed without paging)	
	}	

	if (frame < used_mem_end + PAGE_SIZE) 
	{
		return; //Anything under used_mem_end is identity mapped (Physical Address == Virtual Address) never remap it.
	}

	if (get_current_process() != 0)
	{
		used_list_remove(get_current_process(), frame);
	}

	if (phys_mm_smax <= phys_mm_slock) //Run out of stack space *Shock Horror* Allocate this frame to the end of the stack (Giving another 4kb (4096 bytes) of stack space)
	{
	    map ((POINTER) phys_mm_smax, (POINTER) frame, PAGE_PRESENT | PAGE_USER);
	    phys_mm_smax += PAGE_SIZE;
	
	}
	else
	{
		//We still have stack space left to pop to,
		MEM_LOC* stack = ((POINTER)phys_mm_slock);
		*stack = frame;
		phys_mm_slock += sizeof(uint32);	
	}
}

//Does the initialization of the free pages using the memory map provided by the mboot header.
void map_free_pages(struct multiboot * mboot_ptr) 
{

  uint32 i = mboot_ptr->mmap_addr;
  uint32 dbiter = 0;

  while (i < mboot_ptr->mmap_addr + mboot_ptr->mmap_length)
  {
    mmap_entry_t *me = (mmap_entry_t*) i;
    
    // Does this entry specify usable RAM?
    if (me->type == 1)
    {

      uint32 j;

      // For every page in this entry, add to the free page stack.
      for (j = me->base_addr_low; j < me->base_addr_low+me->length_low; j += 0x1000)
      {
        freeFrame(j);
	dbiter++;
      }

    }

    // The multiboot specification is strange in this respect - the size member does not include "size" itself in its calculations,
    // so we must add sizeof a 32bit int
    i += me->size + sizeof (uint32);
  }

  DEBUG_PRINT("Debug Message: Map Free Pages finished with 0x");
  DEBUG_PRINTX(dbiter);
  DEBUG_PRINT(" 4096 bytes of mapped memory\n"); 
}

extern void asm_copy_frame(uint32 src, uint32 dest);

void copy_frame(uint32 phys_addr_src, uint32 phys_addr_dest) 
{
	DEBUG_PRINT("Debug Message: Copying 0x");
	DEBUG_PRINTX(phys_addr_src);

	DEBUG_PRINT(" to 0x");
	DEBUG_PRINTX(phys_addr_dest);

	DEBUG_PRINT("\n");

	asm_copy_frame(phys_addr_src, phys_addr_dest);
}

MEM_LOC calculate_free_frames()
{
	return ((phys_mm_slock - PHYS_MM_STACK_ADDR) / sizeof(MEM_LOC));
}

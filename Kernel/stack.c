#include <stack/stack.h>
#include <mm/virt_mm.h>
#include <debug/debug.h>

//Returns the location of the new stack (The value passed in new_start) if succesful, null otherwise
//NOTE: A stack unwinds downards, so the mapping = new_start to new_start - size

stack_t move_stack(stack_t new_start, size_t size, size_t initial_esp) 
{
	new_start = ((size_t)new_start) & PAGE_MASK; //Page align it
	uint32 iter = 0; //for iterator

	DEBUG_PRINT("Debug Message: Mapping new stack between 0x");
	DEBUG_PRINTX(((size_t)new_start) - size);
	DEBUG_PRINT(" and 0x");
	DEBUG_PRINTX(new_start);
	DEBUG_PRINT("\n");

	//This for loop maps new_start to new_start - size with physical memory
	for (iter = new_start; iter >= ((size_t)new_start - size); iter -= 0x1000) 
	{
		uint32 frame = alloc_frame();
		map(iter, frame, PAGE_PRESENT | PAGE_WRITE);
	}
	
	stack_t old_stack_pointer = 0; 
	asm volatile("mov %%esp, %0" : "=r" (old_stack_pointer));

	stack_t old_base_pointer = 0;
	asm volatile("mov %%ebp, %0" : "=r" (old_base_pointer)); 

	DEBUG_PRINT("Debug Message: Old stack location 0x");
	DEBUG_PRINTX(old_stack_pointer);
	DEBUG_PRINT("\n");

	DEBUG_PRINT("Debug Message: Old stack base 0x");
	DEBUG_PRINTX(old_base_pointer);
	DEBUG_PRINT("\n");

	DEBUG_PRINT("Debug Message: Initial esp 0x");
	DEBUG_PRINTX(initial_esp);
	DEBUG_PRINT("\n");

	size_t offset = ((size_t)new_start) - initial_esp;
	DEBUG_PRINT("Debug Message: Calculated stack offset 0x");
	DEBUG_PRINTX(offset);
	DEBUG_PRINT("\n");

	stack_t new_stack_pointer = ((size_t)old_stack_pointer) + offset;
	stack_t new_base_pointer  = ((size_t)old_base_pointer)  + offset; 

	//Copy over the new stack
	memcpy((void*)new_stack_pointer, (void*)old_stack_pointer, initial_esp -  ((size_t)old_stack_pointer)); 

	size_t i = 0;

	for(i = (size_t) new_start; i > (size_t) new_start-size; i -= sizeof(uint32))
	{
	   size_t tmp = * (size_t* )i;
	   // If the value of tmp is inside the range of the old stack, assume it is a base pointer
	   // and remap it. This will unfortunately remap ANY value in this range, whether they are
	   // base pointers or not.
	   if (( old_stack_pointer < tmp) && (tmp < initial_esp))
	   {
	     tmp = tmp + offset;
	     size_t *tmp2 = (size_t*)i;
	     *tmp2 = tmp;
	   }
	}

	asm volatile("mov %0, %%esp" : : "r" (new_stack_pointer));
	asm volatile("mov %0, %%ebp" : : "r" (new_base_pointer));

	DEBUG_PRINT("Debug Message: Done moving the stack\n");

	return new_start;
}

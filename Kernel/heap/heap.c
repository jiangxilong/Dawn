#include <heap/heap.h>
#include <printf.h>
#include <debug/debug.h>


uint32 alloc_mem(uint32 size, heap_t * heap);
void free_mem(uint32 address, heap_t * heap);

void list_chunks(heap_t * heap) //List all active chunks in a heap. 
{
	heap_entry_t * ptr = (heap_entry_t *) heap->heap_location;

	while (ptr) 
	{
		printf("Chunk %x size %i used %i\n", ptr, ptr->size, ptr->used);
		ptr = (heap_entry_t *) ptr->next;
	}
}

//Does memory mapping itself, no need to map the first frame of the heap - and if you do you will lose memory to the void.
void init_heap(heap_t * heap, uint32 address) 
{
	//Double check the heap ptr is valid
	if (heap == 0) return;

	DEBUG_PRINT("Debug Message: Creating Heap\n");
	memset(heap, 0, sizeof(heap_t));
	DEBUG_PRINT("Debug Message: 0'ed Heap\n");

	//Set the heap location to the specified address
	heap->heap_location = address & PAGE_MASK; //Page aligned boundry
	DEBUG_PRINT("Debug Message: Heap Location ");
	DEBUG_PRINTX(heap->heap_location);
	DEBUG_PRINT("\n");

	uint32 frame_addr = 0;

	frame_addr = 0;

	frame_addr = alloc_frame();

	DEBUG_PRINT("Debug Message: Allocated frame ");
	DEBUG_PRINTX(frame_addr);
	DEBUG_PRINT("\n");

	map(heap->heap_location, frame_addr, PAGE_PRESENT | PAGE_WRITE);

	DEBUG_PRINT("Debug Message: Mapped Heap\n");

	memset(heap->heap_location, 0, 4096);
	DEBUG_PRINT("Debug Message: 0'ed HEAP\n");

	heap_entry_t * ptr = (heap_entry_t *) heap->heap_location;
	memset(ptr, 0, sizeof(heap_entry_t)); //Initialize it.
	DEBUG_PRINT("Debug Message: Created first heap entry\n");

	ptr->used = 0; //Not used
	ptr->size = 4096 - sizeof(heap_entry_t); //One frame has been allocated. 4096 bytes of heap space at the minute. The heap entrys are also in the space so we need to take into account for them (Only one at the minute though =) )

	DEBUG_PRINT("Debug Message: Set ptr size\n");

	ptr->next = 0; //ptr->next = NULL, this tells the heap that this is the last entry on the heap.
	ptr->prev = 0;
	DEBUG_PRINT("Debug Message: Ptr->next set\n");
}

uint32 alloc_mem(uint32 size, heap_t* heap) 
{
	heap_entry_t* ptr = (heap_entry_t*) heap->heap_location;
	uint8 result = 0;

	while (1) 
	{

		if (ptr->used == 0) //Is the chunk in use?
		{ 
			if (ptr->size > size) //Is it big enough!?!
			{ 
				ptr->used = 1; //Yay it fits!
				
				if (ptr->size < size + (sizeof(heap_entry_t))) 
				{ //If another chunk of memory wont fit then just allocate a few more bytes then asked for (Avoid leak)
					//Don't actually need to do anything. The next ptr is still the next pointer
				} 
				else 
				{
					heap_entry_t* newptr = (heap_entry_t*) (((uint32) ptr) + sizeof(heap_entry_t) + size);
					newptr->used = 0;
					newptr->size = ptr->size - size - sizeof(heap_entry_t);
					newptr->next = ptr->next;
					newptr->prev = ptr;
					ptr->next = newptr;
					ptr->size = size;
				}

				return ((uint32) ptr) + sizeof(heap_entry_t); //Return the location of the actual memory not the header
			}
		}

		if (ptr->next == 0) 
		{
			//Map another 4kb onto the heap
			//This is the last entry on the heap SO if its not used we can just += the size otherwise we have to create a whole new one (More work for mee)
			uint32 frame_addr = alloc_frame();
			uint32 end = ((uint32) ptr) + ptr->size + sizeof(heap_entry_t);

			map(end, frame_addr, PAGE_PRESENT | PAGE_WRITE); //Mapped now! =)
			memset(end, 0, 4096);
			
			if (ptr->used == 0) 
			{
				ptr->size += 4096;
			} 
			else 
			{
				//Create a new entry at end and link it to the heap
				heap_entry_t * nptr = (heap_entry_t *) end;
				nptr->used = 0;
				nptr->size = 4096;
				nptr->prev = ptr;
				nptr->next = 0;
				ptr->next = nptr;
			}

			return alloc_mem(size, heap);
		}

		ptr = (heap_entry_t *) ptr->next; //Move onto the next chunk
	}

	return 0;
}

void free_mem(uint32 address, heap_t * heap)
{
	heap_entry_t * ptr = (heap_entry_t *) (address - sizeof(heap_entry_t));
	heap_entry_t * optr = 0;

	if (ptr->used == 0) //If this happens then somethings gone wrong. Wtf moment much?
	{
		return;
	}

	ptr->used = 0;

	while (1)
	{
		//UNIFY Left
		optr = ptr->prev;
		if (optr != 0) 
		{
			if (optr->used == 0) //Can be unified!
			{ 
				optr->size = optr->size + sizeof(heap_entry_t) + ptr->size;
				optr->next = ptr->next;
				ptr = optr;
			}
			else
			{
				break;
			}
		} 
		else 
		{
			break;
		}
	}

	while (1) 
	{
		//UNIFY Right
		optr = ptr->next;
		if (optr != 0) 
		{
			if (optr->used == 0) //Can be unified!
			{ 
				ptr->size = ptr->size + sizeof(heap_entry_t) + optr->size;
				ptr->next = optr->next;
			} 
			else 
			{ 
				break; 
			}
		} 
		else 
		{ 
			break; 
		}
	}

	//Check if this is the last entry
	if (ptr->next == 0) 
	{
		//TODO: Shrinking the heap!
	}
}
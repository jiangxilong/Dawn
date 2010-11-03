#include <heap/heap.h>
#include <printf.h>
#include <debug/debug.h>
#include <panic/panic.h>
#include <mm/virt_mm.h>
#include <mm/phys_mm.h>

MEM_LOC heapAllocateMemory(size_t size, heap_t* heap);

#define HEAP_BASE_PAGES 4
#define MINIMUM_BLOCK_SIZE 100

size_t mapInitialHeap(MEM_LOC start)
{
	MEM_LOC iter = start;
	for (iter; iter <= start + PAGE_SIZE * HEAP_BASE_PAGES; iter += PAGE_SIZE)
	{
		map(iter, allocateKernelFrame(), PAGE_PRESENT | PAGE_WRITE);
	}

	return HEAP_BASE_PAGES * PAGE_SIZE;
}

void initializeBaseEntry(heap_entry_t* base_entry, size_t heapSizeInBytes)
{
	memset(base_entry, 0, sizeof(heap_entry_t));
	base_entry->magic = HEAP_MAGIC;
	base_entry->size = heapSizeInBytes - sizeof(heap_entry_t);
	base_entry->next = 0;
	base_entry->prev = 0;
}

/**
 * @brief Initializes a new heap at address specified and maps memory from free frames for it
 * @callgraph
 */
void initializeHeap(heap_t* heap, MEM_LOC address)
{
	//Double check the heap ptr is valid
	if (heap == 0) return;

	DEBUG_PRINT("Initializing new heap at 0x%x\n", address);

	//Returns the number of bytes the heap allocated on initialization
	size_t heapSizeBytes = mapInitialHeap(address);

	DEBUG_PRINT("Assigning base address\n");	

	heap_entry_t* base_entry = address;
	initializeBaseEntry(base_entry, heapSizeBytes);

	DEBUG_PRINT("Setting heap->heap_location to address\n");

	heap->heap_location = address;

	return;
}

/**
 * @brief Expands the heap by expansionSize pages
 * @callgraph
 */
void expandHeap(heap_entry_t* last_heap_entry, MEM_LOC heap_end, size_t expansionSize)
{
	MEM_LOC iter = heap_end;

	for (iter; iter <= heap_end + (PAGE_SIZE * expansionSize); iter += PAGE_SIZE)
	{
		map(iter, allocateKernelFrame(), PAGE_PRESENT | PAGE_WRITE);
	}

	//If the last entry is still free
	if (last_heap_entry->used == 0)
	{
		last_heap_entry->size += (expansionSize * PAGE_SIZE);
	}
	else //If not. create a new entry and link it too the list
	{
		heap_entry_t* new_entry = heap_end;
		memset(new_entry, 0, sizeof(heap_entry_t));
		
		new_entry->magic = HEAP_MAGIC;
		new_entry->used = 0;
		new_entry->prev = last_heap_entry;
		new_entry->next = 0;
		new_entry->size = (expansionSize * PAGE_SIZE) - sizeof(heap_entry_t);

		last_heap_entry->next = new_entry;
	}

	return;
}

/**
 * @brief Allocates x bytes of memory from a heap, expanding it if necessary
 * @callgraph
 * @bug Heap never decreases in size
 */
MEM_LOC heapAllocateMemory(size_t size, heap_t* heap) 
{
	heap_entry_t* iterator = (heap_entry_t*) heap->heap_location;

	//Loop through all heap entrys
	while (iterator != 0)
	{
		//If it isn't used and its bigger then the size requested
		if ((iterator->used == 0) && (iterator->size >= size))
		{
			heap_entry_t* used_block = iterator;

			used_block->used = 1;

			size_t remainder = used_block->size - size; //How much is left over

			if (remainder > sizeof(heap_entry_t) + MINIMUM_BLOCK_SIZE)
			{
				//Create a new entry and reduce the size of the original

				//The new entry
				heap_entry_t* new_block = ((MEM_LOC) used_block) + sizeof(heap_entry_t) + size;
				memset(new_block, 0, sizeof(heap_entry_t));
				new_block->magic = HEAP_MAGIC;
				new_block->size = remainder - sizeof(heap_entry_t);
				new_block->next = used_block->next;
				new_block->prev = used_block;


				//The used entry
				used_block->size = size;
				used_block->next = new_block;
			}
			else
			{
			}

			MEM_LOC returnLocation = used_block;
			returnLocation += sizeof(heap_entry_t);

			return returnLocation;
		}
		else
		{
		}

		iterator = iterator->next;
		
	}

	//This bit of code sets iterator to the last valid entry on the heap
	iterator = (heap_entry_t*) heap->heap_location;

	for (;;)
	{
		if (iterator->next == 0) break;
		iterator = iterator->next;
	}

	//This bit finds the end of the heap
	MEM_LOC heap_end = iterator;
	heap_end += sizeof(heap_entry_t) + iterator->size;

	//Expands the heap by x pages
	expandHeap(iterator, heap_end, 2);

	//Recall this function
	return heapAllocateMemory(size, heap);
}


/**
 * @brief If there are 2 unused entrys next to each other, this function turns them into one entry
 */
void unifyHeapEntry(heap_entry_t* entry)
{

	if (entry->prev != 0)
	{

		if (entry->prev->used == 0)
		{
			//Can unify left
			heap_entry_t* new_entry = entry->prev;
			heap_entry_t* engulfed = entry;

			//Add the two sizes and we regain one heap_entry_t size of bytes as one header is finito
			new_entry->size = new_entry->size + engulfed->size + sizeof(heap_entry_t);
			new_entry->next = engulfed->next;

			unifyHeapEntry(new_entry);
			return;
		
		}

	}

	if (entry->next != 0)
	{

		if (entry->next->used == 0)
		{
			//Can unify right

			heap_entry_t* new_entry = entry;
			heap_entry_t* engulfed = entry->next;

			//Add the two sizes and we regain one heap_entry_t size of bytes as one header is finito
			new_entry->size = new_entry->size + engulfed->size + sizeof(heap_entry_t);
			new_entry->next = engulfed->next;

			unifyHeapEntry(new_entry);
			return;
		}

	}

}

/**
 * @brief Free's the memory at location address on the heap specified. Shrinking if necessary
 * @callgraph
 */
void heapFreeMemory(MEM_LOC address, heap_t* heap)
{
	MEM_LOC entry_address = address - sizeof(heap_entry_t);

	heap_entry_t* specificEntry = (heap_entry_t*) entry_address;

	if (specificEntry->magic != HEAP_MAGIC)
	{
		DEBUG_PRINT("Entry magic != Heap magic\n");
		PANIC("AHHHH HEAP_MAGIC error\n");
	}
	else
	{
		specificEntry->used = 0;
		//unifyHeapEntry(specificEntry);
	}

	return;
}

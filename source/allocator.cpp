/*
	MEMORY.C
	--------
*/
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"

namespace JASS
	{
	/*
		ALLOCATOR::ALLOCATOR()
		----------------------
	*/
	allocator::allocator(size_t block_size_for_allocation)
		{
		block_size = block_size_for_allocation;
		current_chunk = NULL;

		rewind();		// set up ready for the initial allocation
		}

	/*
		ALLOCATOR::~ALLOCATOR()
		-----------------------
	*/
	allocator::~allocator()
		{
		rewind();		// free the all in-use memory (if any)
		}

	/*
		ALLOCATOR::ADD_CHUNK()
		----------------------
		The bytes parameter is passed to this routine simply so that we can be sure to
		allocate at least that number of bytes.
	*/
	allocator::chunk *allocator::add_chunk(size_t bytes)
		{
		/*
			If we get asked for a block larger than the current block_size then assume we're going to see more
			such requests so make the block_size larger from now on.
		*/
		if (bytes > block_size)
			block_size = bytes;			// Extend the largest allocate block size

		size_t request = block_size + sizeof(allocator::chunk);

		/*
			Get a new block of memory for the C++ free store of the Operating System
		*/
		allocator::chunk *chain;
		if ((chain = (allocator::chunk *)alloc(request)) == NULL)
			return NULL;					// This can rarely happen because of delayed allocation strategies of Linux (etc.).


		/*
			Create the linked list of large blocks of memory.
		*/
		chain->next_chunk = current_chunk;
		chain->chunk_size = request;
		
		/*
			Make thia chunk the top of the linked list and set it up ready for use
		*/
		current_chunk = chain;
		chunk_at = current_chunk->data;								// this is the start of the data part of the chunk.
		chunk_end = ((uint8_t *)current_chunk) + request;		// this is the end of the allocation unit so we can ignore padding the chunk objects.

		/*
			Update global statistics
		*/
		allocated += request;

		return current_chunk;
		}

	/*
		ALLOCATOR::REALIGN()
		--------------------
	*/
	void allocator::realign(void)
		{
		/*
			Get the current pointer as an integer
		*/
		uintptr_t current_pointer = (uintptr_t)chunk_at;
		
		/*
			Compute the amountof padding that is needed to pad to a boundary of size sizeof(void *) (i.e. 4 bytes on a 32-bit machine of 8 bytes on a 64-bit machine)
		*/
		size_t padding = (current_pointer % sizeof(void *) == 0) ? 0 : sizeof(void *) - current_pointer % sizeof(void *);

		/*
			This might overflow if we're at the end of a block but that doesn't matter because the next call to malloc()
			will notice that the overflow and allocate a new block.
		*/
		chunk_at += padding;
		
		/*
			In the case of overflow its necessary to avoid having more "used" than "allocated" so a check is needed here to
			keep the two in sync
		*/
		if (chunk_at < chunk_end)
			used += padding;
		else
			used += padding - (chunk_end - chunk_at);
		}

	/*
		ALLOCATOR::REWIND()
		-------------------
	*/
	void allocator::rewind(void)
		{
		/*
			Free all memory blocks
		*/
		chunk *killer;
		for (chunk *chain = current_chunk; chain != NULL; chain = killer)
			{
			killer = chain->next_chunk;
			dealloc(chain);
			}

		/*
			zero the counters and pointers
		*/
		chunk_end = chunk_at = NULL;
		current_chunk = NULL;
		used = 0;
		allocated = 0;
		}
		
	/*
		ALLOCATOR::UNITTEST()
		---------------------
	*/
	 void allocator::unittest(void)
		{
		allocator memory;
		
		/*
			Should be empty at the start
		*/
		assert(memory.size() == 0);
		assert(memory.capacity() == 0);
		
		/*
			Allocate some memory
		*/
		uint8_t *block = (uint8_t *)memory.malloc(431);
		assert(memory.size() == 431);
		assert(memory.capacity() != 0);
		
		/*
			write to the memory chunk (this will segfault if we got it all wrong)
		*/
		::memset(block, 1, 431);
		
		/*
			Re-allign the allocator to a word boundary
		*/
		memory.realign();
		assert(memory.size() == 432);
		assert(memory.capacity() != 0);
		
		/*
			free up all the memory
		*/
		memory.rewind();
		assert(memory.size() == 0);
		assert(memory.capacity() == 0);
	
		/*
			Yay, we passed
		*/
		puts("allocator::PASSED");
		}
}

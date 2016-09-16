/*
	ALLOCATOR_POOL.CPP
	------------------
*/
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "allocator_pool.h"

namespace JASS
	{
	/*
		ALLOCATOR_POOL::ALLOCATOR_POOL()
		--------------------------------
	*/
	allocator_pool::allocator_pool(size_t block_size_for_allocation)
		{
		block_size = block_size_for_allocation;
		current_chunk = nullptr;

		rewind();		// set up ready for the initial allocation
		}

	/*
		ALLOCATOR_POOL::~ALLOCATOR_POOL()
		---------------------------------
	*/
	allocator_pool::~allocator_pool()
		{
		rewind();		// free the all in-use memory (if any)
		}

	/*
		ALLOCATOR_POOL::ADD_CHUNK()
		---------------------------
		The bytes parameter is passed to this routine simply so that we can be sure to
		allocate at least that number of bytes.
	*/
	allocator_pool::chunk *allocator_pool::add_chunk(size_t bytes)
		{
		/*
			If we get asked for a block larger than the current block_size then assume we're going to see more
			such requests so make the block_size larger from now on.
		*/
		if (bytes > block_size)
			block_size = bytes;			// Extend the largest allocate block size

		size_t request = block_size + sizeof(allocator_pool::chunk);

		/*
			Get a new block of memory for the C++ free store of the Operating System
		*/
		allocator_pool::chunk *chain;
		if ((chain = (allocator_pool::chunk *)alloc(request)) == nullptr)
			return nullptr;					// This can rarely happen because of delayed allocation strategies of Linux (etc.).


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
		ALLOCATOR_POOL::REALIGN()
		-------------------------
	*/
	void allocator_pool::realign(void)
		{
		/*
			Get the current pointer as an integer
		*/
		uintptr_t current_pointer = (uintptr_t)chunk_at;
		
		/*
			Compute the amount of padding that is needed to pad to a boundary of size sizeof(void *) (i.e. 4 bytes on a 32-bit machine of 8 bytes on a 64-bit machine)
		*/
		size_t padding = (current_pointer % alignment_boundary == 0) ? 0 : alignment_boundary - current_pointer % alignment_boundary;

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
		ALLOCATOR_POOL::MALLOC()
		------------------------
	*/
	void *allocator_pool::malloc(size_t bytes)
		{
		void *answer;			// Pointer to the allocated space.

		/*
			ARM requires all memory reads to be word-aligned.  On some ARM there is a hardware flag to let the Operating System
			catch the alignment fault and manage it, but that can take considerable time to execute.  So, on ARM all memory
			allocations are automatically word-aligned.
		*/
		#ifdef __arm__
			realign();
		#endif

		/*
			If we can allocate out of the current chunk then use it, otherwise allocate a new chunk, update the list, update pointers, and be ready to be used
		*/
		if (chunk_at + bytes > chunk_end)
			if (add_chunk(bytes) == nullptr)
				{
//				#ifdef NEVER
					exit(printf("file:%s line:%d: Out of memory:%lld bytes requested %lld bytes used %lld bytes allocated.\n",  __FILE__, __LINE__, (long long)bytes, (long long)used, (long long)allocated));
//				#endif
				return nullptr;		// out of memory
				}

		/*
			The current chunk is now guaranteed to be large enough to allocate from, so we do so.
		*/
		answer = chunk_at;
		chunk_at += bytes;
		used += bytes;

		/*
			Done
		*/
		return answer;
		}
		
	/*
		ALLOCATOR_POOL::REWIND()
		------------------------
	*/
	void allocator_pool::rewind(void)
		{
		/*
			Free all memory blocks
		*/
		chunk *killer;
		for (chunk *chain = current_chunk; chain != nullptr; chain = killer)
			{
			killer = chain->next_chunk;
			dealloc(chain);
			}

		/*
			zero the counters and pointers
		*/
		chunk_end = chunk_at = nullptr;
		current_chunk = nullptr;
		used = 0;
		allocated = 0;
		}

	/*
		ALLOCATOR_POOL::UNITTEST()
		--------------------------
	*/
	 void allocator_pool::unittest(void)
		{
		allocator_pool memory;
		
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
		puts("allocator_pool::PASSED");
		}
}

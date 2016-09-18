/*
	ALLOCATOR_POOL.CPP
	------------------
*/
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "maths.h"
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
		bool success;
		size_t request;
		
		/*
			work out the size of the request to the C++ free store or Operating System.
		*/
		request = maths::max(block_size, bytes) + sizeof(allocator_pool::chunk);

		/*
			Get a new block of memory for the C++ free store or the Operating System
		*/
		chunk *chain;
		if ((chain = (allocator_pool::chunk *)alloc(request)) == nullptr)
			return nullptr;					// This can rarely happen because of delayed allocation strategies of Linux and other Operating Systems.

		/*
			Initialise the chunk
		*/
		chain->next_chunk = current_chunk;
		chain->chunk_size = request;
		chain->chunk_at = chain->data;										// this is the start of the data part of the chunk.
		chain->chunk_end = ((uint8_t *)chain) + request;		// this is the end of the allocation unit so we can ignore padding the chunk objects.

		/*
			Place this chunk at the top of the list (if it hasn't been updated by a different thread in the mean time)
		*/
		success = current_chunk.compare_exchange_strong(chain->next_chunk, chain);
		if (success)
			allocated += request;		// update the global statistics
		else
			dealloc(chain);				// failed to add to the list because someone else already did!

		return current_chunk;			// some non-nullptr value
		}

	/*
		ALLOCATOR_POOL::MALLOC()
		------------------------
	*/
	void *allocator_pool::malloc(size_t bytes, size_t alignment)
		{
		uint8_t *top_of_stack;
		uint8_t *new_top_of_stack;
		size_t padding = 0;
		bool success = false;
		chunk *chunk;

		do
			{
			chunk = current_chunk;
				
			/*
				Get a pointer to the next free byte
			*/
			if (chunk == nullptr)
				top_of_stack = nullptr;
			else
				top_of_stack = chunk->chunk_at;

			/*
				Align the block if requested to do so
			*/
			if (alignment != 1)
				padding = realign(top_of_stack, alignment);
			
			/*
				If we can allocate out of the current chunk then use it, otherwise allocate a new chunk, update the list, update pointers, and be ready to be used
			*/
			if (chunk == nullptr || top_of_stack + bytes + padding > chunk->chunk_end)
				{
				if (add_chunk(bytes) != nullptr)
					continue;
				else
					{
	//				#ifdef NEVER
						exit(printf("file:%s line:%d: Out of memory:%lld bytes requested %lld bytes used %lld bytes allocated.\n",  __FILE__, __LINE__, (long long)bytes, (long long)used, (long long)allocated));
	//				#endif
					return nullptr;		// out of memory
					}
				}

			/*
				The current chunk is now guaranteed to be large enough to allocate from, so we do so.
			*/
			new_top_of_stack = top_of_stack + bytes + padding;
			success = chunk->chunk_at.compare_exchange_strong(top_of_stack, new_top_of_stack);
			}
		while (!success);
		
		used += bytes;

		/*
			Done
		*/
		return top_of_stack + padding;
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

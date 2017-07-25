/*
	ALLOCATOR_POOL.CPP
	------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <array>
#include <thread>
#include <iostream>

#include "maths.h"
#include "assert.h"
#include "dynamic_array.h"
#include "allocator_pool.h"

namespace JASS
	{
	/*
		ALLOCATOR_POOL::ALLOCATOR_POOL()
		--------------------------------
	*/
	allocator_pool::allocator_pool(size_t block_size_for_allocation) :
		block_size(block_size_for_allocation),
		current_chunk(nullptr)
		{
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
		bool success;				// was the compate_exchange successful?
		size_t request;			// the amount of memory that is going to be allocated
		
		/*
			work out the size of the request to the C++ free store or Operating System.
		*/
		request = maths::maximum(block_size, bytes) + sizeof(allocator_pool::chunk);

		/*
			Get a new block of memory for the C++ free store or the Operating System
		*/
		chunk *chain;
		if ((chain = (allocator_pool::chunk *)alloc(request)) == nullptr)
			return nullptr; //	LCOV_EXCL_LINE		// This can rarely happen because of delayed allocation strategies of Linux and other Operating Systems.

		/*
			Initialise the chunk
		*/
		chain->next_chunk = current_chunk.load();
		chain->chunk_size = request;
		chain->chunk_at.store(chain->data);										// this is the start of the data part of the chunk.
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
#ifdef USE_CRT_MALLOC
		/*
			If USE_CRT_MALLOC is defined then we use the C Runtime Library's malloc.  This is helpful
			when using memory checkers such as Valgrind.
		*/
		void *allocation = ::malloc(bytes);
		used = allocated += bytes;

		std::lock_guard<std::mutex> critical_section(mutex);

		crt_malloc_list.push_back(allocation);

		return allocation;
#else
		uint8_t *top_of_stack;					// The current free pointer
		uint8_t *new_top_of_stack;				// Where the free pointer will be on success
		size_t padding = 0;						// How much padding is needed to correctly align the memory
		bool success = false;					// Success or failure of the compare_and_exchange
		chunk *chunk;								// The current top chunk

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
						exit(printf("file:%s line:%d: Out of memory:%lld bytes requested %lld bytes used %lld bytes allocated.\n",  __FILE__, __LINE__, (long long)bytes, (long long)used, (long long)allocated));	// LCOV_EXCL_LINE
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
#endif
		}
		
	/*
		ALLOCATOR_POOL::REWIND()
		------------------------
	*/
	void allocator_pool::rewind(void)
		{
#ifdef USE_CRT_MALLOC
		for (auto &block : crt_malloc_list)
			free(block);
		used = 0;
		allocated = 0;
		crt_malloc_list.clear();
#else
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
#endif
		}

	/*
		ALLOCATOR_POOL::UNITTEST_THREAD()
		---------------------------------
	*/
	 void allocator_pool::unittest_thread(dynamic_array<uint8_t *> &answer, allocator_pool &memory, uint8_t bytes)
		{
		/*
			Allocate bytes of memory.
		*/
		uint8_t *block = (uint8_t *)memory.malloc(bytes);

		/*
			Write that valuye into the memory (for later checking).
		*/
		for (uint8_t *byte = block; byte < block + bytes; byte++)
			*byte = bytes;

		answer.push_back(block);
		}

	/*
		ALLOCATOR_POOL::UNITTEST()
		--------------------------
	*/
	 void allocator_pool::unittest(void)
		{
		allocator_pool memory(default_allocation_size);
		
		/*
			Should be empty at the start
		*/
		JASS_assert(memory.size() == 0);
		JASS_assert(memory.capacity() == 0);
		
		/*
			Allocate some memory
		*/
		uint8_t *block = (uint8_t *)memory.malloc(431);
		JASS_assert(memory.size() == 431);
		JASS_assert(memory.capacity() != 0);
		
		/*
			write to the memory chunk (this will segfault if we got it all wrong)
		*/
		::memset(block, 1, 431);
		
		/*
			Re-allign the allocator to a word boundary
		*/
		memory.malloc(1, sizeof(void *));
		JASS_assert(memory.size() == 432);
		JASS_assert(memory.capacity() != 0);

		/*
			check the comparison operators
		*/
		allocator_pool second(default_allocation_size);
		JASS_assert(memory == memory);
		JASS_assert(memory != second);

		/*
			free up all the memory
		*/
		memory.rewind();
		JASS_assert(memory.size() == 0);
		JASS_assert(memory.capacity() == 0);


		/*
			Thread test this class.  This calls a seperate routine 255 times, each allocates n bytes of memory
			and fills it will that value, returning the pointer in a dynamic_array for checking.
		*/
		constexpr size_t thread_count = 0xF0;
		std::array<std::thread, thread_count> thread_pool;			// a thread pool used to ensure all the threads are done.
		dynamic_array<uint8_t *> pointer_set(memory);			// keeo a list of pointers for later checking

		/*
			Start each thread
		*/
		for (uint8_t instance = 1; instance < thread_count; instance++)
			thread_pool[instance] = std::thread(&allocator_pool::unittest_thread, std::ref(pointer_set), std::ref(memory), instance);

		/*
			Wait until they are all done
		*/
		for (size_t instance = 1; instance < thread_count; instance++)
			thread_pool[instance].join();

		/*
			Now check that it worked
		*/
		uint8_t hit[thread_count + 1] = {0};									// count the number of allocatsions of the given index (should be 1).
		for (const auto &pointer : pointer_set)
			{
			uint8_t *from = pointer;
			uint8_t value = *from;
			for (uint8_t byte = 0; byte <  value; byte++)					// there should be n bytes of value n
				{
				JASS_assert(*from == value);
				from++;
				}
			hit[value]++;
			}
		for (size_t instance = 1; instance < thread_count; instance++)	// everyr value of n should exist
			JASS_assert(hit[instance] == 1);


		/*
			Yay, we passed
		*/
		puts("allocator_pool::PASSED");
		}
	}

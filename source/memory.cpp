/*
	MEMORY.C
	--------
*/
#ifdef _MSC_VER
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "memory.h"

namespace JASS
	{
	/*
		MEMORY::MEMORY()
		----------------
	*/
	memory::memory(size_t block_size_for_allocation)
		{
		block_size = block_size_for_allocation;
		current_chunk = NULL;

		rewind();		// set up ready for the initial allocation
		}

	/*
		MEMORY::~MEMORY()
		-----------------
	*/
	memory::~memory()
		{
		rewind();		// free the all in-use memory (if any)
		}

	/*
		MEMORY::ALLOC()
		---------------
		Allocate memory from the operating system (in this case the C runtime library).
	*/
	void *memory::alloc(size_t size)
		{
		return ::malloc((size_t)size);
		}

	/*
		MEMORY::DEALLOC()
		-----------------
	*/
	void memory::dealloc(void *buffer)
		{
		::free(buffer);
		}

	/*
		ANT_MEMORY::GET_CHAINED_BLOCK()
		-------------------------------
		The bytes parameter is passed to this routine simply so that we can be sure to
		allocate at least that number of bytes.
	*/
	void *ANT_memory::get_chained_block(long long bytes)
	{
	char **chain;
	long long request, *size;

	#ifdef PURIFY
		request = bytes + sizeof(*chain) + sizeof(bytes);
	#else
		if (bytes > block_size)
			block_size = bytes;			// extend the largest allocate block size

		request = block_size + sizeof(*chain) + sizeof(bytes);
	#endif

	if ((chain = (char **)alloc(&request)) == NULL)
		return NULL;

	/*
		Create a linked list of large blocks of memory.
		the first sizeof(char *) bytes are the pointer to the previous block
		whereas the next sizeof{bytes) bytes are the size of this block
	*/
	*chain = chunk;
	chunk = (char *)chain;
	size = (long long *)(chunk + sizeof(*chain));
	*size = bytes;

	/*
		Mark the current location we are to allocate from (which we call at)
		and the end of the block (chunk_end)
	*/
	at = chunk + sizeof(*chain) + sizeof(bytes);
	chunk_end = chunk + request;		// request is the amount it allocated (which might be more than we asked for)

	/*
		This is the amount we have allocated in total
	*/
	allocated += request;

	return chunk;
	}

	/*
		MEMORY::REALIGN()
		-----------------
	*/
	void memory::realign(void)
	{
	size_t padding;

	padding = (used % sizeof(void *) == 0) ? 0 : sizeof(void *) - used % sizeof(void *);

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
		ANT_MEMORY::REWIND()
		--------------------
	*/
	void ANT_memory::rewind(void)
	{
	char *chain, *killer;

	/*
		Free all memory blocks
	*/
	for (chain = chunk; chain != NULL; chain = killer)
		{
		killer = *(char **)chain;
		dealloc(chain);
		}

	/*
		zero the counters and pointers
	*/
	chunk_end = at = chunk = NULL;
	used = 0;
	allocated = 0;
	}
}
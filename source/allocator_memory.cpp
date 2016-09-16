/*
	ALLOCATOR_MEMORY.CPP
	--------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "allocator_memory.h"

namespace JASS
	{
	/*
		ALLOCATOR_MEMORY::MALLOC()
		--------------------------
	*/
	void *allocator_memory::malloc(size_t bytes)
		{
		/*
			If we don't have capacity to succeed then fail
		*/
		if (used + bytes > buffer_size)
			{
//			#ifdef NEVER
				exit(printf("file:%s line:%d: Out of memory:%lld bytes requested %lld bytes used %lld bytes allocated of %lld bytes available.\n", __FILE__, __LINE__, (long long)bytes, (long long)used, (long long)allocated, (long long)buffer_size));
//			#endif
			return nullptr;
			}

		/*
			Take the current top of stack, and mark the requested bytes are used
		*/
		void *answer = buffer + used;
		used += bytes;
		
		/*
			return the old top of stack
		*/
		return answer;
		}
	
	/*
		ALLOCATOR_MEMORY::REALIGN()
		---------------------------
	*/
	void allocator_memory::realign(void)
		{
		/*
			Compute the amount of padding that is needed to pad to a boundary of size sizeof(void *) (i.e. 4 bytes on a 32-bit machine of 8 bytes on a 64-bit machine)
		*/
		size_t padding = (used % alignment_boundary == 0) ? 0 : alignment_boundary - used % alignment_boundary;

		/*
			This might overflow, but for a fixed buffer it will simply cause failure at the next call to malloc().
		*/
		used += padding;
		}

	/*
		ALLOCATOR_MEMORY::REWIND()
		--------------------------
	*/
	void allocator_memory::rewind(void)
		{
		used = 0;
		}
	}
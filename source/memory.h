/*
	MEMORY.H
	--------
*/
/*!
	@file
	@brief Simple block-allocator that internally allocated a large chunk then allocates smaller blocks from this larger block
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <new>

namespace JASS
	{
	/*
		CLASS MEMORY
		------------
	*/
	/*!
		@brief Simple block-allocator that internally allocates a large chunk then allocates smaller blocks from this larger block
		@details This is a simple block allocator that internally allocated a large single block from the C++ free-store (or operating system)
		and then allows calls to allocate small blocks from that one block.  These small blocks cannot be individually deallocated, but rather
		are all deallocated at once when rewind() is called.  C++ allocators can easily be defined that allocate from a single object of this
		type, but that is left for other classes to manage.
		
		If the large memory block "runs out" then a second (and subsequent) block are allocated from the C++ free-store and they
		are chained together.  If the caller askes for a single piece of memory larger then that default_allocation_size then this
		class will allocate a chunk of the required size and return that to the caller.  Note that there is wastage at the end of each
		chunk as they callot be guaranteed to lie squentially in memory.
	*/
	class memory
	{
	private:
		static const size_t default_allocation_size = 1024 * 1024 * 1024;			///< Allocations from the C++ free-store are this size

	private:
		uint8_t *chunk;
		uint8_t *at;
		uint8_t *chunk_end;
		size_t used;
		size_t allocated;
		size_t block_size;

	protected:
		/*
			MEMORY::ALLOC()
			---------------
		*/
		/*!
			@brief Allocate more memory from the C++ free-store
			@param request_size [in] The size (in bytes) of the requested block.
			@return A pointer to a block of memory of size size, or NULL on failure.
		*/
		void *alloc(size_t size);
		
		/*
			MEMORY::DEALLOC()
			-----------------
		*/
		/*!
			@brief Hand back to the C++ free store (or Operating system) a chunk of memory that has previously been allocated with memory::alloc().
			@param buffer [in] A pointer previously returned by memory::alloc()
		*/
		void dealloc(void *buffer);
		
	public:
		/*
			MEMORY::MEMORY()
			----------------
		*/
		/*!
			@brief Constructor
			@param block_size_for_allocation [in] This size of the large-chunk allocation from the C++ free store or the Operating System.
		*/
		memory(size_t block_size_for_allocation = default_allocation_size);

		/*
			MEMORY::~MEMORY()
			----------------
		*/
		/*!
			@brief Destructor.
		*/
		~memory();

		/*
			MEMORY::MALLOC()
			----------------
		*/
		/*!
			@brief Allocate a small chunk of memory from the internal block and return a pointer to the caller
			@param bytes [in] The size of the chunk of memory.
			@return A pointer to a block of memory of size bytes, or NULL on failure.
		*/
		void *malloc(size_t bytes);
		
		/*
			MEMORY::CAPACITY()
			------------------
		*/
		/*!
			@brief Return the amount of memory that this object has allocated to it (i.e. the sum of the sizes of the large blocks in the large block chain)
			@return The number of bytes of memory allocated to this object.
		*/
		size_t capacity(void)
			{
			return allocated;
			}
		
		/*
			MEMORY::SIZE()
			--------------
		*/
		/*!
			@brief Return the number of bytes of memory this object has handed back to callers.
			@return Bytes used from the capacity()
		*/
		size_t bytes_used(void)
			{
			return used;
			}
		
		/*
			MEMORY::REALIGN()
			-----------------
		*/
		void realign(void);
		
		/*
			MEMORY::REWIND()
			----------------
		*/
		void rewind(void);
	} ;

	/*
		MEMORY::MALLOC()
		----------------
	*/
	inline void *memory::malloc(size_t bytes)
	{
	void *ans;

	#ifdef __arm__
		realign();
	#endif

	if (chunk == NULL || at + bytes > chunk_end)
		if (get_chained_block(bytes) == NULL)
			{
	#ifdef NEVER
			exit(printf("Out of memory:%lld bytes requested %lld bytes used %lld bytes allocated\n", (long long)bytes, (long long)used, (long long)allocated));
	#endif
			return NULL;		// out of memory (or the soft limit on this object has been exceeded)
			}

	ans = at;
	at += bytes;
	used += bytes;

	return ans;
	}
}

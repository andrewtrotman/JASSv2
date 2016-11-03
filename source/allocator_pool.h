/*
	ALLOCATOR_POOL.H
	----------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Simple block-allocator that internally allocated a large chunk then allocates smaller blocks from this larger block.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include <vector>
#include <mutex>

#include "allocator.h"

namespace JASS
	{
	/*
		CLASS ALLOCATOR_POOL
		--------------------
	*/
	/*!
		@brief Simple block-allocator that internally allocates a large chunk then allocates smaller blocks from this larger block
		@details This is a simple block allocator that internally allocated a large single block from the C++ free-store (or operating system)
		and then allows calls to allocate small blocks from that one block.  These small blocks cannot be individually deallocated, but rather
		are all deallocated all at once when rewind() is called.  C++ allocators can easily be defined that allocate from a single object of this
		type, but that is left for other classes to manage (for example, class allocator_cpp).
		
		If the large memory block "runs out" then a second (and subsequent) block are allocated from the C++ free-store and they
		are chained together.  If the caller askes for a single piece of memory larger then that default_allocation_size then this
		class will allocate a chunk of the required size and return that to the caller.  Note that there is wastage at the end of 
		each chunk as they cannot be guaranteed to lay squentially in memory.
		
		By default allocations by this class are not aligned to any particular boundary.  That is, if 1 byte is allocated then the next memory
		allocation is likely to be exactly one byte further on.  So allocation of a uint8_t followed by the allocation of a uint32_t is likely to
		result in the uint32_t being at an odd-numbered memory location.  Call malloc() with an alignment value to allocate an aligned piece of memory.
		On ARM, all memory allocations are word-aligned (sizeof(void *) by default because unaligned reads usually cause a fault.
		
		The use of new and delete in C++ (and malloc and free in C) is expensive as a substantial amount of work is necessary in order to maintain
		the heap.  This class reduces that cost - it exists for efficiency reasons alone.
	
		This allocator is thread safe.  A single allocator can be called from multiple threads concurrently and they will each
		return a valid pointer to a piece of memory that is not overlapping with any pointer returned from any other call and is of
		the requested size.
	*/
	class allocator_pool : public allocator
		{
		protected:
			static const size_t default_allocation_size = 1024 * 1024 * 1024;			///< Allocations from the C++ free-store are this size
			
		protected:
			size_t block_size;					///< The size (in bytes) of the large-allocations this object will make.

#ifdef USE_CRT_MALLOC
		std::vector<void *> crt_malloc_list;	///< When USE_CRT_MALLOC is defined the C RTL malloc() is called and this keeps track of those calls (so that rewind() works).
		std::mutex mutex;						///< Mutex used to control access to crt_malloc_list as it is not thread-safe.
#endif

		protected:
			/*
				CLASS ALLOCATOR_POOL::CHUNK
				---------------------------
			*/
			/*!
				@brief Details of an individual large-allocation unit.
				@details The large-allocations are kept in a linked list of chunks.  Each chunk stores a backwards pointer (of NULL if not backwards chunk) the 
				size of the allocation and details of it's use.  The large block that is allocated is actually the size of the caller's request plus the size of
				this structure.  The large-block is layed out as this object at the start and data[] being of the user's requested length.  That is, if the user
				asks for 1KB then the actual request from the C++ free store (or the Operating System) is 1BK + sizeof(allocator::chunk).
			*/
			class chunk
				{
				public:
					std::atomic<uint8_t *> chunk_at;	///< Pointer to the next byte that can be allocated (within the current chunk).
					uint8_t *chunk_end;					///< Pointer to the end of the current chunk's large allocation (used to check for overflow).
					chunk *next_chunk;					///< Pointer to the previous large allocation (i.e. chunk).
					size_t chunk_size;					///< The size of this chunk.
					#ifdef WIN32
						#pragma warning(push)			// Xcode thinks thinks a 0-sized entity in a class is OK, but Visual Studio kicks up a fuss (but does it anyway).
						#pragma warning(disable : 4200)
					#endif
						uint8_t data[];					///< The data in this large allocation that is available for re-distribution.
					#ifdef WIN32
						#pragma warning(pop)
					#endif
			};

		protected:
			std::atomic<chunk *> current_chunk;			///< Pointer to the top of the chunk list (of large allocations).

		private:
			/*
				ALLOCATOR_POOL::ALLOCATOR_POOL()
				--------------------------------
			*/
			/*!
				@brief Private copy constructor prevents object copying
			*/
			allocator_pool(allocator_pool &)
				{
				assert(0);
				}

			/*
				ALLOCATOR_POOL::OPERATOR=()
				---------------------------
			*/
			/*!
				@brief Private assignment operator prevents assigning to this object
			*/
			allocator_pool &operator=(const allocator_pool &)
				{
				assert(0);
				return *this;
				}
			
		protected:
			/*
				ALLOCATOR_POOL::ALLOC()
				-----------------------
			*/
			/*!
				@brief Allocate more memory from the C++ free-store
				@param size [in] The size (in bytes) of the requested block.
				@return A pointer to a block of memory of size size, or NULL on failure.
			*/
			void *alloc(size_t size) const
				{
				return ::malloc((size_t)size);
				}
			
			/*
				ALLOCATOR_POOL::DEALLOC()
				-------------------------
			*/
			/*!
				@brief Hand back to the C++ free store (or Operating system) a chunk of memory that has previously been allocated with allocator_pool::alloc().
				@param buffer [in] A pointer previously returned by allocator_pool::alloc()
			*/
			void dealloc(void *buffer) const
				{
				::free(buffer);
				}
			
			/*
				ALLOCATOR_POOL::ADD_CHUNK()
				---------------------------
			*/
			/*!
				@brief Get memory from the C++ free store (or the Operating System) and add it to the linked list of large-allocations.
				@details This is a maintenance method whose function is to allocate large chunks of memory and to maintain the liked list
				of these large chunks.  As an allocator this object is allocating memory for the caller, so it may as well manage its own list.
				The bytes parameter to this method is an indicator of the minimum amount of memory the caller requires, this object will allocate
				at leat that amount of space plus any space necessary for housekeeping.
				@param bytes [in] Allocate space so that it is possible to return an allocation is this parameter is size.
				@return A pointer to a chunk containig at least this amount of free space.
			*/
			chunk *add_chunk(size_t bytes);
			
		public:
			/*
				ALLOCATOR_POOL::ALLOCATOR_POOL()
				--------------------------------
			*/
			/*!
				@brief Constructor
				@param block_size_for_allocation [in] This size of the large-chunk allocation from the C++ free store or the Operating System.
			*/
			allocator_pool(size_t block_size_for_allocation = default_allocation_size);

			/*
				ALLOCATOR_POOL::~ALLOCATOR_POOL()
				---------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~allocator_pool();

			/*
				ALLOCATOR_POOL::MALLOC()
				------------------------
			*/
			/*!
				@brief Allocate a small chunk of memory from the internal block and return a pointer to the caller
				@param bytes [in] The size of the chunk of memory.
				@param alignment [in] If a word-aligned piece of memory is needed then this is the word-size (e.g. sizeof(void*))
				@return A pointer to a block of memory of size bytes, or NULL on failure.
			*/
			virtual void *malloc(size_t bytes, size_t alignment = alignment_boundary);
			
			/*
				ALLOCATOR_POOL::REWIND()
				------------------------
			*/
			/*!
				@brief Throw away (without calling delete) all objects allocated in the memory space of this object.
				@details This method rolls-back the memory that has been allocated by handing it all back to the C++ free store
				(or operating system).  delete is not called for any objects allocated in this space, the memory is simply re-claimed.
			*/
			virtual void rewind(void);
			
			/*
				ALLOCATOR_POOL::UNITTEST()
				--------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
}

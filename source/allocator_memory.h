/*
	ALLOCATOR_MEMORY.H
	------------------
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

#include "allocator.h"

namespace JASS
	{
	/*
		@brief Memory allocator out of a single non-growable fixed-sized buffer
	*/
	class allocator_memory : public allocator
		{
		private:
			uint8_t *buffer;		///< The buffer we're allocating from.
			size_t buffer_size;	///< The size of the buffer (we fail past this point).

		public:
			/*
				ALLOCATOR_MEMORY::ALLOCATOR_MEMORY()
				------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			allocator_memory(void *buffer, size_t length) :
				buffer((uint8_t *)buffer),
				buffer_size(length)
				{
				/*
					Nothing
				*/
				}

			/*
				ALLOCATOR_MEMORY::~ALLOCATOR_MEMORY()
				-------------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~allocator_memory()
				{
				/*
					Nothing
				*/
				}

			/*
				ALLOCATOR_MEMORY::MALLOC()
				--------------------------
			*/
			/*!
				@brief Allocate a small chunk of memory from the internal pool and return a pointer to the caller.
				@param bytes [in] The size of the chunk of memory.
				@return A pointer to a block of memory of size bytes, or NULL on failure.
			*/
			virtual void *malloc(size_t bytes);
			
			/*
				ALLOCATOR_MEMORY::REALIGN()
				---------------------------
			*/
			/*!
				@brief Signal that the next allocation should be on a machine-word boundary.
				@details Aligning all allocations on a machine-word boundary is a space / space trade off.  Allocating a string of single
				bytes one after the other and word-aligned would result in a machine word being used per byte.  To avoid this wastage this
				class, by default, does not word-align any allocations.  However, it is sometimes necessary to word-align because some
				assembly instructions require word-alignment.  This method wastes as little memory as possible to make sure that the
				next allocation is word-aligned.
			*/
			virtual void realign(void);

			/*
				ALLOCATOR_MEMORY::REWIND()
				--------------------------
			*/
			/*!
				@brief Throw away (without calling delete) all objects allocated in the memory space of this allocator  Delete is not called 
				for any objects allocated in this space, the memory is simply re-claimed.
			*/
			virtual void rewind(void);
		};
	}
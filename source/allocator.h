/*
	ALLOCATOR.H
	-----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Base class for different allocators.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "allocator.h"

namespace JASS
	{
	/*
		CLASS ALLOCATOR
		---------------
	*/
	/*!
		@brief Virtual base class for C style allocators.
	*/
	class allocator
		{
		protected:
			static constexpr size_t alignment_boundary = sizeof(void *);		///< When align() is called to re-align data to a word-boundary, this is the word-size used to determine the boindary
			
		protected:
			size_t used;						///< The number of bytes this object has passed back to the caller.
			size_t allocated;					///< The number of bytes this object has allocated.

		public:
			/*
				ALLOCATOR::ALLOCATOR()
				----------------------
			*/
			/*!
				@brief Constructor
			*/
			allocator() :
				used(0),
				allocated(0)
				{
				}

			/*
				ALLOCATOR::~ALLOCATOR()
				-----------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~allocator()
				{
				/*
					Nothing
				*/
				}

			/*
				ALLOCATOR::MALLOC()
				-------------------
			*/
			/*!
				@brief Allocate a small chunk of memory from the internal pool and return a pointer to the caller.
				@param bytes [in] The size of the chunk of memory.
				@return A pointer to a block of memory of size bytes, or NULL on failure.
			*/
			virtual void *malloc(size_t bytes) = 0;
			
			/*
				ALLOCATOR::CAPACITY()
				---------------------
			*/
			/*!
				@brief Return the amount of memory that this object has allocated to it (i.e. the sum of the sizes of the large blocks in the large block chain)
				@return The number of bytes of memory allocated to this object.
			*/
			size_t capacity(void) const
				{
				return allocated;
				}
			
			/*
				ALLOCATOR::SIZE()
				-----------------
			*/
			/*!
				@brief Return the number of bytes of memory this object has handed back to callers.
				@return Bytes used from the capacity()
			*/
			size_t size(void) const
				{
				return used;
				}
			
			/*
				ALLOCATOR::REALIGN()
				--------------------
			*/
			/*!
				@brief Signal that the next allocation should be on a machine-word boundary.
				@details Aligning all allocations on a machine-word boundary is a space / space trade off.  Allocating a string of single
				bytes one after the other and word-aligned would result in a machine word being used per byte.  To avoid this wastage this
				class, by default, does not word-align any allocations.  However, it is sometimes necessary to word-align because some
				assembly instructions require word-alignment.  This method wastes as little memory as possible to make sure that the
				next allocation is word-aligned.
			*/
			virtual void realign(void) = 0;

			
			/*
				ALLOCATOR::REWIND()
				-------------------
			*/
			/*!
				@brief Throw away (without calling delete) all objects allocated in the memory space of this allocator  Delete is not called 
				for any objects allocated in this space, the memory is simply re-claimed.
			*/
			virtual void rewind(void) = 0;
		};
}

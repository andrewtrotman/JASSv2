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

#include <atomic>

namespace JASS
	{
	/*
		CLASS ALLOCATOR
		---------------
	*/
	/*!
		@brief Virtual base class for C style allocators.
		@details All allocators are thread safe.  A single allocator can be called from multiple threads concurrently and they will each
		return a valid pointer to a piece of memory that is not overlapping with any pointer returned from any other call and is of
		the requested size.
	*/
	class allocator
		{
		protected:
			#ifdef __aarch64__
				static constexpr size_t alignment_boundary = sizeof(void *);		///< On ARM its necessary to align all memory allocations on word boundaries
			#else
				static constexpr size_t alignment_boundary = 1;							///< Elsewhere don't bother with alignment (align on byte boundaries)
			#endif
		protected:
			std::atomic<size_t> used;						///< The number of bytes this object has passed back to the caller.
			std::atomic<size_t> allocated;				///< The number of bytes this object has allocated.

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
				ALLOCATOR::OPERATOR==()
				-----------------------
			*/
			/*!
				@brief Compare for equality two objects of this class type.
				@param with [in] The object to compare to.
				@return True if this == that, else false.
			*/
			virtual bool operator==(const allocator &with) = 0;
			
			/*
				ALLOCATOR::OPERATOR!=()
				-----------------------
			*/
			/*!
				@brief Compare for inequlity two objects of this class type.
				@param with [in] The object to compare to.
				@return True if this != that, else false.
			*/
			virtual bool operator!=(const allocator &with) = 0;

			/*
				ALLOCATOR::MALLOC()
				-------------------
			*/
			/*!
				@brief Allocate a small chunk of memory from the internal pool and return a pointer to the caller.
				@param bytes [in] The size of the chunk of memory.
				@param alignment [in] If a word-aligned piece of memory is needed then this is the word-size (e.g. sizeof(void*))
				@return A pointer to a block of memory of size bytes, or NULL on failure.
			*/
			virtual void *malloc(size_t bytes, size_t alignment = alignment_boundary) = 0;
			
			/*
				ALLOCATOR::CAPACITY()
				---------------------
			*/
			/*!
				@brief Return the amount of memory that this object has allocated to it.
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
				@brief Compute the number of extra bytes of memory necessary for an allocation to start on an aligned boundary.
				@details Aligning all allocations on a machine-word boundary is a space / space trade off.  Allocating a string of single
				bytes one after the other and word-aligned would result in a machine word being used per byte.  To avoid this wastage this
				class, by default, does not word-align any allocations.  However, it is sometimes necessary to word-align because some
				assembly instructions require word-alignment.  This method return the number of bytes of padding necessary to make an 
				address word-aligned.
				@param address [in] Compute the number of wasted bytes from this address to the next bounday
				@param boundary [in] The byte-boundary to which this address should be alligned (e.g. 4 will ensure the least significant 2 bits are alwasys 00)
				@return The number of bytes to add to address to make it aligned
			*/
			static size_t realign(const void *address, size_t boundary)
				{
				return realign((uintptr_t)address, boundary);
				}

			/*
				ALLOCATOR::REALIGN()
				--------------------
			*/
			/*!
				@brief Compute the number of extra bytes of memory necessary for an allocation to start on an aligned boundary.
				@details Aligning all allocations on a machine-word boundary is a space / space trade off.  Allocating a string of single
				bytes one after the other and word-aligned would result in a machine word being used per byte.  To avoid this wastage this
				class, by default, does not word-align any allocations.  However, it is sometimes necessary to word-align because some
				assembly instructions require word-alignment.  This method return the number of bytes of padding necessary to make an
				address word-aligned.
				@param current_pointer [in] Compute the number of wasted bytes from this address to the next bounday
				@param boundary [in] The byte-boundary to which this address should be alligned (e.g. 4 will ensure the least significant 2 bits are alwasys 00)
				@return The number of bytes to add to address to make it aligned
			*/
			static size_t realign(uintptr_t current_pointer, size_t boundary)
				{
				/*
					Compute the amount of padding that is needed to pad to a boundary of size alignment_boundary
				*/
				size_t padding = (current_pointer % boundary == 0) ? 0 : boundary - current_pointer % boundary;

				/*
					Return the number of bytes that must be addedd to address to make it aligned
				*/
				return padding;
				}

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

/*
	ALLOCATOR_MEMORY.H
	------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Simple block-allocator that internally uses a single non-growable fixed-sized buffer
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "allocator.h"

namespace JASS
	{
	/*!
		@brief Memory allocator out of a single non-growable fixed-sized buffer
		@details This allocator is thread safe.  A single allocator can be called from multiple threads concurrently and they will each
		return a valid pointer to a piece of memory that is not overlapping with any pointer returned from any other call and is of
		the requested size.
	*/
	class allocator_memory : public allocator
		{
		protected:
			std::atomic<uint8_t *> buffer;		///< The buffer we're allocating from.

		public:
			/*
				ALLOCATOR_MEMORY::ALLOCATOR_MEMORY()
				------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			allocator_memory(void *buffer, size_t length) :
				buffer((uint8_t *)buffer)
				{
				allocated = length;
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
				ALLOCATOR_MEMORY::OPERATOR==()
				------------------------------
			*/
			/*!
				@brief Compare for equality two objects of this class type.
				@param with [in] The object to compare to.
				@return True if this == that, else false.
			*/
			virtual bool operator==(const allocator &with)
				{
				try
					{
					/*
						Downcast the parameter to an allocator_memory and see if it matches this.
					*/
					auto other = dynamic_cast<const allocator_memory *>(&with);
					return (used == other->size()) && (allocated == other->capacity()) && (buffer.load() == other->buffer.load());
					}
				catch (...)
					{
					return false;
					}
				}
			
			/*
				ALLOCATOR_MEMORY::OPERATOR!=()
				------------------------------
			*/
			/*!
				@brief Compare for inequlity two objects of this class type.
				@param with [in] The object to compare to.
				@return True if this != that, else false.
			*/
			virtual bool operator!=(const allocator &with)
				{
				try
					{
					/*
						Downcast the parameter to an allocator_memory and see if it matches this.
					*/
					auto other = dynamic_cast<const allocator_memory *>(&with);
					return (used != other->size()) || (allocated != other->capacity()) || (buffer.load() != other->buffer.load());
					}
				catch (...)
					{
					return false;
					}
				}
			
			/*
				ALLOCATOR_MEMORY::MALLOC()
				--------------------------
			*/
			/*!
				@brief Allocate a small chunk of memory from the internal pool and return a pointer to the caller.
				@param bytes [in] The size of the chunk of memory.
				@param alignment [in] If a word-aligned piece of memory is needed then this is the word-size (e.g. sizeof(void*))
				@return A pointer to a block of memory of size bytes, or NULL on failure.
			*/
			virtual void *malloc(size_t bytes, size_t alignment = alignment_boundary);
			
			/*
				ALLOCATOR_MEMORY::REWIND()
				--------------------------
			*/
			/*!
				@brief Throw away (without calling delete) all objects allocated in the memory space of this allocator  Delete is not called 
				for any objects allocated in this space, the memory is simply re-claimed.
			*/
			virtual void rewind(void);
			
			/*
				ALLOCATOR_POOL::UNITTEST()
				--------------------------
			*/
			/*!
				@brief Unit test this class.
			*/
			static void unittest(void);
		};
	}

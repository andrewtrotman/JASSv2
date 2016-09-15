/*
	SLICE.H
	-------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Slices (also known as string-descriptors) for C++.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <string.h>

#include "allocator.h"

namespace JASS
	{
	/*
		@brief C++ slices (string-descriptors)
	*/
	class slice
		{
		private:
			void *pointer;			///< The start of the data.
			size_t length;			///< The length of the data (in bytes).
			
		public:
			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Constructor
				@param pointer [in] The start address of the memory this slice represents.
				@param length [in] The length (in bytes) of this slice.
			*/
			slice(void *pointer = NULL, size_t length = 0) :
				pointer(pointer),
				length(length)
				{
				/* Nothing */
				}
			
			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Constructor
				@param start [in] The start address of the memory this slice represents.
				@param end [in] The end address of the memory this slice represents.
			*/
			slice(void *start, void *end) :
				pointer(start),
				length((uint8_t *)end - (uint8_t *)start)
				{
				/* Nothing */
				}

			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct a slice by copying the data into allocator's pool of memory.  This does NOT ever delete the memory, that is the allocator's task.
				@param pool [in] An allocator to allocate the memory from.
				@param start [in] The start address of the memory this slice represents.
				@param end [in] The end address of the memory this slice represents.
			*/
			slice(allocator &pool, void *start, void *end)
				{
				length = (uint8_t *)end - (uint8_t *)start;
				pointer = (void *)pool.malloc(length);
				memcpy(pointer, start, length);
				}
			
			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct a slice by copying and '\0' termainating a string, using the allocator's pool of memory.  This does NOT ever delete the memory, that is the allocator's task.
				@param pool [in] An allocator to allocate the memory from.
				@param start [in] The start address of the C string.
				@param end [in] The end address of the C string.
			*/
			slice(allocator &pool, const char *start, const char *end)
				{
				length = end - start;
				pointer = (void *)pool.malloc(length + 1);
				memcpy(pointer, start, length);
				((char *)pointer)[length] = '\0';
				}
			
			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct a slice by copying and '\0' termainating a string, using the allocator's pool of memory.  This does NOT ever delete the memory, that is the allocator's task.
				@param pool [in] An allocator to allocate the memory from.
				@param start [in] The start address of the C string.
			*/
			slice(allocator &pool, const char *start)
				{
				length = strlen(start);
				pointer = (void *)pool.malloc(length + 1);
				memcpy(pointer, start, length);
				((char *)pointer)[length] = '\0';
				}

			/*
				SLICE::SLICE()
				--------------
			*/
			/*!
				@brief Construct a slice by allocating bytes of memory from a pool allocagor
				@param pool [in] An allocator to allocate from.
				@param bytes [in] The number of bytes of memory to allocate from the pool.
			*/
			slice(allocator &pool, size_t bytes)
				{
				length = bytes;
				pointer = (void *)pool.malloc(bytes);
				}
			
			/*
				SLICE::SIZE()
				-------------
			*/
			/*!
				@brief Return the length of this slice.
				@return Slice length.
			*/
			size_t size(void)
				{
				return length;
				}
			
			/*
				SLICE::ADDRESS()
				----------------
			*/
			/*!
				@brief Extract the pointer value from the slice
				@return The slice's internal pointer
			*/
			void *address(void)
				{
				return pointer;
				}
			
			/*
				SLICE::RESIZE()
				---------------
			*/
			/*!
				@brief Change the length of the slice.
				@param new_size[in] The new size (in bytes) of the slice.
			*/
			void resize(size_t new_size)
				{
				length = new_size;
				}
			
			/*
				SLICE::OPERATOR[]()
				-------------------
			*/
			/*!
				@brief Return a reference to the n'th byte past the start of the slice.
				@param index [in] Which byte to return.
				@return A reference to the byte at position specified in index.
			*/
			uint8_t &operator[](size_t index)
				{
				return ((uint8_t *)pointer)[index];
				}
		};
	}

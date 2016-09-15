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
		public:
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
		};
	}

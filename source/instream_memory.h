/*
	INSTREAM_MEMORY.H
	-----------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Subclass of instream for reading data from a memory buffer.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdio.h>

#include "instream.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_MEMORY
		---------------------
	*/
	/*!
		@brief Subclass of instream for reading data from a memory buffer.
	*/
	class instream_memory : public instream
		{
		protected:
			size_t bytes_read;					///< The number of bytes this object has read for the buffer
			const uint8_t *file;					///< A pointer to the in-memory buffer (thought of as an in-place file)
			size_t file_length;					///< The length of the in-memory buffer

		public:
			/*
				INSTREAM_MEMORY::INSTREAM_MEMORY()
				----------------------------------
			*/
			/*!
				@brief Constructor
				@param memory [in] A pointer to the start of the buffer
				@param length [in] The length of the buffer
			*/
			instream_memory(const void *memory, size_t length):
				instream(),
				bytes_read(0),
				file((uint8_t *)memory),
				file_length(length)
				{
				/* Nothing */
				}
			/*
				INSTREAM_MEMORY::READ()
				-----------------------
			*/
			/*!
				@brief Read buffer.size() bytes of data into buffer.contents, resizing on eof.
				@param buffer [out] buffer.contents.size() bytes of data are read from source into buffer which is resized to the number of bytes read on eof.
			*/
			virtual void read(document &buffer);
			
			/*
				INSTREAM_MEMORY::UNITTEST()
				---------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}

/*
	CHANNEL_BUFFER.H
	--------------
	Copyright (c) 2020 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Input (not output) channel from a block of memory
	@author Andrew Trotman
	@copyright 2020 Andrew Trotman
*/
#pragma once

#include <sstream>

#include "maths.h"
#include "channel.h"

namespace JASS
	{
	/*
		CLASS CHANNEL_BUFFER
		--------------------
		This channel reads from a block of memory passed to the constructor..
	*/
	class channel_buffer : public channel
		{
		private:
			std::shared_ptr<char> source;		///< The block of memory this object is reading from
			char *current;							///< Where we are in that block of memory
			size_t remaining;						///< How many bytes are remaining to the read from the channel

		protected:
			/*
				CHANNEL_BUFFER::BLOCK_WRITE()
				-----------------------------
			*/
			/*!
				@brief Not implemented (will cause program termination)
			*/
			virtual size_t block_write(const void *buffer, size_t length)
				{
				exit(printf("channel_buffer::block_write not implemented (class only reading)"));
				}
				
			/*
				CHANNEL_BUFFER::BLOCK_READ()
				----------------------------
			*/
			/*!
				@brief All input happens via the block_read method.
				@param into [out] length number of bytes are written into here
				@param length [in] The number of bytes go write.
				@return The number of buyes read (usually equal to length).
			*/
			virtual size_t block_read(void *into, size_t length)
				{
				/*
					Check for EOF
				*/
				if (remaining <= 0 || length == 0)
					return 0;

				/*
					Read the given number of bytes from the buffer.
				*/
				size_t bytes_to_read = JASS::maths::minimum(length, remaining);
				::memcpy(into, current, bytes_to_read);
				remaining -= bytes_to_read;
				current += bytes_to_read;

				return bytes_to_read;
				}

		public:
			/*
				CHANNEL_BUFFER::CHANNEL_BUFFER()
				--------------------------------
			*/
			/*!
				@brief Constructor
				@param source [in] a pointer to the data to read from
			*/
			channel_buffer(std::shared_ptr<char> source) :
				source(source)
				{
				current = this->source.get();
				remaining = strlen(current);
				}

			/*
				CHANNEL_BUFFER::UNITTEST()
				--------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		} ;
	}

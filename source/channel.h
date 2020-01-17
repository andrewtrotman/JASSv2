/*
	CHANNEL.H
	---------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Input and output channels so that JASS i/o is device independant.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <stdio.h>
#include <string.h>

#include <string>
#include <sstream>
#include <type_traits>

#include "string_cpp.h"

namespace JASS
	{
	/*
		CLASS CHANNEL
		-------------
	*/
	/*!
		@brief General purpose device independant I/O class
		@details So that the search engine can read and write to either a file or stdin/stdout or a socket (or othereise)
		it uses a channel rather than a specific device to do the I/O.  To create a new channel, overload the constructor, destructor
		as well as block_write() and block_read().  It is not necessary to overload getsz(), but for efficiency reasons it might be desired.
	*/
	class channel
		{
		protected:
			/*
				CHANNEL::BLOCK_WRITE()
				----------------------
			*/
			/*!
				@brief All output happens via the block_write method.
				@param buffer [in] write length number of bytes from buffer
				@param length [in] The number of bytes go write.
				@return The number of bytes written (usually equal to length).
			*/
			virtual size_t block_write(const void *buffer, size_t length) = 0;

			/*
				CHANNEL::BLOCK_READ()
				---------------------
			*/
			/*!
				@brief All input happens via the block_read method.
				@param into [out] length number of bytes are written into into
				@param length [in] The number of bytes to read.
				@return The number of buyes read (usually equal to length).
			*/
			virtual size_t block_read(void *into, size_t length) = 0;

			/*
				CHANNEL::GETSZ()
				----------------
			*/
			/*!
				@brief Read up-to and including a terminator terminated string of bytes from the channel.
				@details This method calls block_read byte at a time until a byte of value terminator is seen.
				these bytes (including the terminator) are writen to into.  The size of into is set to the 
				number of bytes read.
				@param into [out] bytes are read from the channel until a bytes equal to terminator is found.
				@param terminator [in] Stop when this byte is seen in the channel and after it is written to into.
			*/
			template<typename STRING_TYPE>
			void getsz(STRING_TYPE &into, char terminator = '\0')
				{
				char next;								// the byte being read
				size_t used = 0;						// the number of bytes that have been copied so far
				size_t buffer_length = 0;			// the current length of the string buffer
				size_t growth_factor = 1024;		// how much larger the buffer should get when it runs out.

				/*
					In case of failure set the size to 0.
				*/
				into.resize(0);

				/*
					For as long as we can read, add to the end of the string so far.
				*/
				while (block_read(&next, 1) == 1)
					{
					used++;

					/*
						check to see if we need to extend the length of the string buffer
					*/
					if (used >= buffer_length)
						{
						buffer_length += growth_factor;
						into.reserve(buffer_length);
						}

					/*
						shove the next byte on the end
					*/
					into.push_back(next);

					/*
						if we match terminator then we're done so set the correct string length
					*/
					if (next == terminator)
						{
						into.resize(used);
						return;
						}
					}
				/*
					at eof so set the string length and return.
				*/
				into.resize(used);
				}

		public:
			/*
				CHANNEL::CHANNEL()
				------------------
			*/
			/*!
				@brief Constructor
			*/
			channel()
				{
				/* Nothing */
				}

			/*
				CHANNEL::~CHANNEL()
				-------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~channel()
				{
				/* Nothing */
				}

			/*
				CHANNEL::READ()
				---------------
			*/
			/*!
				@brief Read into.size() bytes into the string.
				@param into [out] Read into.size() bytes into this string.
			*/
			void read(std::string &into)
				{
				into.resize(block_read(const_cast<char *>(into.c_str()), into.size()));
				}

			/*
				CHANNEL::GETS()
				---------------
			*/
			/*!
				@brief Read a '\n' terminated string from the channel into the parameter.
				@param into [out] Read into this string.
			*/
			virtual void gets(std::string &into)
				{
				getsz<std::string>(into, '\n');
				}
			
			/*
				CHANNEL::GETS()
				---------------
			*/
			/*!
				@brief Read a '\n' terminated string from the channel into the parameter.
				@param into [out] Read into this string.
			*/
			virtual void gets(JASS::string &into)
				{
				getsz<JASS::string>(into, '\n');
				}

			/*
				CHANNEL::PUTS()
				---------------
			*/
			/*!
				@brief write buffer to the channel followed by a '\n' character.
				@param buffer [in] Byte sequence to write.
			*/
			virtual void puts(const std::string &buffer)
				{
				block_write(buffer.c_str(), buffer.size());
				block_write("\n", 1);
				}

			/*
				CHANNEL::OPERATOR<<()
				---------------------
			*/
			/*!
				@brief Catch-all for writing arithmetic types (itegral and floating) to a channel.
				@param source [in] data to write.
			*/
			template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value, T>::type>
			channel &operator<<(T source)
				{
				std::ostringstream output;

				output << source;
				*this << output.str();
				return *this;
				}

			/*
				CHANNEL::OPERATOR<<()
				---------------------
			*/
			/*!
				@brief write a string to this channel.
				@param source [in] data to write.
			*/
			channel &operator<<(const char *source)
				{
				block_write(source, strlen(source));
				return *this;
				}

			/*
				CHANNEL::OPERATOR<<()
				---------------------
			*/
			/*!
				@brief write a string to this channel.
				@param source [in] data to write.
			*/
			channel &operator<<(const unsigned char *source)
				{
				block_write(source, strlen(reinterpret_cast<const char *>(source)));
				return *this;
				}

			/*
				CHANNEL::OPERATOR<<()
				---------------------
			*/
			/*!
				@brief write a string to this channel.
				@param source [in] data to write.
			*/
			channel &operator<<(const std::string &source)
				{
				block_write(source.c_str(), source.size());
				return *this;
				}
			
			/*
				CHANNEL::OPERATOR<<()
				---------------------
			*/
			/*!
				@brief write a string to this channel.
				@param source [in] data to write.
			*/
			channel &operator<<(const JASS::string &source)
				{
				block_write(source.c_str(), source.size());
				return *this;
				}
		} ;
	}

/*
	COMPRESS_INTEGER.H
	------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compression codexes for integer sequences.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdint.h>
#include <stdlib.h>

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER
		----------------------
	*/
	/*!
		@brief Compression codexes for integer sequences
		@details To implement a codex you need to subclass this virtual base class and implement two
		methods, encode() and decode().  As those methods are virtual, an object of the given subclass
		is needed in order to encode or decode integer sequences.
	*/
	class compress_integer
		{
		public:
			typedef uint32_t integer;									///< This class and descendants will work on integers of this size.  Do not change without also changing JASS_COMPRESS_INTEGER_BITS_PER_INTEGER
			#define JASS_COMPRESS_INTEGER_BITS_PER_INTEGER 32	///< The number of bits in compress_integer::integer (either 32 or 64). This must remain in sync with compress_integer::integer (and a hard coded value to be used in \#if statements)

		public:
			/*
				COMPRESS_INTEGER::COMPRESS_INTEGER()
				------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			compress_integer()
				{
				/* Nothing */
				}
			/*
				COMPRESS_INTEGER::COMPRESS_INTEGER()
				------------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~compress_integer()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER::ENCODE()
				--------------------------
			*/
			/*!
				@brief Encode a sequence of integers returning the number of bytes used for the encoding, or 0 if the encoded sequence doesn't fit in the buffer.
				@param encoded [out] The sequence of bytes that is the encoded sequence.
				@param encoded_buffer_length [in] The length (in bytes) of the output buffer, encoded.
				@param source [in] The sequence of integers to encode.
				@param source_integers [in] The length (in integers) of the source buffer.
				@return The number of bytes used to encode the integer sequence, or 0 on error (i.e. overflow).
			*/
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers) = 0;
			
			/*
				COMPRESS_INTEGER::DECODE()
				--------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex.
				@param decoded [out] The sequence of decoded integers.
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
			virtual void decode(integer *decoded, size_t integers_to_decode, const void *source, size_t source_length) = 0;
		} ;
	}

/*
	COMPRESS_INTEGER_NONE.H
	-----------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief The null compressor for integer sequences (i.e. just copy, don't compress).
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_NONE
		---------------------------
	*/
	/*!
		@brief The null compressor for integer sequences (i.e. just copy, don't compress).
	*/
	class compress_integer_none : public compress_integer
		{
		public:
			/*
				COMPRESS_INTEGER_NONE::COMPRESS_INTEGER_NONE()
				----------------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			compress_integer_none()
				{
				/* Nothing */
				}
			
			/*
				COMPRESS_INTEGER_NONE::~COMPRESS_INTEGER_NONE()
				-----------------------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~compress_integer_none()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_NONE::ENCODE()
				-------------------------------
			*/
			/*!
				@brief Encode a sequence of integers returning the number of bytes used for the encoding, or 0 if the encoded sequence doesn't fit in the buffer.
				@param encoded [out] The sequence of bytes that is the encoded sequence.
				@param encoded_buffer_length [in] The length (in bytes) of the output buffer, encoded.
				@param source [in] The sequence of integers to encode.
				@param source_integers [in] The length (in integers) of the source buffer.
				@return The number of bytes used to encode the integer sequence, or 0 on error (i.e. overflow).
			*/
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers);
			
			/*
				COMPRESS_INTEGER_NONE::DECODE()
				-------------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex.
				@param decoded [out] The sequence of decoded integers.
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
			virtual void decode(integer *decoded, size_t integers_to_decode, const void *source, size_t source_length);
				
			/*
				COMPRESS_INTEGER_NONE::UNITTEST()
				---------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		} ;


}

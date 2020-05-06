/*
	COMPRESS_INTEGER_BITPACK_256.H
	------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Pack fixed-width integers into a 256-bit SIMD register
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include "compress_integer_bitpack.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_BITPACK_256
		-----------------------------------
	*/
	/*!
		@brief Fixed-width pack integers into as few 256-bit SIMD words as possible.
		@details
		Pack into 256-bit SIMD word a bunch of same-width integers, so:
				32 * 8 * 1-bit integers
				16 * 8 * 2-bit integers
				10 * 8 * 3-bit integers
				8 * 8 * 4-bit integers
				6 * 8 * 5-bit integers
				5 * 8 * 6-bit integers
				4 * 8 * 8-bit integers
				3 * 8 * 10-bit integers
				2 * 8 * 16-bit integers
				1 * 8 * 32-bit integers
	*/
	class compress_integer_bitpack_256 : public compress_integer_bitpack
		{
		public:
			/*
				COMPRESS_INTEGER_BITPACK_256::ENCODE()
				--------------------------------------
			*/
			/*!
				@brief Encode a sequence of integers returning the number of bytes used for the encoding, or 0 if the encoded sequence doesn't fit in the buffer.
				@param encoded [out] The sequence of bytes that is the encoded sequence.
				@param encoded_buffer_length [in] The length (in bytes) of the output buffer, encoded.
				@param source [in] The sequence of integers to encode.
				@param source_integers [in] The length (in integers) of the source buffer.
				@return The number of bytes used to encode the integer sequence, or 0 on error (i.e. overflow).
			*/
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers)
				{
				return compress_integer_bitpack::encode<256>(encoded, encoded_buffer_length, source, source_integers);
				}

			/*
				COMPRESS_INTEGER_BITPACK_256::DECODE()
				--------------------------------------
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
				COMPRESS_INTEGER_BITPACK_256::UNITTEST()
				----------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				compress_integer_bitpack_256 *compressor = new compress_integer_bitpack_256;
				compress_integer::unittest(*compressor);
				delete compressor;
				puts("compress_integer_bitpack_256::PASSED");
				}
		};
	}


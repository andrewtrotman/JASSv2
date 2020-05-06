/*
	COMPRESS_INTEGER_BITPACK_128.H
	------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Pack fixed-width integers into a 128-bit SIMD register
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include "compress_integer_bitpack.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_BITPACK_128
		-----------------------------------
	*/
	/*!
		@brief Fixed-width pack integers into as few 128-bit SIMD words as possible.
		@details
		Pack into 128-bit SIMD word a bunch of same-width integers, so:
				32 * 4 * 1-bit integers
				16 * 4 * 2-bit integers
				10 * 4 * 3-bit integers
				8 * 4 * 4-bit integers
				6 * 4 * 5-bit integers
				5 * 4 * 6-bit integers
				4 * 4 * 8-bit integers
				3 * 4 * 10-bit integers
				2 * 4 * 16-bit integers
				1 * 4 * 32-bit integers
	*/
	class compress_integer_bitpack_128: public compress_integer_bitpack
		{
		public:
			/*
				COMPRESS_INTEGER_BITPACK_128::ENCODE()
				---------------------------------------
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
				return compress_integer_bitpack::encode<128>(encoded, encoded_buffer_length, source, source_integers);
				}

			/*
				COMPRESS_INTEGER_BITPACK_128::DECODE()
				---------------------------------------
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
				COMPRESS_INTEGER_BITPACK_128::UNITTEST()
				----------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				compress_integer_bitpack_128 *compressor = new compress_integer_bitpack_128;
				compress_integer::unittest(*compressor);
				delete compressor;
				puts("compress_integer_bitpack_128::PASSED");
				}
		};
	}


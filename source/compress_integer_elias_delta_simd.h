/*
	COMPRESS_INTEGER_ELIAS_DELTA_SIMD.H
	-----------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Elias delta based SIMD compression
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/

#pragma once

#include <stdint.h>
#include <immintrin.h>


#include "forceinline.h"
#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_ELIAS_DELTA_SIMD
		---------------------------------------
	*/
	/*!
		@brief SIMD compression (line Simple-9) using Elias delta to encode the selectors

		@details  To quote the Wikipedia (https://en.wikipedia.org/wiki/Elias_delta_coding):
		The selectors are encoded using elias delta and the payload is an AVX-512 word with each integer width being determined by the selector

		To code a number X ≥ 1:
			Let N = floor(log2 X) be the highest power of 2 in X, so 2^N ≤ X < 2^(N+1).
			Let L = floor(log2 N+1) be the highest power of 2 in N+1, so 2^L ≤ N+1 < 2^(L+1).
			Write L zeros, followed by
			the L+1-bit binary representation of N+1, followed by
			all but the leading bit (i.e. the last N bits) of X.
		An equivalent way to express the same process:
			Separate X into the highest power of 2 it contains (2^N) and the remaining N binary digits.
			Encode N+1 with Elias gamma coding.
			Append the remaining N binary digits to this representation of N+1.
		See:  P. Elias (1975). "Universal codeword sets and representations of the integers". IEEE Transactions on Information Theory. 21 (2): 194–203. doi:10.1109/tit.1975.1055349.

	*/
	class compress_integer_elias_delta_simd: public compress_integer
		{
		protected:
			/*
				COMPRESS_INTEGER_ELIAS_DELTA_SIMD::FIND_FIRST_SET_BIT()
				-------------------------------------------------------
			*/
			/*!
				@brief return the position of the least significant set bit (using a single machine code instruction)
				@param [in] value the integer to check.
				@return The position of the lowest set bit (or 0 if no bits are set)
			*/
			static forceinline uint64_t find_first_set_bit(uint64_t value)
				{
				return _tzcnt_u64(value) + 1;
				}

			forceinline void push_selector(uint32_t *&destination, uint8_t raw, uint32_t &selector_bits_used, uint64_t &accumulated_selector);
 			uint8_t forceinline decode_selector(const uint32_t *&selector_set, uint32_t &selector_bits_used, uint64_t &accumulated_selector);

		public:
			/*
				COMPRESS_INTEGER_ELIAS_DELTA_SIMD::ENCODE()
				-------------------------------------------
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
				COMPRESS_INTEGER_ELIAS_DELTA_SIMD::DECODE()
				-------------------------------------------
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
				COMPRESS_INTEGER_ELIAS_DELTA_SIMD::UNITTEST()
				---------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}

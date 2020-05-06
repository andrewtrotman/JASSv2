/*
	COMPRESS_INTEGER_ELIAS_GAMMA.H
	------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Elias gamma compression
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_ELIAS_GAMMA
		----------------------------------
	*/
	/*!
		@brief Elias gamma encoding of integers
		@details  To quote the Wikipedia (https://en.wikipedia.org/wiki/Elias_gamma_coding):

		To code a number x ≥ 1:
			Let N = floor(log2(x)) be the highest power of 2 it contains, so 2^N ≤ x < 2^(N+1).
			Write out N zero bits, then
			Append the binary form of x, an (N + 1)-bit binary number.
		An equivalent way to express the same process:
			Encode N in unary; that is, as N zeroes followed by a one.
			Append the remaining N binary digits of x to this representation of N.

		To represent a number x Elias gamma uses 2 * floor(log2(x)) + 1 bits.

		See:  P. Elias (1975). "Universal codeword sets and representations of the integers". IEEE Transactions on Information Theory. 21 (2): 194–203. doi:10.1109/tit.1975.1055349.
	*/
	class compress_integer_elias_gamma: public compress_integer
		{
		public:
			/*
				COMPRESS_INTEGER_ELIAS_GAMMA::ENCODE()
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
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers);

			/*
				COMPRESS_INTEGER_ELIAS_GAMMA::DECODE()
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
				COMPRESS_INTEGER_ELIAS_GAMMA::UNITTEST()
				----------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}

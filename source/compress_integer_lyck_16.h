/*
	COMPRESS_INTEGER_LYCK_16.H
	--------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Pack (small) integers into 2-bit pairs (lycks) and then into a 16-bit word.
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_LYCK_16
		------------------------------
	*/
	/*!
		@brief Fixed-width pack integers into lycks and then into a 16-bit word
		@details
			Pack either:
					8 * 2-bit integers
					4 * 4-bit integers
					2 * 8-bit integers
					1 * 16-bit integer
			and place a selector beforehand saying how the data is encoded.
	*/
	class compress_integer_lyck_16: public compress_integer
		{
		public:
			/*
				COMPRESS_INTEGER_LYCK_16::ENCODE()
				----------------------------------
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
				COMPRESS_INTEGER_LYCK_16::DECODE()
				----------------------------------
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
				COMPRESS_INTEGER_LYCK_16::UNITTEST_ONE()
				----------------------------------------
			*/
			/*!
				@brief Test one sequence to make sure it encodes and decodes to the same thing.  Assert if not.
				@param sequence [in] the sequernce to encode.
			*/
			static void unittest_one(const std::vector<uint32_t> &sequence);

			/*
				COMPRESS_INTEGER_LYCK_16::UNITTEST()
				------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}



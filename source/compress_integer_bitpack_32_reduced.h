/*
	COMPRESS_INTEGER_BITPACK_32_REDUCED.H
	-------------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Pack fixed-width integers into a 32-bit register
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include "compress_integer_bitpack.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_BITPACK_32_REDUCED
		-----------------------------------------
	*/
	/*!
		@brief Fixed-width pack integers into as few 32-bit words as possible.
		@details
		Pack into 32-bit word a bunch of same-width integers, so:
				32 * 1-bit integers
				16 * 2-bit integers
				8 * 4-bit integers
				6 * 5-bit integers
				4 * 8-bit integers
				3 * 10-bit integers
				2 * 16-bit integers
				1 * 32-bit integers
		In this case we're using a 3-bit selector so only 8 of the 10 possible packings are used.
	*/
	class compress_integer_bitpack_32_reduced: public compress_integer_bitpack
		{
		private:
			/*
				COMPRESS_INTEGER_BITPACK_32_REDUCED::BITS_TO_USE_COMPLETE[]
				-----------------------------------------------------------
				Given the number of bits needed to store the integers, return the actual width to use.  This
				happens when, for example, you can get away with 9 bits, but since 9 * 3 = 27 and 10 * 3 = 30, you
				may as well use 10 bits.
			*/
			static const uint32_t bits_to_use_complete[]; 		///< bits to use (column 1 ) for bits in integer (right column)

			/*
				COMPRESS_INTEGER_BITPACK_32_REDUCED::SELECTOR_TO_USE_COMPLETE[]
				---------------------------------------------------------------
				Given the width in bits, which selector should be used?  This is used to ensure
				a switch() statement has all the entries 0..n with no gaps.
			*/
			static const uint32_t selector_to_use_complete[];			///< selector to use (column 1 ) for bits in integer (right column)

		public:
			/*
				COMPRESS_INTEGER_BITPACK_32_REDUCED::ENCODE()
				---------------------------------------------
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
				return compress_integer_bitpack::encode<32>(encoded, encoded_buffer_length, source, source_integers, bits_to_use_complete, selector_to_use_complete);
				}

			/*
				COMPRESS_INTEGER_BITPACK_32_REDUCED::DECODE()
				---------------------------------------------
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
				COMPRESS_INTEGER_BITPACK_32_REDUCED::UNITTEST()
				-----------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				compress_integer_bitpack_32_reduced *compressor = new compress_integer_bitpack_32_reduced;
				compress_integer::unittest(*compressor);
				delete compressor;
				puts("compress_integer_bitpack_32_reduced::PASSED");
				}
		};
	}


/*
	COMPRESS_INTEGER_BITPACK.H
	--------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Base class for bit-packing into sequences of 32-bit words.
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <stdint.h>
#include "maths.h"
#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_BITPACK
		------------------------------
	*/
	/*!
		@brief Fixed-width pack integers into as few 256-bit SIMD words as possible.
		@details
		Pack into n * 32-bit worda a bunch of same-width integers, so:
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
	class compress_integer_bitpack : public compress_integer
		{
		private:
			/*
				COMPRESS_INTEGER_BITPACK::BITS_TO_USE_COMPLETE[]
				------------------------------------------------
				Given the number of bits needed to store the integers, return the actual width to use.  This
				happens when, for example, you can get away with 9 bits, but since 9 * 3 = 27 and 10 * 3 = 30, you
				may as well use 10 bits.
			*/
			static const uint32_t bits_to_use_complete[]; 		///< bits to use (column 1 ) for bits in integer (right column)

			/*
				COMPRESS_INTEGER_BITPACK::SELECTOR_TO_USE_COMPLETE[]
				----------------------------------------------------
				Given the width in bits, which selector should be used?  This is used to ensure
				a switch() statement has all the entries 0..n with no gaps.
			*/
			static const uint32_t selector_to_use_complete[];			///< selector to use (column 1 ) for bits in integer (right column)

	public:
			/*
				BITS_NEEDED()
				-------------
			*/
			inline uint32_t bits_needed(uint32_t integer)
				{
				uint32_t current_width = maths::ceiling_log2(integer);
				return maths::maximum(current_width, static_cast<decltype(current_width)>(1));
				}

		public:
			/*
				COMPRESS_INTEGER_BITPACK::ENCODE()
				----------------------------------
			*/
			/*!
				@brief Encode a sequence of integers returning the number of bytes used for the encoding, or 0 if the encoded sequence doesn't fit in the buffer.
				@tparam WIDTH_IN_BITS The size of the machine word (in bits).
				@param encoded [out] The sequence of bytes that is the encoded sequence.
				@param encoded_buffer_length [in] The length (in bytes) of the output buffer, encoded.
				@param source [in] The sequence of integers to encode.
				@param source_integers [in] The length (in integers) of the source buffer.
				@return The number of bytes used to encode the integer sequence, or 0 on error (i.e. overflow).
			*/
			template <size_t WIDTH_IN_BITS>
			size_t encode(void *encoded, size_t encoded_buffer_length, const integer *array, size_t source_integers, const uint32_t *bits_to_use = bits_to_use_complete, const uint32_t *selector_to_use = selector_to_use_complete)
				{
				uint8_t *destination = (uint8_t *)encoded;

				while (1)
					{
					/*
						Find the widest integer in the list
					*/
					uint32_t widest = 0;
					const uint32_t *end = array + source_integers;
					for (const uint32_t *current = array; current < end; current++)
						{
						widest = maths::maximum(widest, bits_needed(*current));
						if (widest * (current - array) > WIDTH_IN_BITS)
							break;
						}

					/*
						Compute the width to use and how many integers to encode
					*/
					widest = bits_to_use[widest];
					end = array + (32 / widest) * (WIDTH_IN_BITS / 32);

					/*
						Now encode the integers
					*/
					uint32_t *buffer = (uint32_t *)(destination + 1);
					uint32_t current_word = 0;
					uint32_t placement = 0;
					for (const uint32_t *current = array; current < end; current++)
						{
						buffer[current_word] = buffer[current_word] | (*current << (widest * placement));
						current_word++;
						if (current_word >= (WIDTH_IN_BITS / 32))
							{
							current_word = 0;
							placement++;
							}
						}

					/*
						move on to the next word
					*/
					*destination = selector_to_use[widest];
					destination += (WIDTH_IN_BITS / 8) + 1;
					if (end - array > source_integers)
						break;
					source_integers -= end - array;
					array += end - array;
					}

				/*
					return the length of the encoded data (in bytes)
				*/
				return destination - (uint8_t *)encoded;
				}
		};
	}


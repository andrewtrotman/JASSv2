
/*
	COMPRESS_INTEGER_ELIAS_DELTA_SIMD.H
	------------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>
#include <stdint.h>
#include <strings.h>
#include <immintrin.h>

#include <bitset>
#include <vector>
#include <iostream>

#include "maths.h"
#include "compress_integer_elias_delta_simd.h"

#define WORD_WIDTH 32
#define WORDS (512 / WORD_WIDTH)

namespace JASS
	{
	static const uint32_t high_bits[] =
		{
		0b00000000000000000000000000000000,
		0b10000000000000000000000000000000,
		0b11000000000000000000000000000000,
		0b11100000000000000000000000000000,
		0b11110000000000000000000000000000,
		0b11111000000000000000000000000000,
		0b11111100000000000000000000000000,
		0b11111110000000000000000000000000,
		0b11111111000000000000000000000000,
		0b11111111100000000000000000000000,
		0b11111111110000000000000000000000,
		0b11111111111000000000000000000000,
		0b11111111111100000000000000000000,
		0b11111111111110000000000000000000,
		0b11111111111111000000000000000000,
		0b11111111111111100000000000000000,
		0b11111111111111110000000000000000,
		0b11111111111111111000000000000000,
		0b11111111111111111100000000000000,
		0b11111111111111111110000000000000,
		0b11111111111111111111000000000000,
		0b11111111111111111111100000000000,
		0b11111111111111111111110000000000,
		0b11111111111111111111111000000000,
		0b11111111111111111111111100000000,
		0b11111111111111111111111110000000,
		0b11111111111111111111111111000000,
		0b11111111111111111111111111100000,
		0b11111111111111111111111111110000,
		0b11111111111111111111111111111000,
		0b11111111111111111111111111111100,
		0b11111111111111111111111111111110,
		0b11111111111111111111111111111111
		};

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::PUSH_SELECTOR()
		--------------------------------------------------
	*/
	bool compress_integer_elias_delta_simd::push_selector(uint32_t *destination, uint8_t raw)
		{
		/*
			Write out the length of the selector in unary
		*/
		uint32_t width = maths::floor_log2(raw);
		selector_bits_used += width;

		/*
			Zig-zag and write out the selector in binary
		*/
		uint64_t zig_zag = ((raw & ~(1 << width)) << 1) + 1;
		accumulated_selector |= zig_zag << selector_bits_used;
		selector_bits_used += width + 1;

		/*
			Write out the low 32 bits of the selector if we've filled it up.
		*/
		if (selector_bits_used > 32)
			{
			*destination = accumulated_selector & 0xFFFFFFFFFFFFFFFF;
			accumulated_selector >>= 32;
			selector_bits_used -= 32;

			return true;
			}

		return false;
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::FLUSH()
		------------------------------------------
	*/
	void compress_integer_elias_delta_simd::flush(uint32_t *destination)
		{
		/*
			Flush the selector.
		*/
		uint32_t value_to_write = accumulated_selector & 0xFFFFFFFFFFFFFFFF;
		*destination = value_to_write;
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::DECODE_SELECTOR()
		----------------------------------------------------
		selector_bits_used = 0;
		accumulated_selector = 0;
	*/
	uint32_t compress_integer_elias_delta_simd::decode_selector(uint32_t *&selector_set)
		{
		/*
			If we need to read more bits if we need to.  As values 1..32 all fit within 11 bits, a read will happen less often than once per integer
		*/
		if (selector_bits_used >= 32)
			{
			selector_bits_used -= 32;
			uint64_t next_word = *selector_set++;
			accumulated_selector = accumulated_selector | (next_word << (32 - selector_bits_used));
			}

		/*
			Get the width
		*/
		uint64_t unary = _tzcnt_u64(accumulated_selector);

		/*
			Get the zig-zag encoded binary and unzig-zag it
		*/
		uint32_t decoded = (_bextr_u64(accumulated_selector, unary, unary + 1) >> 1) | (1UL << unary);

		/*
			Remember how much we've already used
		*/
		selector_bits_used += unary + unary + 1;
		accumulated_selector >>= unary + unary + 1;

		/*
			Return the decoded selector
		*/
		return decoded;
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::ENCODE()
		-------------------------------------------
	*/
	size_t compress_integer_elias_delta_simd::encode(void *encoded, size_t encoded_buffer_length, const integer *array, size_t elements)
		{
		uint32_t *destination = reinterpret_cast<uint32_t *>(encoded);
		uint32_t *end_of_destination = (uint32_t *)((uint8_t *)encoded + encoded_buffer_length);
		uint32_t carryover = 0;
		uint32_t actual_max_width = 0;

		/*
			get the address of the first selector and the first payload
		*/
		uint32_t *selector = destination;
		destination++;
		uint32_t *payload = destination;
		destination += WORDS;

		while (1)
			{
			/*
				Check for overflow of the output buffer
			*/
			if (destination > end_of_destination)
				return 0;

			/*
				Zero the result memory before writing the bit paterns into it
			*/
			memset(payload, 0, WORDS * 4);

			/*
				Encode the next integer
			*/
			uint32_t remaining = 32;
			uint32_t cumulative_shift = 0;
			bool overflow = false;
			uint32_t integers_encoded = 0;

			/*
				Find the width of this column
			*/
			for (uint32_t slice = 0; slice < 32; slice++)
				{
				uint32_t row_start = integers_encoded;
				uint32_t max_width = 0;
				for (uint32_t word = 0; word < WORDS; word++)
					{
					size_t index = slice * WORDS + word;
					uint32_t value;

					if (index < elements)
						{
						integers_encoded++;
						value = array[index];
						}
					else
						{
						if (!overflow)
							flush(selector);			// flush the selector

						overflow = true;
						value = 0;					// if we overflow then pad with 0s (so that we don't carryover stray bits)
						}

					uint32_t width = maths::ceiling_log2(value);
					width = maths::maximum(width, static_cast<decltype(width)>(1));					// coz ffs(0) != 1.
					max_width = maths::maximum(max_width, width);
					if (carryover == 0)
						payload[word] |= value << cumulative_shift;
					else
						payload[word] |= value & ~high_bits[32 - (actual_max_width - carryover)];
					}

				/*
					We push the selector width (max_width) here.
				*/
				if (push_selector(selector, max_width))
					selector = destination++;

				/*
					Manage the carryover
				*/
				actual_max_width = max_width;
				max_width -= carryover;
				carryover = 0;
				cumulative_shift += max_width;

				if (max_width > remaining)
					{
					/*
						We can't fit this column so take the high bits of the next set of integers
					*/
					integers_encoded = row_start;		// Wind back to the start of this row as its about to become the start of the next block.

					/*
						Encode the remaining high bits of the current integer set into the remaining space
					*/
					for (uint32_t word = 0; word < WORDS; word++)
						{
						size_t index = slice * WORDS + word;
						uint32_t value = index < elements ? array[index] : 0;
						payload[word] &= ~high_bits[remaining];
						uint32_t shift = actual_max_width - remaining;
						payload[word] |= (value >> shift) << (32 - remaining);
						}
					carryover = remaining;
					break;
					}
				else if (max_width == remaining || overflow || (slice + 1) * WORDS >= elements)
					break;

				remaining -= max_width;
				}

			payload = destination;
			destination += WORDS;
			array += integers_encoded;
			elements -= integers_encoded;
			if (elements == 0)
				break;
			}

		return  (uint8_t *)destination - (uint8_t *)encoded;
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::DECODE()
		-------------------------------------------
	*/
	void compress_integer_elias_delta_simd::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::UNITTEST()
		---------------------------------------------
	*/
	void compress_integer_elias_delta_simd::unittest(void)
		{
		const size_t end = 50;
		compress_integer_elias_delta_simd compressor;
		std::vector<uint8_t> mem(1024);

		uint32_t *into = (uint32_t *)mem.data();
		compressor.rewind();
		for (size_t x = 1; x <= end; x++)
			compressor.push_selector(into, x);

		compressor.flush(into);				// check the end case when we don't need to flush

		into = (uint32_t *)mem.data();
		compressor.rewind2();
		for (size_t x = 1; x <= end; x++)
			{
			uint32_t got = compressor.decode_selector(into);
			std::cout << x << "->" << got;
			if (x != got)
				std::cout << " WRONG";
			std::cout << '\n';
			}

exit(1);
		puts("compress_integer_prn_512_carryover::PASSED");
		}
	}

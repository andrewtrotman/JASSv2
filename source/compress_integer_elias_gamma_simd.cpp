/*
	COMPRESS_INTEGER_ELIAS_GAMMA_SIMD.CPP
	-------------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#include <vector>
#include <iostream>

#include "maths.h"
#include "compress_integer_elias_gamma_simd.h"

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
		COMPRESS_INTEGER_ELIAS_GAMMA_SIMD::COMPUTE_SELECTOR()
		-----------------------------------------------------
	*/
	uint32_t compress_integer_elias_gamma_simd::compute_selector(const uint8_t *encodings)
		{
		uint32_t value = 0;
		int current;

		for (current = 0; current < 32; current++)
			if (encodings[current] == 0)
				break;

		/*
			Compute the permutation number
		*/
		for (current--; current >=  0; current--)
			{
			size_t number_of_0s = encodings[current];
			value <<= number_of_0s;
			value |= 1 << (number_of_0s - 1);
			}

		return value;
		}

	/*
		COMPRESS_INTEGER_ELIAS_GAMMA_SIMD::ENCODE()
		-------------------------------------------
	*/
	size_t compress_integer_elias_gamma_simd::encode(void *encoded, size_t encoded_buffer_length, const integer *array, size_t elements)
		{
		uint8_t encodings[33] = {0};
		uint32_t *destination = (uint32_t *)encoded;
		uint32_t *end_of_destination = (uint32_t *)((uint8_t *)encoded + encoded_buffer_length);
		uint32_t carryover = 0;
		uint32_t actual_max_width = 0;

		while (1)
			{
			/*
				Check for overflow of the output buffer
			*/
			if (destination + WORDS + 1 + 1 > end_of_destination)
				return 0;

			/*
				Zero the result memory before writing the bit paterns into it
			*/
			memset(destination, 0, (WORDS + 1) * 4);

			/*
				Get the address of the selector and move on to the payload
			*/
			uint32_t *selector = destination;
			destination++;

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
						overflow = true;
						value = 0;					// if we overflow then pad with 0s (so that we don't carryover stray bits)
						}

					uint32_t width = maths::ceiling_log2(value);
					width = maths::maximum(width, static_cast<decltype(width)>(1));					// coz ffs(0) != 1.
					max_width = maths::maximum(max_width, width);
					if (carryover == 0)
						destination[word] |= value << cumulative_shift;
					else
						destination[word] |= value & ~high_bits[32 - (actual_max_width - carryover)];
					}
				actual_max_width = max_width;
				max_width -= carryover;
				carryover = 0;
				cumulative_shift += max_width;

				if (max_width > remaining)
					{
					/*
						We can't fit this column so take the high bits of the next set of integers
					*/
					encodings[slice] = 0;
					integers_encoded = row_start;		// Wind back to the start of this row as its about to become the start of the next block.

					/*
						Encode the remaining high bits of the current integer set into the remaining space
					*/
					for (uint32_t word = 0; word < WORDS; word++)
						{
						size_t index = slice * WORDS + word;
						uint32_t value = index < elements ? array[index] : 0;
						destination[word] &= ~high_bits[remaining];
						uint32_t shift = actual_max_width - remaining;
						destination[word] |= (value >> shift) << (32 - remaining);
						}
					carryover = remaining;
					break;
					}
				else if (max_width == remaining || overflow || (slice + 1) * WORDS >= elements)
					{
					/*
						We've filled a word without carryover.
					*/
					encodings[slice] = max_width;
					encodings[slice + 1] = 0;
					break;
					}

				remaining -= max_width;
				encodings[slice] = max_width;
				}
			*selector = compute_selector(encodings);

			destination += WORDS;
			array += integers_encoded;
			elements -= integers_encoded;
			if (elements == 0)
				break;
			}

		return  (uint8_t *)destination - (uint8_t *)encoded;
		}

	alignas(64)  const uint32_t compress_integer_elias_gamma_simd::mask_set[33][16]=
		{
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},								///< Sentinal as the selector cannot be 0.
		{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},								///< AND mask for 1-bit integers
		{0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03},								///< AND mask for 2-bit integers
		{0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07},								///< AND mask for 3-bit integers
		{0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f},								///< AND mask for 4-bit integers
		{0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f},								///< AND mask for 5-bit integers
		{0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f},								///< AND mask for 6-bit integers
		{0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f},								///< AND mask for 7-bit integers
		{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},								///< AND mask for 8-bit integers
		{0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff},						///< AND mask for 9-bit integers
		{0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff},						///< AND mask for 10-bit integers
		{0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff},						///< AND mask for 11-bit integers
		{0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff},						///< AND mask for 12-bit integers
		{0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff},			///< AND mask for 13-bit integers
		{0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff},			///< AND mask for 14-bit integers
		{0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff},			///< AND mask for 15-bit integers
		{0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff},			///< AND mask for 16-bit integers
		{0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff},								///< AND mask for 17-bit integers
		{0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff},								///< AND mask for 18-bit integers
		{0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff},								///< AND mask for 19-bit integers
		{0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff},								///< AND mask for 20-bit integers
		{0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff},								///< AND mask for 21-bit integers
		{0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff},								///< AND mask for 22-bit integers
		{0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff},								///< AND mask for 23-bit integers
		{0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff},								///< AND mask for 24-bit integers
		{0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff},						///< AND mask for 25-bit integers
		{0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff},						///< AND mask for 26-bit integers
		{0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff},						///< AND mask for 27-bit integers
		{0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff},						///< AND mask for 28-bit integers
		{0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff},			///< AND mask for 29-bit integers
		{0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff},			///< AND mask for 30-bit integers
		{0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff},			///< AND mask for 31-bit integers
		{0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff},			///< AND mask for 32-bit integers
		};

#ifdef __AVX512F__
		/*
			COMPRESS_INTEGER_ELIAS_GAMMA_SIMD::DECODE()
			-------------------------------------------
			AVX-512F version
		*/
		void compress_integer_elias_gamma_simd::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
			{
			__m512i mask;
			const uint8_t *source = (const uint8_t *)source_as_void;
			const uint8_t *end_of_source = source + source_length;
			__m512i *into = (__m512i *)decoded;

			uint64_t selector = *(uint32_t *)source;
			__m512i payload = _mm512_loadu_si512((__m512i *)(source + 4));
			source += 68;

			while (1)
				{
				uint32_t width = (uint32_t)find_first_set_bit(selector);
				//coverity[OVERRUN]
				mask = _mm512_loadu_si512((__m512i *)mask_set[width]);
				_mm512_storeu_si512(into, _mm512_and_si512(payload, mask));
				payload = _mm512_srli_epi32(payload, width);

				into++;
				//coverity[BAD_SHIFT]
				selector >>= width;

				while (selector == 0)
					{
					if (source >= end_of_source)
						return;

					/*
						Save the remaining bits
					*/
					__m512i high_bits = payload;

					/*
						move on to the next word
					*/
					selector = *(uint32_t *)source;
					payload = _mm512_loadu_si512((__m512i *)(source + 4));
					source += 68;

					/*
						get the low bits and write to memory
					*/
					width = (uint32_t)find_first_set_bit(selector);

					high_bits = _mm512_slli_epi32(high_bits, width);

					//coverity[OVERRUN]
					mask = _mm512_loadu_si512((__m512i *)mask_set[width]);
					_mm512_storeu_si512(into, _mm512_or_si512(_mm512_and_si512(payload, mask), high_bits));
					payload = _mm512_srli_epi32(payload, width);

					/*
						move on to the next slector
					*/
					into++;
					//coverity[BAD_SHIFT]
					selector >>= width;
					}
				}
			}
#elif defined(__AVX2__)
	/*
		COMPRESS_INTEGER_ELIAS_GAMMA_SIMD::DECODE()
		-------------------------------------------
		AVX2 version
	*/
	void compress_integer_elias_gamma_simd::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		__m256i mask;
		const uint8_t *source = (const uint8_t *)source_as_void;
		const uint8_t *end_of_source = source + source_length;
		__m256i *into = (__m256i *)decoded;

		uint64_t selector = *(uint32_t *)source;
		__m256i payload1 = _mm256_loadu_si256((__m256i *)(source + 4));
		__m256i payload2 = _mm256_loadu_si256((__m256i *)(source + 36));
		source += 68;

		while (1)
			{
			uint32_t width = (uint32_t)find_first_set_bit(selector);
			//coverity[OVERRUN]
			mask = _mm256_loadu_si256((__m256i *)mask_set[width]);
			_mm256_storeu_si256(into, _mm256_and_si256(payload1, mask));
			_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
			payload1 = _mm256_srli_epi32(payload1, width);
			payload2 = _mm256_srli_epi32(payload2, width);

			into += 2;
			//coverity[BAD_SHIFT]
			selector >>= width;

			while (selector == 0)
				{
				if (source >= end_of_source)
					return;

				/*
					Save the remaining bits
				*/
				__m256i high_bits1 = payload1;
				__m256i high_bits2 = payload2;

				/*
					move on to the next word
				*/
				selector = *(uint32_t *)source;
				payload1 = _mm256_loadu_si256((__m256i *)(source + 4));
				payload2 = _mm256_loadu_si256((__m256i *)(source + 36));
				source += 68;

				/*
					get the low bits and write to memory
				*/
				width = (uint32_t)find_first_set_bit(selector);

				high_bits1 = _mm256_slli_epi32(high_bits1, width);
				high_bits2 = _mm256_slli_epi32(high_bits2, width);

				//coverity[OVERRUN]
				mask = _mm256_loadu_si256((__m256i *)mask_set[width]);
				_mm256_storeu_si256(into, _mm256_or_si256(_mm256_and_si256(payload1, mask), high_bits1));
				_mm256_storeu_si256(into + 1, _mm256_or_si256(_mm256_and_si256(payload2, mask), high_bits2));
				payload1 = _mm256_srli_epi32(payload1, width);
				payload2 = _mm256_srli_epi32(payload2, width);

				/*
					move on to the next selctor
				*/
				into += 2;
				//coverity[BAD_SHIFT]
				selector >>= width;
				}
			}
		}

#else
	#error "Must have either AVX2 or AVX512"
#endif

	/*
		COMPRESS_INTEGER_ELIAS_GAMMA_SIMD::UNITTEST()
		---------------------------------------------
	*/
	void compress_integer_elias_gamma_simd::unittest(void)
		{
		compress_integer_elias_gamma_simd *compressor;

		compressor = new compress_integer_elias_gamma_simd();
		std::vector<std::string>pk;
		compressor->init(pk);
		compress_integer::unittest(*compressor);

		std::vector<uint32_t> broken_sequence =
			{
			6,10,2,1,2,1,1,1,1,2,2,1,1,14,1,1,		// 4 bits
			4,1,2,1,2,5,3,4,3,1,3,4,2,3,1,1,			// 3 bits
			6,13,5,1,2,8,4,2,5,1,1,1,2,1,1,2,		// 4 bits
			3,1,2,1,1,2,2,1,3,1,1,1,1,1,1,1,			// 2 bits
			1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,3,			// 2 bits
			1,7,1,4,5,3,2,1,10,1,8,1,2,5,1,24,		// 5 bits
			1,1,1,1,1,1,1,5,5,2,2,1,3,4,5,5,			// 3 bits
			2,4,2,2,1,1,1,2,2,1,2,1,2,1,3,3,			// 3 bits
			3,7,3,2,1,1,4,5,4,1,4,8,6,1,2,1,			// 4 bits
			1,1,1,1,1,3,1,2,1,1,1,1,1,1,1,2,			// 2 bits						// 160 integers

			1,3,2,2,3,1,2,1,1,2,1,1,1,1,1,2,			// 2 bits
			9,1,1,4,5,6,1,4,2,5,4,6,7,1,1,2,			// 4 bits
			1,1,9,2,2,1,2,1,1,1,1,1,1,1,1,1,			// 4 bits
			1,1,1,1,1,1,1,6,4,1,5,7,1,1,1,1,			// 3 bits
			2,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,			// 2 bits
			1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,			// 2 bits
			2,1,1,1,2,2,1,4,1,1,4,1,1,1,1,1,			// 3 bits
			1,1,1,1,1,2,5,3,1,3,1,1,4,1,2,1,			// 3 bits
			3,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,			// 2 bits						// 304 integers
			1,1,1,1,1,2,2,1,1,1,8,3,1,2,56,2,		// 6 bits (expand to 7)		// 320 integers

			12,1,6,70,68,25,13,44,36,22,4,95,19,5,39,8, // 7 bits
			25,14,9,8,27,6,1,1,8,11,8,3,4,1,2,8,			// 5 bits
			3,23,2,16,8,2,28,26,6,11,9,16,1,1,7,7,			// 5 bits
			45,2,33,39,20,14,2,1,8,26,1,10,12,3,16,3,		// 6 bits
			25,9,6,9,6,3,41,17,15,11,33,8,1,1,1,1			// 6 bits
			};

		unittest_one(*compressor, broken_sequence);

		std::vector<uint32_t> second_broken_sequence =
			{
			1, 1, 1, 793, 1, 1, 1, 1, 2, 1, 5, 3, 2, 1, 5, 63,		// 10 bits
			1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 5, 6, 2, 4, 1, 2,			// 3 bits
			1, 1, 1, 1, 4, 2, 1, 2, 2, 1, 1, 1, 3, 2, 2, 1,			// 3 bits
			1, 1, 2, 3, 1, 1, 8, 1, 1, 21, 2, 9, 15, 27, 7, 4,		// 5 bits
			2, 7, 1, 1, 2, 1, 1, 3, 2, 3, 1, 3, 3, 1, 2, 2,			// 3 bits
			3, 1, 3, 1, 2, 1, 2, 4, 1, 1, 3, 10, 1, 2, 1, 1,		// 4 bits
			6, 2, 1, 1, 3, 3, 7, 3, 2, 1, 2, 4, 3, 1, 2, 1,			// 3 bits <31 bits>, carryover 1 from next line
			6, 2, 2, 1															// 3 bits
			};
		unittest_one(*compressor, second_broken_sequence);

		puts("compress_integer_elias_gamma_simd::PASSED");
		}
	}

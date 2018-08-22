
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
std::cout << "[" << (int)raw << "]";
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
			std::cout << "<WS:" << *destination << ">";
			accumulated_selector >>= 32;
			selector_bits_used -= 32;

			return true;
			}

		return false;
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::ENCODE()
		-------------------------------------------
	*/
	size_t compress_integer_elias_delta_simd::encode(void *encoded, size_t encoded_buffer_length, const integer *array, size_t elements)
		{
		selector_bits_used = 0;
		accumulated_selector = 0;

		uint32_t *destination = reinterpret_cast<uint32_t *>(encoded);
		uint32_t *end_of_destination = (uint32_t *)((uint8_t *)encoded + encoded_buffer_length);
		uint32_t carryover = 0;
		uint32_t actual_max_width = 0;

		/*
			get the address of the first selector and the first payload
		*/
		uint32_t *selector = destination;
		destination++;
printf("S");
		uint32_t *payload = destination;
		destination += WORDS;
printf("P");
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
if (carryover == 0)
				if (push_selector(selector, max_width))
					{
					printf("S");
					selector = destination++;
					}

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
printf("P");
			array += integers_encoded;
			elements -= integers_encoded;
			if (elements == 0)
				break;
			}

		/*
			Flush the last selector.
		*/
		*selector = accumulated_selector & 0xFFFFFFFFFFFFFFFF;			// flush the final selector
std::cout << "<WSf:" << *selector << ">";

		return  (uint8_t *)destination - (uint8_t *)encoded;
		}

	alignas(64) static const uint32_t mask_set[33][16]=
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

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::DECODE_SELECTOR()
		----------------------------------------------------
		selector_bits_used = 0;
		accumulated_selector = 0;
	*/
	uint32_t compress_integer_elias_delta_simd::decode_selector(const uint32_t *&selector_set)
		{
		uint64_t unary;
		uint32_t decoded;

		/*
			Get the width
		*/
		if (accumulated_selector == 0)
			{
			/*
				The unary doesn't fit in the selector and is spread across 2 words
			*/
			uint64_t remainder = 32 - selector_bits_used;
			accumulated_selector = *selector_set++;
std::cout << "<RS:" << accumulated_selector << ">";
			unary = _tzcnt_u64(accumulated_selector);
			accumulated_selector >>= unary;
			selector_bits_used = unary;
			unary += remainder;

			}
		else
			{
			unary = _tzcnt_u64(accumulated_selector);
			selector_bits_used += unary;
			accumulated_selector >>= unary;
			}

		/*
			Get the zig-zag encoded binary and unzig-zag it
		*/
		if (selector_bits_used + unary > 32)
			{
			uint32_t low_bits = accumulated_selector;
			accumulated_selector = *selector_set++;
std::cout << "<RS:" << accumulated_selector << ">";
			uint32_t high_bits = _bextr_u64(accumulated_selector, 0, 32 - selector_bits_used);
			uint32_t zig_zag = (high_bits << (32 - selector_bits_used)) | low_bits;
			decoded = (zig_zag >> 1) | (1UL << unary);

			selector_bits_used = selector_bits_used + unary - 32;
			accumulated_selector >>= selector_bits_used;
			}
		else
			{
			decoded = (_bextr_u64(accumulated_selector, 0, unary + 1) >> 1) | (1UL << unary);
			selector_bits_used += unary + 1;
			accumulated_selector >>= unary + 1;
			}

		/*
			Return the decoded selector
		*/
std::cout << "[" << decoded << "]";
		return decoded;
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::DECODE()
		-------------------------------------------
	*/
	void compress_integer_elias_delta_simd::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		uint32_t used = 0;
		__m256i mask;
		const uint32_t *source = reinterpret_cast<const uint32_t *>(source_as_void);
		const uint32_t *end_of_source = reinterpret_cast<const uint32_t *>(reinterpret_cast<const uint8_t *>(source_as_void) + source_length);
		__m256i *into = (__m256i *)decoded;

		/*
			Set up the initial selector
		*/
		selector_bits_used = 0;
		accumulated_selector = *source;
std::cout << "\n<RS:" << accumulated_selector << ">";

		/*
			Set up the initial payload
		*/
std::cout << "P";
		__m256i payload1 = _mm256_loadu_si256((__m256i *)(source + 1));
		__m256i payload2 = _mm256_loadu_si256((__m256i *)(source + 9));
		source += 17;

		while (source < end_of_source)
			{
			uint32_t width = decode_selector(source);

if (width == 6)
{
int x = 0;
}

			if (used + width > 32)
				{
				/*
					Save the remaining bits
				*/
				__m256i high_bits1 = payload1;
				__m256i high_bits2 = payload2;

				/*
					move on to the next word
				*/
				payload1 = _mm256_loadu_si256((__m256i *)(source));
				payload2 = _mm256_loadu_si256((__m256i *)(source + 8));
				source += 16;
std::cout << "P";

				/*
					get the low bits and write to memory
				*/
				high_bits1 = _mm256_slli_epi32(high_bits1, width - (32 - used));
				high_bits2 = _mm256_slli_epi32(high_bits2, width - (32 - used));

				mask = _mm256_loadu_si256((__m256i *)mask_set[used + width - 32]);		// the remaining bits in the AVX word
				_mm256_storeu_si256(into, _mm256_or_si256(_mm256_and_si256(payload1, mask), high_bits1));
				_mm256_storeu_si256(into + 1, _mm256_or_si256(_mm256_and_si256(payload2, mask), high_bits2));
				payload1 = _mm256_srli_epi32(payload1, used + width - 32);
				payload2 = _mm256_srli_epi32(payload2, used + width - 32);

				used = used + width - 32;
				into += 2;
				}
			else
				{
				mask = _mm256_loadu_si256((__m256i *)mask_set[width]);
				_mm256_storeu_si256(into, _mm256_and_si256(payload1, mask));
				_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
				payload1 = _mm256_srli_epi32(payload1, width);
				payload2 = _mm256_srli_epi32(payload2, width);

				used += width;
				into += 2;
				}
		}
	}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::UNITTEST()
		---------------------------------------------
	*/
	void compress_integer_elias_delta_simd::unittest(void)
		{
		compress_integer_elias_delta_simd compressor;

//		compress_integer::unittest(compress_integer_elias_delta_simd());

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
			1,1,1,1,1,3,1,2,1,1,1,1,1,1,1,2,			// 2 bits						// 160 integers (1 complete AVX-512 word).

			1,3,2,2,3,1,2,1,1,2,1,1,1,1,1,2,			// 2 bits
			9,1,1,4,5,6,1,4,2,5,4,6,7,1,1,2,			// 4 bits
			1,1,9,2,2,1,2,1,1,1,1,1,1,1,1,1,			// 4 bits
			1,1,1,1,1,1,1,6,4,1,5,7,1,1,1,1,			// 3 bits
			2,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,			// 2 bits
			1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,			// 2 bits
			2,1,1,1,2,2,1,4,1,1,4,1,1,1,1,1,			// 3 bits
			1,1,1,1,1,2,5,3,1,3,1,1,4,1,2,1,			// 3 bits
			3,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,			// 2 bits						// 304 integers
			1,1,1,1,1,2,2,1,1,1,8,3,1,2,56,2,		// 6 bits						// 320 integers

			12,1,6,70,68,25,13,44,36,22,4,95,19,5,39,8, // 7 bits
			25,14,9,8,27,6,1,1,8,11,8,3,4,1,2,8,			// 5 bits
			3,23,2,16,8,2,28,26,6,11,9,16,1,1,7,7,			// 5 bits
			45,2,33,39,20,14,2,1,8,26,1,10,12,3,16,3,		// 6 bits
			25,9,6,9,6,3,41,17,15,11,33,8,1,1,1,1			// 6 bits
			};

puts("\n");
		unittest_one(compressor, broken_sequence);
puts("\n");

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
puts("\n");
		unittest_one(compressor, second_broken_sequence);

exit(1);
		puts("compress_integer_prn_512_carryover::PASSED");
		}
	}

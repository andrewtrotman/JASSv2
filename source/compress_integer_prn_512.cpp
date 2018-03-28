/*
	COMPRESS_INTEGER_PRN_512.H
	--------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>
#include <stdint.h>
#include <strings.h>
#include <immintrin.h>

#include <vector>
#include <iostream>

#include "maths.h"
#include "compress_integer_prn_512.h"
#include "compress_integer_lyck_16.h"
#include "compress_integer_nybble_8.h"
#include "compress_integer_bitpack_64.h"
#include "compress_integer_bitpack_256.h"
#include "compress_integer_bitpack_128.h"
#include "compress_integer_bitpack_32_reduced.h"

#define WORD_WIDTH 32
#define WORDS (512 / WORD_WIDTH)

namespace JASS
	{
	/*
		COMPRESS_INTEGER_PRN_512::COMPUTE_SELECTOR()
		--------------------------------------------
	*/
	uint32_t compress_integer_prn_512::compute_selector(const uint8_t *encodings)
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
		COMPRESS_INTEGER_PRN_512::ENCODE()
		----------------------------------
	*/
	size_t compress_integer_prn_512::encode(void *encoded, size_t encoded_buffer_length, const integer *array, size_t elements)
		{
		uint8_t encodings[33] = {0};
		uint32_t *destination = (uint32_t *)encoded;
		uint32_t *end_of_destination = (uint32_t *)((uint8_t *)encoded + encoded_buffer_length);

		while (1)
			{
			/*
				Check for overflow of the output buffer
			*/
			if (destination + WORDS + 1 + 1 > end_of_destination)
				return 0;

			/*
				Compute the encoding
			*/
			uint32_t remaining = 32;
			bool overflow = false;
			for (uint32_t slice = 0; slice < 32; slice++)
				{
				uint32_t max_width = 0;
				for (uint32_t word = 0; word < WORDS; word++)
					{
					size_t index = slice * WORDS + word;
					uint32_t value;

					if (index < elements)
						value = array[index];
					else
						{
						overflow = true;
						value = 1;
						}

					uint32_t width = maths::ceiling_log2(value);
					width = maths::maximum(width, static_cast<decltype(width)>(1));					// coz ffs(0) != 1.
					max_width = maths::maximum(max_width, width);
					}

				if (max_width > remaining)
					{
					/*
						We can't fit this column so pad the previous width to the full 32-bits then mark this as end of list
					*/
					encodings[slice - 1] += remaining;
					encodings[slice] = 0;
					break;
					}
				else if (max_width == remaining || overflow || (slice + 1) * WORDS >= elements)
					{
					/*
						We're past the end of the input data to pad the current slice and mark this as the end of the list.
					*/
					encodings[slice] = remaining;
					encodings[slice + 1] = 0;
					break;
					}
				remaining -= max_width;
				encodings[slice] = max_width;
				}

			/*
				Compute the selector
			*/
			*destination++ = compute_selector(encodings);

			/*
				Encode the answer
			*/
			memset(destination, 0, WORDS * 4);
			uint32_t position_in_word = 0;
			uint32_t cumulative_shift = 0;
			uint32_t integers_encoded = 0;
//			printf("\n");
			for (uint32_t slice = 0; encodings[slice] != 0; slice++)
				{
				uint32_t width = encodings[position_in_word];
				position_in_word++;
				for (size_t word = 0; word < WORDS; word++)
					{
					size_t index = slice * WORDS + word;
					uint32_t value;
					if (index < elements)
						{
						integers_encoded++;
						value = array[index];
						}
					else
						value = 0;

					destination[word] |= value << cumulative_shift;
//					printf("%08X ", value << cumulative_shift);
					}
				cumulative_shift += width;
//				printf(" [%02d]\n", width);

				}
			destination += WORDS;
			array += integers_encoded;
			elements -= integers_encoded;
			if (elements == 0)
				break;
			}

		/*
			return the number of bytes that were used to encode the integer sequence.
		*/
		return (uint8_t *)destination - (uint8_t *)encoded;
		}

	alignas(64) static uint32_t static_mask_32[] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};			///< AND mask for 32-bit integers
	alignas(64) static uint32_t static_mask_31[] = {0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff};			///< AND mask for 31-bit integers
	alignas(64) static uint32_t static_mask_30[] = {0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff};			///< AND mask for 30-bit integers
	alignas(64) static uint32_t static_mask_29[] = {0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff};			///< AND mask for 29-bit integers
	alignas(64) static uint32_t static_mask_28[] = {0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff};						///< AND mask for 28-bit integers
	alignas(64) static uint32_t static_mask_27[] = {0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff};						///< AND mask for 27-bit integers
	alignas(64) static uint32_t static_mask_26[] = {0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff};						///< AND mask for 26-bit integers
	alignas(64) static uint32_t static_mask_25[] = {0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff};						///< AND mask for 25-bit integers
	alignas(64) static uint32_t static_mask_24[] = {0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff};								///< AND mask for 24-bit integers
	alignas(64) static uint32_t static_mask_23[] = {0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff};								///< AND mask for 23-bit integers
	alignas(64) static uint32_t static_mask_22[] = {0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff};								///< AND mask for 22-bit integers
	alignas(64) static uint32_t static_mask_21[] = {0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff};								///< AND mask for 21-bit integers
	alignas(64) static uint32_t static_mask_20[] = {0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff};								///< AND mask for 20-bit integers
	alignas(64) static uint32_t static_mask_19[] = {0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff};								///< AND mask for 19-bit integers
	alignas(64) static uint32_t static_mask_18[] = {0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff};								///< AND mask for 18-bit integers
	alignas(64) static uint32_t static_mask_17[] = {0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff};								///< AND mask for 17-bit integers
	alignas(64) static uint32_t static_mask_16[] = {0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff};			///< AND mask for 16-bit integers
	alignas(64) static uint32_t static_mask_15[] = {0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff};			///< AND mask for 15-bit integers
	alignas(64) static uint32_t static_mask_14[] = {0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff};			///< AND mask for 14-bit integers
	alignas(64) static uint32_t static_mask_13[] = {0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff};			///< AND mask for 13-bit integers
	alignas(64) static uint32_t static_mask_12[] = {0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff};						///< AND mask for 12-bit integers
	alignas(64) static uint32_t static_mask_11[] = {0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff};						///< AND mask for 11-bit integers
	alignas(64) static uint32_t static_mask_10[] = {0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff};						///< AND mask for 10-bit integers
	alignas(64) static uint32_t static_mask_9[] =  {0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff};						///< AND mask for 9-bit integers
	alignas(64) static uint32_t static_mask_8[] =  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};								///< AND mask for 8-bit integers
	alignas(64) static uint32_t static_mask_7[]  = {0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f};								///< AND mask for 7-bit integers
	alignas(64) static uint32_t static_mask_6[]  = {0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f};								///< AND mask for 6-bit integers
	alignas(64) static uint32_t static_mask_5[]  = {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};								///< AND mask for 5-bit integers
	alignas(64) static uint32_t static_mask_4[]  = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f};								///< AND mask for 4-bit integers
	alignas(64) static uint32_t static_mask_3[]  = {0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07};								///< AND mask for 3-bit integers
	alignas(64) static uint32_t static_mask_2[]  = {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};								///< AND mask for 2-bit integers
	alignas(64) static uint32_t static_mask_1[]  = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};								///< AND mask for 1-bit integers

	/*
		COMPRESS_INTEGER_PRN_512::DECODE()
		----------------------------------
	*/
	void compress_integer_prn_512::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		__m256i mask;
		const uint8_t *source = (const uint8_t *)source_as_void;
		__m256i *into = (__m256i *)decoded;

		uint64_t selector = *(uint32_t *)source;
		source += 4;
		__m256i payload1 = _mm256_loadu_si256((__m256i *)source);
		source += 32;
		__m256i payload2 = _mm256_loadu_si256((__m256i *)source);
		source += 32;

		while (1)
			{
			switch (find_first_set_bit(selector))
				{
				case 0:
					if (source >= ((const uint8_t *)source_as_void) + source_length)
						return;

					selector = *(uint32_t *)source;
					source += 4;
					payload1 = _mm256_loadu_si256((__m256i *)source);
					source += 32;
					payload2 = _mm256_loadu_si256((__m256i *)source);
					source += 32;
					continue;
				case 1:			// 16 * 1-bit integers
					mask = _mm256_loadu_si256((__m256i *)static_mask_1);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 1);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 1);

					into += 2;
					decoded += 16;
					selector >>= 1;
					continue;
				case 2:
					mask = _mm256_loadu_si256((__m256i *)static_mask_2);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 2);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 2);

					into += 2;
					decoded += 16;
					selector >>= 2;
					continue;
				case 3:
					mask = _mm256_loadu_si256((__m256i *)static_mask_3);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 3);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 3);

					into += 2;
					decoded += 16;
					selector >>= 3;
					continue;
				case 4:
					mask = _mm256_loadu_si256((__m256i *)static_mask_4);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 4);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 4);

					into += 2;
					decoded += 16;
					selector >>= 4;
					continue;
				case 5:
					mask = _mm256_loadu_si256((__m256i *)static_mask_5);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 5);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 5);

					into += 2;
					decoded += 16;
					selector >>= 5;
					continue;
				case 6:
					mask = _mm256_loadu_si256((__m256i *)static_mask_6);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 6);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 6);

					into += 2;
					decoded += 16;
					selector >>= 6;
					continue;
				case 7:
					mask = _mm256_loadu_si256((__m256i *)static_mask_7);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 7);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 7);

					into += 2;
					decoded += 16;
					selector >>= 7;
					continue;
				case 8:
					mask = _mm256_loadu_si256((__m256i *)static_mask_8);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 8);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 8);

					into += 2;
					decoded += 16;
					selector >>= 8;
					continue;
				case 9:
					mask = _mm256_loadu_si256((__m256i *)static_mask_9);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 9);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 9);

					into += 2;
					decoded += 16;
					selector >>= 9;
					continue;
				case 10:
					mask = _mm256_loadu_si256((__m256i *)static_mask_10);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 10);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 10);

					into += 2;
					decoded += 16;
					selector >>= 10;
					continue;
				case 11:
					mask = _mm256_loadu_si256((__m256i *)static_mask_11);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 11);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 11);

					into += 2;
					decoded += 16;
					selector >>= 11;
					continue;
				case 12:
					mask = _mm256_loadu_si256((__m256i *)static_mask_12);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 12);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 12);

					into += 2;
					decoded += 16;
					selector >>= 12;
					continue;
				case 13:
					mask = _mm256_loadu_si256((__m256i *)static_mask_13);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 13);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 13);

					into += 2;
					decoded += 16;
					selector >>= 13;
					continue;
				case 14:
					mask = _mm256_loadu_si256((__m256i *)static_mask_14);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 14);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 14);

					into += 2;
					decoded += 16;
					selector >>= 14;
					continue;
				case 15:
					mask = _mm256_loadu_si256((__m256i *)static_mask_15);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 15);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 15);

					into += 2;
					decoded += 16;
					selector >>= 15;
					continue;
				case 16:
					mask = _mm256_loadu_si256((__m256i *)static_mask_16);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 16);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 16);

					into += 2;
					decoded += 16;
					selector >>= 16;
					continue;
				case 17:
					mask = _mm256_loadu_si256((__m256i *)static_mask_17);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 17);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 17);

					into += 2;
					decoded += 16;
					selector >>= 17;
					continue;
				case 18:
					mask = _mm256_loadu_si256((__m256i *)static_mask_18);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 18);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 18);

					into += 2;
					decoded += 16;
					selector >>= 18;
					continue;
				case 19:
					mask = _mm256_loadu_si256((__m256i *)static_mask_19);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 19);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 19);

					into += 2;
					decoded += 16;
					selector >>= 19;
					continue;
				case 20:
					mask = _mm256_loadu_si256((__m256i *)static_mask_20);

					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 20);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 20);

					into += 2;
					decoded += 16;
					selector >>= 20;
					continue;
				case 21:
					mask = _mm256_loadu_si256((__m256i *)static_mask_21);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 21);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 21);

					into += 2;
					decoded += 16;
					selector >>= 21;
					continue;
				case 22:
					mask = _mm256_loadu_si256((__m256i *)static_mask_22);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 22);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 22);

					into += 2;
					decoded += 16;
					selector >>= 22;
					continue;
				case 23:
					mask = _mm256_loadu_si256((__m256i *)static_mask_23);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 23);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 23);

					into += 2;
					decoded += 16;
					selector >>= 23;
					continue;
				case 24:
					mask = _mm256_loadu_si256((__m256i *)static_mask_24);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 24);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 24);

					into += 2;
					decoded += 16;
					selector >>= 24;
					continue;
				case 25:
					mask = _mm256_loadu_si256((__m256i *)static_mask_25);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 25);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 25);

					into += 2;
					decoded += 16;
					selector >>= 25;
					continue;
				case 26:
					mask = _mm256_loadu_si256((__m256i *)static_mask_26);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 26);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 26);

					into += 2;
					decoded += 16;
					selector >>= 26;
					continue;
				case 27:
					mask = _mm256_loadu_si256((__m256i *)static_mask_27);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 27);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 27);

					into += 2;
					decoded += 16;
					selector >>= 27;
					continue;
				case 28:
					mask = _mm256_loadu_si256((__m256i *)static_mask_28);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 28);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 28);

					into += 2;
					decoded += 16;
					selector >>= 28;
					continue;
				case 29:
					mask = _mm256_loadu_si256((__m256i *)static_mask_29);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 29);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 29);

					into += 2;
					decoded += 16;
					selector >>= 29;
					continue;
				case 30:
					mask = _mm256_loadu_si256((__m256i *)static_mask_30);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 30);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 30);

					into += 2;
					decoded += 16;
					selector >>= 30;
					continue;
				case 31:
					mask = _mm256_loadu_si256((__m256i *)static_mask_31);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 31);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 31);

					into += 2;
					decoded += 16;
					selector >>= 31;
					continue;
				case 32:
					mask = _mm256_loadu_si256((__m256i *)static_mask_32);
					_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
					payload1 = _mm256_srli_epi32(payload1, 32);
					_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
					payload2 = _mm256_srli_epi32(payload2, 32);

					into += 2;
					decoded += 16;
					selector >>= 32;
					continue;
				default:
					continue;
				}
			}
		}

	/*
		COMPRESS_INTEGER_PRN_512::UNITTEST_ONE()
		----------------------------------------
	*/
	void compress_integer_prn_512::unittest_one(const std::vector<uint32_t> &sequence)
		{
		compress_integer_prn_512 compressor;
		std::vector<uint32_t>compressed(sequence.size() * 5 + 1024);
		std::vector<uint32_t>decompressed(sequence.size() + 512);

		auto size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &sequence[0], sequence.size());
		compressor.decode(&decompressed[0], sequence.size(), &compressed[0], size_once_compressed);
		decompressed.resize(sequence.size());
		JASS_assert(decompressed == sequence);
		}

	/*
		COMPRESS_INTEGER_PRN_512::UNITTEST()
		------------------------------------
	*/
	void compress_integer_prn_512::unittest(void)
		{
		std::vector<uint32_t> every_case;
		size_t instance;

		/*
			1-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 32 * 8; instance++)
			every_case.push_back(0x01);
		unittest_one(every_case);

		/*
			2-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 16 * 8; instance++)
			every_case.push_back(0x03);
		unittest_one(every_case);

		/*
			3-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 10 * 8; instance++)
			every_case.push_back(0x07);
		unittest_one(every_case);

		/*
			4-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 8 * 8; instance++)
			every_case.push_back(0x0F);
		unittest_one(every_case);

		/*
			5-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 6 * 8; instance++)
			every_case.push_back(0x1F);
		unittest_one(every_case);

		/*
			6-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 5 * 8; instance++)
			every_case.push_back(0x3F);
		unittest_one(every_case);

		/*
			8-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 4 * 8; instance++)
			every_case.push_back(0xFF);
		unittest_one(every_case);

		/*
			10-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 3 * 8; instance++)
			every_case.push_back(0x3FF);
		unittest_one(every_case);

		/*
			16-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 2 * 8; instance++)
			every_case.push_back(0xFFFF);
		unittest_one(every_case);

		/*
			32-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 3 /* * 8*/; instance++)
			every_case.push_back(0x3FF);
		unittest_one(every_case);

		every_case.clear();
		every_case.push_back(0x3F1);
		every_case.push_back(0x3F2);
		every_case.push_back(0x3F3);
		unittest_one(every_case);

		std::vector<uint32_t> broken_sequence = {
			6,10,2,1,2,1,1,1,1,2,2,1,1,14,1,1,		// 4 bits
			4,1,2,1,2,5,3,4,3,1,3,4,2,3,1,1,			// 3 bits
			6,13,5,1,2,8,4,2,5,1,1,1,2,1,1,2,		// 4 bits
			3,1,2,1,1,2,2,1,3,1,1,1,1,1,1,1,			// 2 bits
			1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,3,			// 2 bits
			1,7,1,4,5,3,2,1,10,1,8,1,2,5,1,24,		// 5 bits
			1,1,1,1,1,1,1,5,5,2,2,1,3,4,5,5,			// 3 bits
			2,4,2,2,1,1,1,2,2,1,2,1,2,1,3,3,			// 3 bits
			3,7,3,2,1,1,4,5,4,1,4,8,6,1,2,1,			// 4 bits
			1,1,1,1,1,3,1,2,1,1,1,1,1,1,1,2,			// 2 bits

			1,3,2,2,3,1,2,1,1,2,1,1,1,1,1,2,			// 2 bits
			9,1,1,4,5,6,1,4,2,5,4,6,7,1,1,2,			// 4 bits
			1,1,9,2,2,1,2,1,1,1,1,1,1,1,1,1,			// 4 bits
			1,1,1,1,1,1,1,6,4,1,5,7,1,1,1,1,			// 3 bits
			2,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,			// 2 bits
			1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,			// 2 bits
			2,1,1,1,2,2,1,4,1,1,4,1,1,1,1,1,			// 3 bits
			1,1,1,1,1,2,5,3,1,3,1,1,4,1,2,1,			// 3 bits
			3,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,			// 2 bits
			1,1,1,1,1,2,2,1,1,1,8,3,1,2,56,2,		// 6 bits
			12,1,6,70,68,25,13,44,36,22,4,95,19,5,39,8, // 7 bits
			25,14,9,8,27,6,1,1,8,11,8,3,4,1,2,8,3,23,2,16,8,2,28,26,6,11,9,16,1,1,7,7,45,2,33,39,20,14,2,1,8,26,1,10,12,3,16,3,25,9,6,9,6,3,41,17,15,11,33,8,1,1,1,1};
		unittest_one(broken_sequence);

		puts("compress_integer_prn_512::PASSED");
		}
	}

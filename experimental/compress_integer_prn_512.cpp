/*
	COMPRESS_INTEGER_PRN_512.H
	--------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdint.h>
#include <strings.h>
#include <immintrin.h>

#include <vector>
#include <iostream>

#include "maths.h"
#include "compress_integer_prn_512.h"
#include "compress_integer_bitpack_256.h"
#include "compress_integer_bitpack_128.h"
#include "compress_integer_bitpack_64.h"
#include "compress_integer_bitpack_32_reduced.h"
#include "compress_integer_lyck_16.h"
#include "compress_integer_nybble_8.h"

#define WORD_WIDTH 32
#define WORDS (256 / WORD_WIDTH)

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

		for (current = 32; current >  0; current--)
			if (encodings[current] != 0)
				break;

		/*
			Compute the permutation number
		*/
		for (;current >=  0; current--)
			{
			size_t number_of_0s = encodings[current];
			value <<= number_of_0s;
			value |= 1 << (number_of_0s - 1);
			}

		return value;
		}

	/*
		COMPRESS_INTEGER_PRN_512::ESTIMATE()
		------------------------------------
	*/
	uint32_t compress_integer_prn_512::estimate(const uint32_t *array, size_t elements)
		{
		const uint32_t *current;
		uint32_t width = 0;

		for (current = array; current < array + elements; current++)
			{
			uint32_t current_width = JASS::maths::ceiling_log2(*current);
			current_width = JASS::maths::maximum(current_width, static_cast<decltype(current_width)>(1));

			width += current_width;
			if (width > WORD_WIDTH)
				break;
			}
		return current - array;
		}

	/*
		COMPRESS_INTEGER_PRN_512::TEST_ENCODING()
		-----------------------------------------
	*/
	bool compress_integer_prn_512::test_encoding(uint8_t *encodings, const uint32_t *array, size_t elements_in_array, size_t elements)
		{
		uint32_t width = 0;
		const uint32_t *current;

		for (current = array; current < array + elements; current++)
			{
			uint32_t max_width = 0;
			for (uint32_t word = 0; word < WORDS; word++)
				{
				uint32_t current_width;
				if ((current + word * elements) >= array + elements_in_array)
					current_width = 0;
				else
					{
					current_width = JASS::maths::ceiling_log2(*(current + word * elements));
					current_width = JASS::maths::maximum(current_width, static_cast<decltype(current_width)>(1));
					}
				max_width = JASS::maths::maximum(max_width, current_width);
				}
			width += max_width;
			if (width > WORD_WIDTH)
				{
				encodings[current - array] = 0;
				return false;
				}
			encodings[current - array] = max_width;
			}
		encodings[current - array] = 0;
		return true;
		}

	/*
		COMPRESS_INTEGER_PRN_512::ENCODE()
		----------------------------------
	*/
	void compress_integer_prn_512::encode(uint32_t *array, size_t elements)
		{
		uint8_t encodings[33];
		uint32_t *current;
		uint32_t integers_to_encode;

		/*
			Get the initial guess
		*/
		uint32_t initial_guess = estimate(array, elements);
		std::cout << "initial:" << initial_guess << "\n";

		/*
			Linear search for the best answer
		*/
		for (integers_to_encode = initial_guess; integers_to_encode > 1; integers_to_encode--)
			if (test_encoding(encodings, array, elements, integers_to_encode))
				break;

		/*
			Dump the answer
		*/
		for (size_t word = 0; word < WORDS; word++)
			{
			for (current = array + word * integers_to_encode; current < array + (word + 1) * integers_to_encode; current++)
				if (current < array + elements)
					printf("%08X ", *current);
				else
					printf("     (0) ");

			printf("\n");
			}

		printf("=\n");

		for (current = array; current < array + integers_to_encode; current++)
			printf("%08u ", encodings[current - array]);
		printf("\n");

		compute_selector(encodings);
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
		const uint8_t *source;
		__m256i *into = (__m256i *)decoded;

		uint32_t selector = *(uint32_t *)source;
		source += 4;
		__m256i payload1 = _mm256_loadu_si256((__m256i *)source);
		source += 32;
		__m256i payload2 = _mm256_loadu_si256((__m256i *)source);
		source += 32;

		while (1)
			{
			uint32_t shift = find_first_set_bit(selector);
			switch (shift)
				{
				case 0:
					selector = *(uint32_t *)source;
					if (selector == 0)
						return;							// at present we terminate on a 0 selector (fix this).
					source += 4;
					payload1 = _mm256_loadu_si256((__m256i *)source);
					source += 32;
					payload2 = _mm256_loadu_si256((__m256i *)source);
					source += 32;
					break;
				case 1:			// 16 * 1-bit integers
					mask = _mm256_loadu_si256((__m256i *)static_mask_1);
					break;
				case 2:
					mask = _mm256_loadu_si256((__m256i *)static_mask_2);
					break;
				case 3:
					mask = _mm256_loadu_si256((__m256i *)static_mask_3);
					break;
				case 4:
					mask = _mm256_loadu_si256((__m256i *)static_mask_4);
					break;
				case 5:
					mask = _mm256_loadu_si256((__m256i *)static_mask_5);
					break;
				case 6:
					mask = _mm256_loadu_si256((__m256i *)static_mask_6);
					break;
				case 7:
					mask = _mm256_loadu_si256((__m256i *)static_mask_7);
					break;
				case 8:
					mask = _mm256_loadu_si256((__m256i *)static_mask_8);
					break;
				case 9:
					mask = _mm256_loadu_si256((__m256i *)static_mask_9);
					break;
				case 10:
					mask = _mm256_loadu_si256((__m256i *)static_mask_10);
					break;
				case 11:
					mask = _mm256_loadu_si256((__m256i *)static_mask_11);
					break;
				case 12:
					mask = _mm256_loadu_si256((__m256i *)static_mask_12);
					break;
				case 13:
					mask = _mm256_loadu_si256((__m256i *)static_mask_13);
					break;
				case 14:
					mask = _mm256_loadu_si256((__m256i *)static_mask_14);
					break;
				case 15:
					mask = _mm256_loadu_si256((__m256i *)static_mask_15);
					break;
				case 16:
					mask = _mm256_loadu_si256((__m256i *)static_mask_16);
					break;
				case 17:
					mask = _mm256_loadu_si256((__m256i *)static_mask_17);
					break;
				case 18:
					mask = _mm256_loadu_si256((__m256i *)static_mask_18);
					break;
				case 19:
					mask = _mm256_loadu_si256((__m256i *)static_mask_19);
					break;
				case 20:
					mask = _mm256_loadu_si256((__m256i *)static_mask_20);
					break;
				case 21:
					mask = _mm256_loadu_si256((__m256i *)static_mask_21);
					break;
				case 22:
					mask = _mm256_loadu_si256((__m256i *)static_mask_22);
					break;
				case 23:
					mask = _mm256_loadu_si256((__m256i *)static_mask_23);
					break;
				case 24:
					mask = _mm256_loadu_si256((__m256i *)static_mask_24);
					break;
				case 25:
					mask = _mm256_loadu_si256((__m256i *)static_mask_25);
					break;
				case 26:
					mask = _mm256_loadu_si256((__m256i *)static_mask_26);
					break;
				case 27:
					mask = _mm256_loadu_si256((__m256i *)static_mask_27);
					break;
				case 28:
					mask = _mm256_loadu_si256((__m256i *)static_mask_28);
					break;
				case 29:
					mask = _mm256_loadu_si256((__m256i *)static_mask_29);
					break;
				case 30:
					mask = _mm256_loadu_si256((__m256i *)static_mask_30);
					break;
				case 31:
					mask = _mm256_loadu_si256((__m256i *)static_mask_31);
					break;
				case 32:
					mask = _mm256_loadu_si256((__m256i *)static_mask_32);
					break;
				}

			_mm256_storeu_si256(into + 0, _mm256_and_si256(payload1, mask));
			payload1 = _mm256_srli_epi32(payload1, shift);
			_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
			payload2 = _mm256_srli_epi32(payload2, shift);
			into += 2;
			decoded += 16;
			selector >>= shift;
			}
		}
	}













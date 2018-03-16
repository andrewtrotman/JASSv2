/*
	COMPRESS_INTEGER_BITPACK_256.H
	------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdint.h>
#include <immintrin.h>

#include <vector>

#include "maths.h"
#include "asserts.h"
#include "compress_integer_bitpack_256.h"

namespace JASS
	{
	/*
		BITS_TO_USE[]
		-------------
		Given the number of bits needed to store the integers, return the actual width to use.  This
		happens when, for example, you can get away with 9 bits, but since 9 * 3 = 27 and 10 * 3 = 30, you
		may as well use 10 bits.
	*/
	static uint32_t bits_to_use[] =			///< bits to use (column 1 ) for bits in integer (right column)
		{
		1, //0
		1, //1
		2, //2
		3, //3
		4, //4
		5, //5
		6, //6
		8, //7
		8, //8
		10, //9
		10, //10
		16, //11
		16, //12
		16, //13
		16, //14
		16, //15
		16, //16
		32, //17
		32, //18
		32, //19
		32, //20
		32, //21
		32, //22
		32, //23
		32, //24
		32, //25
		32, //26
		32, //27
		32, //28
		32, //29
		32, //30
		32, //31
		32  //32
		};

	/*
		SELECTOR_TO_USE[]
		-----------------
		Given the width in bits, which selector should be used?  This is used to ensure
		a switch() statement has all the entries 0..n with no gaps.
	*/
	static uint32_t selector_to_use[] =			///< selector to use (column 1 ) for bits in integer (right column)
		{
		0, //0
		0, //1
		1, //2
		2, //3
		3, //4
		4, //5
		5, //6
		6, //7
		6, //8
		7, //9
		7, //10
		8, //11
		8, //12
		8, //13
		8, //14
		8, //15
		8, //16
		9, //17
		9, //18
		9, //19
		9, //20
		9, //21
		9, //22
		9, //23
		9, //24
		9, //25
		9, //26
		9, //27
		9, //28
		9, //29
		9, //30
		9, //31
		9  //32
		};


	/*
		BITS_NEEDED()
		-------------
	*/
	static inline uint32_t bits_needed(uint32_t integer)
		{
		uint32_t current_width = JASS::maths::ceiling_log2(integer);
		return JASS::maths::maximum(current_width, static_cast<decltype(current_width)>(1));
		}

	/*
		COMPRESS_INTEGER_BITPACK_256::ENCODE()
		--------------------------------------
	*/
	size_t compress_integer_bitpack_256::encode(void *encoded, size_t encoded_buffer_length, const integer *array, size_t source_integers)
		{
		#define WORD_WIDTH_IN_BITS 256

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
				widest = JASS::maths::maximum(widest, bits_needed(*current));
				if (widest * (current - array) > WORD_WIDTH_IN_BITS)
					break;
				}

			/*
				Compute the width to use and how many integers to encode
			*/
			widest = bits_to_use[widest];
			end = array + (32 / widest) * (WORD_WIDTH_IN_BITS / 32);

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
				if (current_word >= (WORD_WIDTH_IN_BITS / 32))
					{
					current_word = 0;
					placement++;
					}
				}

			/*
				move on to the next word
			*/
			*destination = selector_to_use[widest];
			destination += (WORD_WIDTH_IN_BITS / 8) + 1;
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

	alignas(32) static uint32_t static_mask_16[] = {0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff};			///< AND mask for 16-bit integers
	alignas(32) static uint32_t static_mask_10[] = {0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff};						///< AND mask for 10-bit integers
	alignas(32) static uint32_t static_mask_8[] =  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};								///< AND mask for 8-bit integers
	alignas(32) static uint32_t static_mask_6[]  = {0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f};								///< AND mask for 6-bit integers
	alignas(32) static uint32_t static_mask_5[]  = {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};								///< AND mask for 5-bit integers
	alignas(32) static uint32_t static_mask_4[]  = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f};								///< AND mask for 4-bit integers
	alignas(32) static uint32_t static_mask_3[]  = {0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07};								///< AND mask for 3-bit integers
	alignas(32) static uint32_t static_mask_2[]  = {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};								///< AND mask for 2-bit integers
	alignas(32) static uint32_t static_mask_1[]  = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};								///< AND mask for 1-bit integers

	/*
		COMPRESS_INTEGER_BITPACK_256::DECODE()
		--------------------------------------
	*/
	static const __m256i mask_16 = _mm256_loadu_si256((__m256i *)static_mask_16);
	static const __m256i mask_10 = _mm256_loadu_si256((__m256i *)static_mask_10);
	static const __m256i mask_8 = _mm256_loadu_si256((__m256i *)static_mask_8);
	static const __m256i mask_6 = _mm256_loadu_si256((__m256i *)static_mask_6);
	static const __m256i mask_5 = _mm256_loadu_si256((__m256i *)static_mask_5);
	static const __m256i mask_4 = _mm256_loadu_si256((__m256i *)static_mask_4);
	static const __m256i mask_3 = _mm256_loadu_si256((__m256i *)static_mask_3);
	static const __m256i mask_2 = _mm256_loadu_si256((__m256i *)static_mask_2);
	static const __m256i mask_1 = _mm256_loadu_si256((__m256i *)static_mask_1);

	void compress_integer_bitpack_256::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		__m256i *into = (__m256i *)decoded;
		__m256i data;
		const uint8_t *source = (uint8_t *)source_as_void;
		const uint8_t *end_of_source = source + source_length;

		while (source < end_of_source)
			{
			uint32_t width = *source;
			source++;
			data = _mm256_loadu_si256((__m256i *)source);
			switch (width)
				{
				case 0:
					_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 5, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 6, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 7, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 8, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 9, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 10, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 11, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 12, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 13, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 14, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 15, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 16, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 17, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 18, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 19, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 20, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 21, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 22, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 23, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 24, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 25, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 26, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 27, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 28, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 29, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 30, _mm256_and_si256(data, mask_1));
					data = _mm256_srli_epi32(data, 1);
					_mm256_store_si256(into + 31, _mm256_and_si256(data, mask_1));
					into += 32;
					break;
				case 1:
					_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 5, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 6, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 7, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 8, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 9, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 10, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 11, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 12, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 13, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 14, _mm256_and_si256(data, mask_2));
					data = _mm256_srli_epi32(data, 2);
					_mm256_store_si256(into + 15, _mm256_and_si256(data, mask_2));
					into += 16;
					break;
				case 2:
					_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_3));
					data = _mm256_srli_epi32(data, 3);
					_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_3));
					data = _mm256_srli_epi32(data, 3);
					_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_3));
					data = _mm256_srli_epi32(data, 3);
					_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_3));
					data = _mm256_srli_epi32(data, 3);
					_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_3));
					data = _mm256_srli_epi32(data, 3);
					_mm256_store_si256(into + 5, _mm256_and_si256(data, mask_3));
					data = _mm256_srli_epi32(data, 3);
					_mm256_store_si256(into + 6, _mm256_and_si256(data, mask_3));
					data = _mm256_srli_epi32(data, 3);
					_mm256_store_si256(into + 7, _mm256_and_si256(data, mask_3));
					data = _mm256_srli_epi32(data, 3);
					_mm256_store_si256(into + 8, _mm256_and_si256(data, mask_3));
					data = _mm256_srli_epi32(data, 3);
					_mm256_store_si256(into + 9, _mm256_and_si256(data, mask_3));
					into += 10;
					break;
				case 3:
					_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_4));
					data = _mm256_srli_epi32(data, 4);
					_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_4));
					data = _mm256_srli_epi32(data, 4);
					_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_4));
					data = _mm256_srli_epi32(data, 4);
					_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_4));
					data = _mm256_srli_epi32(data, 4);
					_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_4));
					data = _mm256_srli_epi32(data, 4);
					_mm256_store_si256(into + 5, _mm256_and_si256(data, mask_4));
					data = _mm256_srli_epi32(data, 4);
					_mm256_store_si256(into + 6, _mm256_and_si256(data, mask_4));
					data = _mm256_srli_epi32(data, 4);
					_mm256_store_si256(into + 7, _mm256_and_si256(data, mask_4));
					into += 8;
					break;
				case 4:
					_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_5));
					data = _mm256_srli_epi32(data, 5);
					_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_5));
					data = _mm256_srli_epi32(data, 5);
					_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_5));
					data = _mm256_srli_epi32(data, 5);
					_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_5));
					data = _mm256_srli_epi32(data, 5);
					_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_5));
					data = _mm256_srli_epi32(data, 5);
					_mm256_store_si256(into + 5, _mm256_and_si256(data, mask_5));
					into += 6;
					break;
				case 5:
					_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_6));
					data = _mm256_srli_epi32(data, 6);
					_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_6));
					data = _mm256_srli_epi32(data, 6);
					_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_6));
					data = _mm256_srli_epi32(data, 6);
					_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_6));
					data = _mm256_srli_epi32(data, 6);
					_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_6));
					into += 5;
					break;
				case 6:
					_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_8));
					data = _mm256_srli_epi32(data, 8);
					_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_8));
					data = _mm256_srli_epi32(data, 8);
					_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_8));
					data = _mm256_srli_epi32(data, 8);
					_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_8));
					into += 4;
					break;
				case 7:
					_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_10));
					data = _mm256_srli_epi32(data, 10);
					_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_10));
					data = _mm256_srli_epi32(data, 10);
					_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_10));
					into += 3;
					break;
				case 8:
					_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_16));
					data = _mm256_srli_epi32(data, 16);
					_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_16));
					into += 2;
					break;
				case 9:
					_mm256_store_si256(into, _mm256_loadu_si256((__m256i *)source));
					into++;
					break;
				}
			source += sizeof(__m256i);
			}
		}

	/*
		COMPRESS_INTEGER_BITPACK_256::UNITTEST_ONE()
		--------------------------------------------
	*/
	void compress_integer_bitpack_256::unittest_one(const std::vector<uint32_t> &sequence)
		{
		compress_integer_bitpack_256 compressor;
		std::vector<uint32_t>compressed(sequence.size() * 2);
		std::vector<uint32_t>decompressed(sequence.size() + 256);

		auto size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &sequence[0], sequence.size());
		compressor.decode(&decompressed[0], sequence.size(), &compressed[0], size_once_compressed);
		decompressed.resize(sequence.size());
		JASS_assert(decompressed == sequence);
		}

	void compress_integer_bitpack_256::unittest(void)
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
			every_case.push_back(0x3FF);
		unittest_one(every_case);

		/*
			32-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 1 * 8; instance++)
			every_case.push_back(0x3FF);
		unittest_one(every_case);

		puts("parser_query::PASSED");
		}
	}





/*
	COMPRESS_INTEGER_BITPACK_128.CPP
	--------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdint.h>
#include <immintrin.h>

#include <vector>

#include "maths.h"
#include "asserts.h"
#include "compress_integer_bitpack_128.h"

namespace JASS
	{
	alignas(16) static uint32_t static_mask_16[] = {0xffff, 0xffff, 0xffff, 0xffff};												///< AND mask for 16-bit integers
	alignas(16) static uint32_t static_mask_10[] = {0x3ff, 0x3ff, 0x3ff, 0x3ff};						///< AND mask for 10-bit integers
	alignas(16) static uint32_t static_mask_8[] =  {0xff, 0xff, 0xff, 0xff};								///< AND mask for 8-bit integers
	alignas(16) static uint32_t static_mask_6[]  = {0x3f, 0x3f, 0x3f, 0x3f};								///< AND mask for 6-bit integers
	alignas(16) static uint32_t static_mask_5[]  = {0x1f, 0x1f, 0x1f, 0x1f};								///< AND mask for 5-bit integers
	alignas(16) static uint32_t static_mask_4[]  = {0x0f, 0x0f, 0x0f, 0x0f};								///< AND mask for 4-bit integers
	alignas(16) static uint32_t static_mask_3[]  = {0x07, 0x07, 0x07, 0x07};								///< AND mask for 3-bit integers
	alignas(16) static uint32_t static_mask_2[]  = {0x03, 0x03, 0x03, 0x03};								///< AND mask for 2-bit integers
	alignas(16) static uint32_t static_mask_1[]  = {0x01, 0x01, 0x01, 0x01};								///< AND mask for 1-bit integers

	/*
		COMPRESS_INTEGER_BITPACK_128::DECODE()
		--------------------------------------
	*/
	void compress_integer_bitpack_128::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		__m128i *into = (__m128i *)decoded;
		__m128i data;
		const uint8_t *source = (uint8_t *)source_as_void;
		const uint8_t *end_of_source = source + source_length;

		static const __m128i mask_16 = _mm_loadu_si128((__m128i *)static_mask_16);
		static const __m128i mask_10 = _mm_loadu_si128((__m128i *)static_mask_10);
		static const __m128i mask_8 = _mm_loadu_si128((__m128i *)static_mask_8);
		static const __m128i mask_6 = _mm_loadu_si128((__m128i *)static_mask_6);
		static const __m128i mask_5 = _mm_loadu_si128((__m128i *)static_mask_5);
		static const __m128i mask_4 = _mm_loadu_si128((__m128i *)static_mask_4);
		static const __m128i mask_3 = _mm_loadu_si128((__m128i *)static_mask_3);
		static const __m128i mask_2 = _mm_loadu_si128((__m128i *)static_mask_2);
		static const __m128i mask_1 = _mm_loadu_si128((__m128i *)static_mask_1);

		while (source < end_of_source)
			{
			uint32_t width = *source;
			source++;
			data = _mm_loadu_si128((__m128i *)source);
			switch (width)
				{
				case 0:
					_mm_storeu_si128(into + 0, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 1, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 2, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 3, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 4, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 5, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 6, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 7, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 8, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 9, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 10, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 11, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 12, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 13, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 14, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 15, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 16, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 17, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 18, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 19, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 20, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 21, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 22, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 23, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 24, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 25, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 26, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 27, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 28, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 29, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 30, _mm_and_si128(data, mask_1));
					data = _mm_srli_epi32(data, 1);
					_mm_storeu_si128(into + 31, _mm_and_si128(data, mask_1));
					into += 32;
					break;
				case 1:
					_mm_storeu_si128(into + 0, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 1, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 2, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 3, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 4, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 5, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 6, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 7, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 8, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 9, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 10, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 11, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 12, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 13, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 14, _mm_and_si128(data, mask_2));
					data = _mm_srli_epi32(data, 2);
					_mm_storeu_si128(into + 15, _mm_and_si128(data, mask_2));
					into += 16;
					break;
				case 2:
					_mm_storeu_si128(into + 0, _mm_and_si128(data, mask_3));
					data = _mm_srli_epi32(data, 3);
					_mm_storeu_si128(into + 1, _mm_and_si128(data, mask_3));
					data = _mm_srli_epi32(data, 3);
					_mm_storeu_si128(into + 2, _mm_and_si128(data, mask_3));
					data = _mm_srli_epi32(data, 3);
					_mm_storeu_si128(into + 3, _mm_and_si128(data, mask_3));
					data = _mm_srli_epi32(data, 3);
					_mm_storeu_si128(into + 4, _mm_and_si128(data, mask_3));
					data = _mm_srli_epi32(data, 3);
					_mm_storeu_si128(into + 5, _mm_and_si128(data, mask_3));
					data = _mm_srli_epi32(data, 3);
					_mm_storeu_si128(into + 6, _mm_and_si128(data, mask_3));
					data = _mm_srli_epi32(data, 3);
					_mm_storeu_si128(into + 7, _mm_and_si128(data, mask_3));
					data = _mm_srli_epi32(data, 3);
					_mm_storeu_si128(into + 8, _mm_and_si128(data, mask_3));
					data = _mm_srli_epi32(data, 3);
					_mm_storeu_si128(into + 9, _mm_and_si128(data, mask_3));
					into += 10;
					break;
				case 3:
					_mm_storeu_si128(into + 0, _mm_and_si128(data, mask_4));
					data = _mm_srli_epi32(data, 4);
					_mm_storeu_si128(into + 1, _mm_and_si128(data, mask_4));
					data = _mm_srli_epi32(data, 4);
					_mm_storeu_si128(into + 2, _mm_and_si128(data, mask_4));
					data = _mm_srli_epi32(data, 4);
					_mm_storeu_si128(into + 3, _mm_and_si128(data, mask_4));
					data = _mm_srli_epi32(data, 4);
					_mm_storeu_si128(into + 4, _mm_and_si128(data, mask_4));
					data = _mm_srli_epi32(data, 4);
					_mm_storeu_si128(into + 5, _mm_and_si128(data, mask_4));
					data = _mm_srli_epi32(data, 4);
					_mm_storeu_si128(into + 6, _mm_and_si128(data, mask_4));
					data = _mm_srli_epi32(data, 4);
					_mm_storeu_si128(into + 7, _mm_and_si128(data, mask_4));
					into += 8;
					break;
				case 4:
					_mm_storeu_si128(into + 0, _mm_and_si128(data, mask_5));
					data = _mm_srli_epi32(data, 5);
					_mm_storeu_si128(into + 1, _mm_and_si128(data, mask_5));
					data = _mm_srli_epi32(data, 5);
					_mm_storeu_si128(into + 2, _mm_and_si128(data, mask_5));
					data = _mm_srli_epi32(data, 5);
					_mm_storeu_si128(into + 3, _mm_and_si128(data, mask_5));
					data = _mm_srli_epi32(data, 5);
					_mm_storeu_si128(into + 4, _mm_and_si128(data, mask_5));
					data = _mm_srli_epi32(data, 5);
					_mm_storeu_si128(into + 5, _mm_and_si128(data, mask_5));
					into += 6;
					break;
				case 5:
					_mm_storeu_si128(into + 0, _mm_and_si128(data, mask_6));
					data = _mm_srli_epi32(data, 6);
					_mm_storeu_si128(into + 1, _mm_and_si128(data, mask_6));
					data = _mm_srli_epi32(data, 6);
					_mm_storeu_si128(into + 2, _mm_and_si128(data, mask_6));
					data = _mm_srli_epi32(data, 6);
					_mm_storeu_si128(into + 3, _mm_and_si128(data, mask_6));
					data = _mm_srli_epi32(data, 6);
					_mm_storeu_si128(into + 4, _mm_and_si128(data, mask_6));
					into += 5;
					break;
				case 6:
					_mm_storeu_si128(into + 0, _mm_and_si128(data, mask_8));
					data = _mm_srli_epi32(data, 8);
					_mm_storeu_si128(into + 1, _mm_and_si128(data, mask_8));
					data = _mm_srli_epi32(data, 8);
					_mm_storeu_si128(into + 2, _mm_and_si128(data, mask_8));
					data = _mm_srli_epi32(data, 8);
					_mm_storeu_si128(into + 3, _mm_and_si128(data, mask_8));
					into += 4;
					break;
				case 7:
					_mm_storeu_si128(into + 0, _mm_and_si128(data, mask_10));
					data = _mm_srli_epi32(data, 10);
					_mm_storeu_si128(into + 1, _mm_and_si128(data, mask_10));
					data = _mm_srli_epi32(data, 10);
					_mm_storeu_si128(into + 2, _mm_and_si128(data, mask_10));
					into += 3;
					break;
				case 8:
					_mm_storeu_si128(into + 0, _mm_and_si128(data, mask_16));
					data = _mm_srli_epi32(data, 16);
					_mm_storeu_si128(into + 1, _mm_and_si128(data, mask_16));
					into += 2;
					break;
				case 9:
					_mm_storeu_si128(into, data);
					into++;
					break;
				}
			source += sizeof(__m128i);
			}
		}
	}





/*
	SIMD.H
	-------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Helpful SIMD methods
	@author Andrew Trotman
	@copyright 2020 Andrew Trotman
*/

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#include "asserts.h"
#include "forceinline.h"


namespace JASS
	{
	/*
		CLASS SIMD
		----------
	*/
	/*!
		@brief Helpful SIMD methods
	*/
	class simd
		{
		public:
			/*
				GATHER()
				--------
			*/
			/*!
				@brief Gather 8 x 8 bit values into an 8 x 32-bit integer register.
				@param array [in] The base address of the array to read from.
				@param vindex [in] The indexes into the array to read from.
				@param a [in] The value to split and scatter.
				@return The 8 integers
			*/
			forceinline static __m256i gather(const uint8_t *array, __m256i vindex)
				{
				return _mm256_set_epi32
					(
					array[_mm256_extract_epi32(vindex, 7)],
					array[_mm256_extract_epi32(vindex, 6)],
					array[_mm256_extract_epi32(vindex, 5)],
					array[_mm256_extract_epi32(vindex, 4)],
					array[_mm256_extract_epi32(vindex, 3)],
					array[_mm256_extract_epi32(vindex, 2)],
					array[_mm256_extract_epi32(vindex, 1)],
					array[_mm256_extract_epi32(vindex, 0)]
					);
				}

			/*
				SCATTER()
				---------
			*/
			/*!
				@brief Scatter 8-bit integers
				@param array [in] The base address of the array
				@param vindex [in] The indexes into the array to write into
				@param a [in] The value to split and scatter (8 x 32-bit integers written as 8 x 16-bit integers)
			*/
			forceinline static void scatter(uint8_t *array, __m256i vindex, __m256i a)
				{
				array[_mm256_extract_epi32(vindex, 0)] = _mm256_extract_epi16(a, 0);
				array[_mm256_extract_epi32(vindex, 1)] = _mm256_extract_epi16(a, 2);
				array[_mm256_extract_epi32(vindex, 2)] = _mm256_extract_epi16(a, 4);
				array[_mm256_extract_epi32(vindex, 3)] = _mm256_extract_epi16(a, 6);
				array[_mm256_extract_epi32(vindex, 4)] = _mm256_extract_epi16(a, 8);
				array[_mm256_extract_epi32(vindex, 5)] = _mm256_extract_epi16(a, 10);
				array[_mm256_extract_epi32(vindex, 6)] = _mm256_extract_epi16(a, 12);
				array[_mm256_extract_epi32(vindex, 7)] = _mm256_extract_epi16(a, 14);
				}

			/*
				GATHER()
				--------
			*/
			/*!
				@brief Gather 8 x 16 bit values into an 8 x 32-bit integer register.
				@param array [in] The base address of the array to read from.
				@param vindex [in] The indexes into the array to read from.
				@param a [in] The value to split and scatter.
				@return The 8 integers
			*/
			forceinline static __m256i gather(const uint16_t *array, __m256i vindex)
				{
				return _mm256_set_epi32
					(
					array[_mm256_extract_epi32(vindex, 7)],
					array[_mm256_extract_epi32(vindex, 6)],
					array[_mm256_extract_epi32(vindex, 5)],
					array[_mm256_extract_epi32(vindex, 4)],
					array[_mm256_extract_epi32(vindex, 3)],
					array[_mm256_extract_epi32(vindex, 2)],
					array[_mm256_extract_epi32(vindex, 1)],
					array[_mm256_extract_epi32(vindex, 0)]
					);
				}

			/*
				SCATTER()
				---------
			*/
			/*!
				@brief Scatter 16-bit integers
				@param array [in] The base address of the array
				@param vindex [in] The indexes into the array to write into
				@param a [in] The value to split and scatter (8 x 32-bit integers written as 8 x 16-bit integers)
			*/
			forceinline static void scatter(uint16_t *array, __m256i vindex, __m256i a)
				{
				array[_mm256_extract_epi32(vindex, 0)] = _mm256_extract_epi16(a, 0);
				array[_mm256_extract_epi32(vindex, 1)] = _mm256_extract_epi16(a, 2);
				array[_mm256_extract_epi32(vindex, 2)] = _mm256_extract_epi16(a, 4);
				array[_mm256_extract_epi32(vindex, 3)] = _mm256_extract_epi16(a, 6);
				array[_mm256_extract_epi32(vindex, 4)] = _mm256_extract_epi16(a, 8);
				array[_mm256_extract_epi32(vindex, 5)] = _mm256_extract_epi16(a, 10);
				array[_mm256_extract_epi32(vindex, 6)] = _mm256_extract_epi16(a, 12);
				array[_mm256_extract_epi32(vindex, 7)] = _mm256_extract_epi16(a, 14);
				}

			/*
				GATHER()
				--------
			*/
			/*!
				@brief Gather 8 x 32 bit values into an 8 x 32-bit integer register.
				@param array [in] The base address of the array to read from.
				@param vindex [in] The indexes into the array to read from.
				@param a [in] The value to split and scatter.
				@return The 8 integers
			*/
			forceinline static __m256i gather(const uint32_t *array, __m256i vindex)
				{
				return _mm256_i32gather_epi32((int const *)array, vindex, 4);
				}

			/*
				SCATTER()
				---------
			*/
			/*!
				@brief Scatter 32-bit integers
				@param array [in] The base address of the array
				@param vindex [in] The indexes into the array to write into
				@param a [in] The value to split and scatter (8 x 32-bit integers written as 8 x 32-bit integers)
			*/
			forceinline static void scatter(uint32_t *array, __m256i vindex, __m256i a)
				{
				array[_mm256_extract_epi32(vindex, 0)] = _mm256_extract_epi32(a, 0);
				array[_mm256_extract_epi32(vindex, 1)] = _mm256_extract_epi32(a, 1);
				array[_mm256_extract_epi32(vindex, 2)] = _mm256_extract_epi32(a, 2);
				array[_mm256_extract_epi32(vindex, 3)] = _mm256_extract_epi32(a, 3);
				array[_mm256_extract_epi32(vindex, 4)] = _mm256_extract_epi32(a, 4);
				array[_mm256_extract_epi32(vindex, 5)] = _mm256_extract_epi32(a, 5);
				array[_mm256_extract_epi32(vindex, 6)] = _mm256_extract_epi32(a, 6);
				array[_mm256_extract_epi32(vindex, 7)] = _mm256_extract_epi32(a, 7);
				}


			/*
				CUMULATIVE_SUM()
				----------------
			*/
			/*!
				@brief Calculate the cumulative sum of the 32-bit integers in an AVX2 register.
				@param elements [in] The 32-bit integers.
				@return An AVX2 register holding the cumulative sums.
			*/
			forceinline static __m256i cumulative_sum(__m256i elements)
				{
				/*
					shift left by 1 integer and add
					A B C D E F G H
					B C D 0 F G H 0
				*/
				__m256i bottom = _mm256_bslli_epi128(elements, 4);
				elements = _mm256_add_epi32(elements, bottom);

				/*
					shift left by 2 integers and add
					AB BC CD D0 EF FG GH H0
					CD D0 00 00 GH H0 00 00
				*/
				bottom = _mm256_bslli_epi128(elements, 8);
				elements = _mm256_add_epi32(elements, bottom);
				/*
					We have: ABCD BCD0 CD00 D000 EFGH FGH0 GH00 H0000
				*/

				/*
					shuffle to get: 0000 0000 0000 0000 EFGH EFGH EFGH EFGH
					permute to get: EFGH EFGH EFGH EFGH 0000 0000 0000 0000
				*/
				__m256i missing = _mm256_shuffle_epi32(elements, _MM_SHUFFLE(3, 3, 3, 3));
				missing = _mm256_permute2x128_si256(_mm256_setzero_si256(), missing, 2 << 4);

				/*
					ABCD BCD0 CD00 D000 EFGH FGH0 GH00 H000
					EFGH EFGH EFGH EFGH 0000 0000 0000 0000
				*/
				__m256i answer = _mm256_add_epi32(elements, missing);

				return answer;
				}

			/*
				CUMULATIVE_SUM()
				----------------
			*/
			/*
				@brief Calculate (inplace) the cumulative sum of the array of integers.
				@details As this uses AVX2 instrucrtions is can read and write more than length load of integers
				@param data [in/out] The integers to sum (and result).
				@param length [in] The number of integrers to sum.
			*/
			static void cumulative_sum(uint32_t *data, size_t length)
				{
				/*
					previous cumulative sum is zero
				*/
				__m256i previous_max = _mm256_setzero_si256();

				/*
					Loop over all the data (going too far if necessary)
				*/
				__m256i *end = (__m256i *)(data + length);
				for (__m256i *block = (__m256i *)data; block < end; block++)
					{
					/*
						load the next 8 integers
					*/
					__m256i current_set = _mm256_lddqu_si256(block);

					/*
						compute the cumulative sum of those
					*/
					current_set = cumulative_sum(current_set);

					/*
						add the previous maximum to each of them
					*/
					current_set = _mm256_add_epi32(current_set, previous_max);

					/*
						and write back out to the same location we read from
					*/
					_mm256_storeu_si256(block, current_set);

					/*
						Broadcast the largest number from the result for next time
					*/
					current_set = _mm256_shuffle_epi32(current_set, _MM_SHUFFLE(3, 3, 3, 3));
					previous_max = _mm256_permute2x128_si256(current_set, current_set, 3 | (3 << 4));
					}
				}

			/*
				SIMD::UNITTEST()
				----------------
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				uint32_t source_32[8];
				uint32_t destination_32[8];
				uint16_t source_16[8];
				uint16_t destination_16[8];
				uint32_t indexes[8];

				/*
					Initialise
				*/
				for (size_t pos = 0; pos < 8; pos++)
					{
					indexes[pos] = source_32[pos] = pos;
					source_16[pos] = pos;
					}

				::memset(destination_32, 0, sizeof(destination_32));
				::memset(destination_16, 0, sizeof(destination_16));

				__m256i vindex = _mm256_lddqu_si256 ((__m256i const *)indexes);

				/*
					Check 16-bit scatter/gather
				*/
				__m256i got = simd::gather(source_16, vindex);
				for (size_t pos = 0; pos < 8; pos++)
					JASS_assert(((uint32_t *)&got)[pos] == pos);

				simd::scatter(destination_16, vindex, got);
				for (size_t pos = 0; pos < 8; pos++)
					JASS_assert(destination_16[pos] == pos);

				/*
					Check 32-bit scatter/gather
				*/
				got = simd::gather(source_32, vindex);
				for (size_t pos = 0; pos < 8; pos++)
					JASS_assert(((uint32_t *)&got)[pos] == pos);

				simd::scatter(destination_32, vindex, got);
				for (size_t pos = 0; pos < 8; pos++)
					JASS_assert(destination_32[pos] == pos);

				puts("simd::PASSED");
				}
			};
	}

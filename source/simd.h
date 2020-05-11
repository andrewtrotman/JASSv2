/*
	SLICE.H
	-------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Slices (also known as string-descriptors) for C++.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/

#pragma once

#include <stdio.h>
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
		@bried Helpful SIMD methods
	*/
	class simd
		{
		public:
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

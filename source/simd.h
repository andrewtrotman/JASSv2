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
				SIMD::GATHER()
				--------------
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
#ifdef __AVX512F__
				return _mm256_maskz_mov_epi8((__mmask32)0x11111111, _mm256_i32gather_epi32((int const *)array, vindex, 1));
#else
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
#endif
				}

			/*
				SIMD::GATHER()
				--------------
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
#ifdef __AVX512F__
				return _mm256_maskz_mov_epi16((__mmask16)0x5555, _mm256_i32gather_epi32((int const *)array, vindex, 2));
#else
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
#endif
				}

			/*
				SIMD::GATHER()
				--------------
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
				SIMD::GATHER()
				--------------
			*/
			/*!
				@brief Gather 16 x 8 bit values into an 16 x 32-bit integer register.
				@param array [in] The base address of the array to read from.
				@param vindex [in] The indexes into the array to read from.
				@param a [in] The value to split and scatter.
				@return The 16 integers
			*/
			forceinline static __m512i gather(const uint8_t *array, __m512i vindex)
				{
				return _mm512_maskz_mov_epi8((__mmask64)0x1111111111111111, _mm512_i32gather_epi32(vindex, array, 1));
				}

			/*
				SIMD::GATHER()
				--------------
			*/
			/*!
				@brief Gather 16 x 16 bit values into an 16 x 32-bit integer register.
				@param array [in] The base address of the array to read from.
				@param vindex [in] The indexes into the array to read from.
				@param a [in] The value to split and scatter.
				@return The 16 integers
			*/
			forceinline static __m512i gather(const uint16_t *array, __m512i vindex)
				{
				return _mm512_maskz_mov_epi16((__mmask32)0x55555555, _mm512_i32gather_epi32(vindex, array, 2));
				}

			/*
				SIMD::GATHER()
				--------------
			*/
			/*!
				@brief Gather 16 x 32 bit values into an 16 x 32-bit integer register.
				@param array [in] The base address of the array to read from.
				@param vindex [in] The indexes into the array to read from.
				@param a [in] The value to split and scatter.
				@return The 16 integers
			*/
			forceinline static __m512i gather(const uint32_t *array, __m512i vindex)
				{
				return _mm512_i32gather_epi32(vindex, array, 4);
				}

			/*
				SIMD::SCATTER()
				---------------
			*/
			/*!
				@brief Scatter 8-bit integers
				@param array [in] The base address of the array
				@param vindex [in] The indexes into the array to write into
				@param a [in] The value to split and scatter (8 x 32-bit integers written as 8 x 16-bit integers)
			*/
			forceinline static void scatter(uint8_t *array, __m256i vindex, __m256i a)
				{
#ifdef __AVX512F__
				/*
					Assunme array == NULL, now vindex is an address.  Turn that into a 32-bit "chunk" by dividing by 4
					then check to see if there are any address conflicts. If there are not then we can gather, blend, scatter.
					If there are conflicts then we to do individual writes because gather,blend,write will give the wrong
					result when two bytes are adjacent (because the results of the gather, blend will be wrong).
				*/
				__m256i word_locations = _mm256_srli_epi32(vindex, 2);
				__m256i conflict = _mm256_conflict_epi32(word_locations);
				if (_mm256_testz_si256(_mm256_set1_epi32(0xFFFF'FFFF), conflict))
					{
					__m256i was = _mm256_i32gather_epi32((int *)array, vindex, 1);
					__m256i send = _mm256_mask_blend_epi8((__mmask32)0x1111'1111, was, a);
					_mm256_i32scatter_epi32(array, vindex, send, 1);
					}
				else
#endif
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
				}

			/*
				SIMD::SCATTER()
				---------------
			*/
			/*!
				@brief Scatter 16-bit integers
				@param array [in] The base address of the array
				@param vindex [in] The indexes into the array to write into
				@param a [in] The value to split and scatter (8 x 32-bit integers written as 8 x 16-bit integers)
			*/
			forceinline static void scatter(uint16_t *array, __m256i vindex, __m256i a)
				{
#ifdef __AVX512F__
				__m256i word_locations = _mm256_srli_epi32(vindex, 1);
				__m256i conflict = _mm256_conflict_epi32(word_locations);
				if (_mm256_testz_si256(_mm256_set1_epi32(0xFFFF'FFFF), conflict))
					{
					__m256i was = _mm256_i32gather_epi32((int *)array, vindex, 2);
					__m256i send = _mm256_mask_blend_epi16((__mmask16)0x5555, was, a);
					_mm256_i32scatter_epi32(array, vindex, send, 2);
					}
				else
#endif
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
				}

			/*
				SIMD::SCATTER()
				---------------
			*/
			/*!
				@brief Scatter 32-bit integers
				@param array [in] The base address of the array
				@param vindex [in] The indexes into the array to write into
				@param a [in] The value to split and scatter (8 x 32-bit integers written as 8 x 32-bit integers)
			*/
			forceinline static void scatter(uint32_t *array, __m256i vindex, __m256i a)
				{
#ifdef __AVX512F__
				_mm256_i32scatter_epi32 (array, vindex, a, 4);
#else
				array[_mm256_extract_epi32(vindex, 0)] = _mm256_extract_epi32(a, 0);
				array[_mm256_extract_epi32(vindex, 1)] = _mm256_extract_epi32(a, 1);
				array[_mm256_extract_epi32(vindex, 2)] = _mm256_extract_epi32(a, 2);
				array[_mm256_extract_epi32(vindex, 3)] = _mm256_extract_epi32(a, 3);
				array[_mm256_extract_epi32(vindex, 4)] = _mm256_extract_epi32(a, 4);
				array[_mm256_extract_epi32(vindex, 5)] = _mm256_extract_epi32(a, 5);
				array[_mm256_extract_epi32(vindex, 6)] = _mm256_extract_epi32(a, 6);
				array[_mm256_extract_epi32(vindex, 7)] = _mm256_extract_epi32(a, 7);
#endif
				}


			/*
				SIMD::SCATTER()
				---------------
			*/
			/*!
				@brief Scatter 32-bit integers
				@param array [in] The base address of the array
				@param vindex [in] The indexes into the array to write into
				@param a [in] The value to split and scatter (16 x 8-bit integers written as 16 x 32-bit integers)
			*/
			forceinline static void scatter(uint8_t *array, __m512i vindex, __m512i a)
			{
			__m512i word_locations = _mm512_srli_epi32(vindex, 2);
			__mmask16 unwritten = 0xFFFF;

			do
				{
				__m512i conflict = _mm512_maskz_conflict_epi32 (unwritten, word_locations);
				conflict = _mm512_and_epi32(_mm512_set1_epi32(unwritten), conflict);
				__mmask16 mask = _mm512_testn_epi32_mask(conflict, _mm512_set1_epi32(0xFFFF'FFFF));
				mask &= unwritten;

				__m512i was = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), mask, vindex, array, 1);
				__m512i send = _mm512_mask_blend_epi8((__mmask64)0x1111'1111'1111'1111, was, a);
				_mm512_mask_i32scatter_epi32 (array, mask, vindex, send, 1);

				unwritten ^= mask;
				}
			while (unwritten != 0);
			}

			/*
				SIMD::SCATTER()
				---------------
			*/
			/*!
				@brief Scatter 32-bit integers
				@param array [in] The base address of the array
				@param vindex [in] The indexes into the array to write into
				@param a [in] The value to split and scatter (16 x 16-bit integers written as 16 x 32-bit integers)
			*/
			forceinline static void scatter(uint16_t *array, __m512i vindex, __m512i a)
				{
				/*
					Convert from indexes of 16-bit integers to indexes of 32-bitwites
				*/
				__m512i word_locations = _mm512_srli_epi32(vindex, 1);

				/*
					See if we have any conflicts, and turn into a mask for first occurences
				*/
				__m512i conflict = _mm512_conflict_epi32(word_locations);
				__mmask16 mask = _mm512_testn_epi32_mask(conflict, _mm512_set1_epi32(0xFFFF'FFFF));

				/*
					read, merge, write the fitst occurences.
					The worst case is indexes of 1,2,3,4... which will result in every second index being read/written then the sanme a second time.
				*/
				__m512i was = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), mask, vindex, array, 2);
				__m512i send = _mm512_mask_blend_epi16((__mmask32)0x5555'5555, was, a);
				_mm512_mask_i32scatter_epi32 (array, mask, vindex, send, 2);

				/*
					read, merge, write the subsequnce occurences
					If the same address appears more than once then the result is indeterminant (you get what you deserve)
				*/
				mask = _knot_mask16(mask);
				was = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), mask, vindex, array, 2);
				send = _mm512_mask_blend_epi16((__mmask32)0x5555'5555, was, a);
				_mm512_mask_i32scatter_epi32 (array, mask, vindex, send, 2);
				}

			/*
				SIMD::SCATTER()
				---------------
			*/
			/*!
				@brief Scatter 32-bit integers
				@param array [in] The base address of the array
				@param vindex [in] The indexes into the array to write into
				@param a [in] The value to split and scatter (16 x 32-bit integers written as 16 x 32-bit integers)
			*/
			forceinline static void scatter(uint32_t *array, __m512i vindex, __m512i a)
				{
				_mm512_i32scatter_epi32(array, vindex, a, 4);
				}

			/*
				SIMD::CUMULATIVE_SUM()
				----------------------
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
				SIMD::CUMULATIVE_SUM()
				----------------------
			*/
			/*!
				@brief Calculate the cumulative sum of the 32-bit integers in an AVX512 register.
				@param elements [in] The 32-bit integers.
				@return An AVX512 register holding the cumulative sums.
			*/
			forceinline static __m512i cumulative_sum(__m512i elements)
				{
				/*
					shift left by 1 integer and add
					A B C D E F G H I J K L M N O P
					B C D 0 F G H 0 J K L M N O P 0
				*/
				__m512i bottom = _mm512_bslli_epi128(elements, 4);
				elements = _mm512_add_epi32(elements, bottom);

				/*
					shift left by 2 integers and add
					AB BC CD D0 EF FG GH H0 IJ JK KL L0 MN NO OP P0
					CD D0 00 00 GH H0 00 00 KL L0 00 00 OP P0 00 00
				*/
				bottom = _mm512_bslli_epi128(elements, 8);
				elements = _mm512_add_epi32(elements, bottom);

				/*
					We have: ABCD BCD0 CD00 D000 EFGH FGH0 GH00 H000 IJKL JKL0 KL00 L000 MNOP NOP0 OP00 P000
					permute: EFGH EFGH EFGH EFGH 0000 0000 0000 0000 MNOP MNOP MNOP MNOP 0000 0000 0000 0000
				*/
				static const __m512i missing_in_lane = _mm512_set_epi32(11, 11, 11, 11, 0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0);
				__m512i missing = _mm512_maskz_permutexvar_epi32((__mmask16)0xF0F0, missing_in_lane, elements);
				__m512i answer = _mm512_add_epi32(elements, missing);

				/*
					We have: ABCDEFGH BCD0EFGH CD00EFGH D000EFGH EFGH0000 FGH00000 GH000000 H0000000 IJKLMNOP JKL0MNOP KL00MNOP L000MNOP MNOP0000 NOP00000 OP000000 P0000000
					permute: IJKLMNOP IJKLMNOP IJKLMNOP IJKLMNOP IJKLMNOP IJKLMNOP IJKLMNOP IJKLMNOP 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
				*/
				static const __m512i missing_cross_lane = _mm512_set_epi32(7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0);
				__m512i result = _mm512_maskz_permutexvar_epi32((__mmask16)0xFF00, missing_cross_lane, answer);
				answer = _mm512_add_epi32(answer, result);

				return answer;
				}

			/*
				SIMD::CUMULATIVE_SUM_512()
				--------------------------
			*/
			/*
				@brief Calculate (inplace) the cumulative sum of the array of integers.
				@details As this uses AVX512 instrucrtions is can read and write more than length load of integers
				@param data [in/out] The integers to sum (and result).
				@param length [in] The number of integrers to sum.
			*/
			static void cumulative_sum_512(uint32_t *data, size_t length)
				{
				/*
					previous cumulative sum is zero
				*/
				__m512i previous_max = _mm512_setzero_si512();

				/*
					Loop over all the data (going too far if necessary)
				*/
				__m512i *end = (__m512i *)(data + length);
				for (__m512i *block = (__m512i *)data; block < end; block++)
					{
					/*
						load the next 16 integers
					*/
					__m512i current_set = _mm512_loadu_si512(block);

					/*
						compute the cumulative sum of those
					*/
					current_set = cumulative_sum(current_set);

					/*
						add the previous maximum to each of them
					*/
					current_set = _mm512_add_epi32(current_set, previous_max);

					/*
						and write back out to the same location we read from
					*/
					_mm512_storeu_si512(block, current_set);

					/*
						Broadcast the largest number from the result for next time
					*/
					previous_max = _mm512_maskz_permutexvar_epi32((__mmask16)0xFFFF, _mm512_set1_epi32(0x0F), current_set);
					}
				}

			/*
				SIMD::CUMULATIVE_SUM_256()
				--------------------------
			*/
			/*
				@brief Calculate (inplace) the cumulative sum of the array of integers.
				@details As this uses AVX2 instrucrtions is can read and write more than length load of integers
				@param data [in/out] The integers to sum (and result).
				@param length [in] The number of integrers to sum.
			*/
			static void cumulative_sum_256(uint32_t *data, size_t length)
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
				SIMD::CUMULATIVE_SUM()
				----------------------
			*/
			/*
				@brief Calculate (inplace) the cumulative sum of the array of integers.
				@details As this uses AVX2 instrucrtions is can read and write more than length load of integers
				@param data [in/out] The integers to sum (and result).
				@param length [in] The number of integrers to sum.
			*/
			forceinline static void cumulative_sum(uint32_t *data, size_t length)
				{
		#ifdef __AVX512F__
				cumulative_sum_512(data, length);
		#else
				cumulative_sum_256(data, length);
		#endif
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

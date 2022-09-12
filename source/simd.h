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

#include <iostream>

#include "asserts.h"
#include "forceinline.h"

/*
	Here we can choose between individual writes or block writes on AVX512:

	USE_AXV512_READS_8:   gather then mask 8-bit reads
	USE_AXV512_READS_16:  gather then mask 16-bit reads
	USE_AXV512_WRITES_8:  gather/merge/scatter then mask 8-bit writes
	USE_AXV512_WRITES_16: gather/merge/scatter then mask 16-bit writes
	USE_AXV512_WRITES_32: gather/merge/scatter then mask 32-bit writes

	For "Intel(R) Core(TM) SICPU @ 3.80GHz" enable all of these.
*/

#ifdef _MSC_VER								// the optimizer get AVX512 wrong on Visual Studio 2022 (preview 2)
//	#define USE_AXV512_READS_8 0
//	#define USE_AXV512_READS_16 0
//	#define USE_AXV512_WRITES_8 0
//	#define USE_AXV512_WRITES_16 0
//	#define USE_AXV512_WRITES_32 0
#else
	#define USE_AXV512_READS_8 1
	#define USE_AXV512_READS_16 1
	#define USE_AXV512_WRITES_8 1
	#define USE_AXV512_WRITES_16 1
	#define USE_AXV512_WRITES_32 1
#endif


namespace JASS
	{
	std::ostream &operator<<(std::ostream &stream, const __m128i &data);
	std::ostream &operator<<(std::ostream &stream, const __m256i &data);
	std::ostream &operator<<(std::ostream &stream, const __m512i &data);

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
#if defined(USE_AXV512_READS_8) && defined(__AVX512F__)
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
#if defined(USE_AXV512_READS_16) && defined(__AVX512F__)
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

#ifdef __AVX512F__
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
				return _mm512_maskz_mov_epi8((__mmask64)0x1111'1111'1111'1111, _mm512_i32gather_epi32(vindex, array, 1));
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
				__m512i got = _mm512_i32gather_epi32(vindex, array, 2);
				__m512i answer = _mm512_maskz_mov_epi16((__mmask32)0x5555'5555, got);
				return answer;
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
#endif

			/*
				SIMD::SCATTER()
				---------------
			*/
			/*!
				@brief Scatter 8-bit integers
				@param array [in] The base address of the array
				@param indexes [in] The indexes into the array to write into
				@param values [in] The value to split and scatter (4 x 32-bit integers written as 4 x 8-bit integers)
			*/
			forceinline static void scatter(uint8_t *array, __m128i indexes, __m128i values)
				{
				/*
					Extracting 32-bit integers then downcasting to 8-bit integers is faster then extracting 8-bit integers
				*/
				array[_mm_extract_epi32(indexes, 0)] = _mm_extract_epi32(values, 0);
				array[_mm_extract_epi32(indexes, 1)] = _mm_extract_epi32(values, 1);
				array[_mm_extract_epi32(indexes, 2)] = _mm_extract_epi32(values, 2);
				array[_mm_extract_epi32(indexes, 3)] = _mm_extract_epi32(values, 3);
				}

			/*
				SIMD::SCATTER()
				---------------
			*/
			/*!
				@brief Scatter 16-bit integers
				@param array [in] The base address of the array
				@param indexes [in] The indexes into the array to write into
				@param values [in] The value to split and scatter (4 x 32-bit integers written as 4 x 16-bit integers)
			*/
			forceinline static void scatter(uint16_t *array, __m128i indexes, __m128i values)
				{
				/*
					Extracting 16-bit integers is faster then 32-bit integers
				*/
				array[_mm_extract_epi32(indexes, 0)] = _mm_extract_epi16(values, 0);
				array[_mm_extract_epi32(indexes, 1)] = _mm_extract_epi16(values, 2);
				array[_mm_extract_epi32(indexes, 2)] = _mm_extract_epi16(values, 4);
				array[_mm_extract_epi32(indexes, 3)] = _mm_extract_epi16(values, 6);
				}

			/*
				SIMD::SCATTER()
				---------------
			*/
			/*!
				@brief Scatter 32-bit integers
				@param array [in] The base address of the array
				@param indexes [in] The indexes into the array to write into
				@param values [in] The value to split and scatter (4 x 32-bit integers written as 4 x 32-bit integers)
			*/
			forceinline static void scatter(uint32_t *array, __m128i indexes, __m128i values)
				{
				array[_mm_extract_epi32(indexes, 0)] = _mm_extract_epi32(values, 0);
				array[_mm_extract_epi32(indexes, 1)] = _mm_extract_epi32(values, 1);
				array[_mm_extract_epi32(indexes, 2)] = _mm_extract_epi32(values, 2);
				array[_mm_extract_epi32(indexes, 3)] = _mm_extract_epi32(values, 3);
				}

			/*
				SIMD::SCATTER()
				---------------
			*/
			/*!
				@brief Scatter 8-bit integers
				@param array [in] The base address of the array
				@param vindex [in] The indexes into the array to write into
				@param a [in] The value to split and scatter (8 x 32-bit integers written as 8 x 8-bit integers)
			*/
			forceinline static void scatter(uint8_t *array, __m256i vindex, __m256i a)
				{
#if defined(USE_AXV512_WRITES_8) && defined(__AVX512F__)
				__m256i low_two_bits = _mm256_and_si256(vindex, _mm256_set1_epi32(3));

				__mmask8 zero = _mm256_cmp_epi32_mask(low_two_bits, _mm256_setzero_si256(), _MM_CMPINT_EQ);
				__m256i was = _mm256_mmask_i32gather_epi32(_mm256_setzero_si256(), zero, vindex, array, 1);
				__m256i data = _mm256_mask_blend_epi8(0x1111'1111, was, a);
				_mm256_mask_i32scatter_epi32(array, zero, vindex, data, 1);

				__mmask8 one = _mm256_cmp_epi32_mask(low_two_bits, _mm256_set1_epi32(1), _MM_CMPINT_EQ);
				was = _mm256_mmask_i32gather_epi32(_mm256_setzero_si256(), one, vindex, array, 1);
				data = _mm256_mask_blend_epi8(0x1111'1111, was, a);
				_mm256_mask_i32scatter_epi32(array, one, vindex, data, 1);

				__mmask8 two = _mm256_cmp_epi32_mask(low_two_bits, _mm256_set1_epi32(2), _MM_CMPINT_EQ);
				was = _mm256_mmask_i32gather_epi32(_mm256_setzero_si256(), two, vindex, array, 1);
				data = _mm256_mask_blend_epi8(0x1111'1111, was, a);
				_mm256_mask_i32scatter_epi32(array, two, vindex, data, 1);

				__mmask8 three = _mm256_cmp_epi32_mask(low_two_bits, _mm256_set1_epi32(3), _MM_CMPINT_EQ);
				was = _mm256_mmask_i32gather_epi32(_mm256_setzero_si256(), three, vindex, array, 1);
				data = _mm256_mask_blend_epi8(0x1111'1111, was, a);
				_mm256_mask_i32scatter_epi32(array, three, vindex, data, 1);
#else
				/*
					Extracting 32-bit integers then downcasting to 8-bit integers is faster then extracting 8-bit integers
				*/
				scatter(array, _mm256_extracti128_si256(vindex, 0), _mm256_extracti128_si256(a, 0));
				scatter(array, _mm256_extracti128_si256(vindex, 1), _mm256_extracti128_si256(a, 1));
#endif
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
#if defined(USE_AXV512_WRITES_16) && defined(__AVX512F__)
				int32_t *arr32 = (int32_t *)array;

				__mmask8 odd = _mm256_test_epi32_mask(vindex, _mm256_set1_epi32(1));

				__m256i was_odd = _mm256_mmask_i32gather_epi32(_mm256_setzero_si256(), ~odd, vindex, arr32, 2);
				__m256i data_even = _mm256_mask_blend_epi16(0x5555, was_odd, a);
				_mm256_mask_i32scatter_epi32(array, ~odd, vindex, data_even, 2);

				__m256i was_even = _mm256_mmask_i32gather_epi32(_mm256_setzero_si256(), odd, vindex, arr32, 2);
				__m256i data_odd = _mm256_mask_blend_epi16(0x5555, was_even, a);
				_mm256_mask_i32scatter_epi32(array, odd, vindex, data_odd, 2);
#else
				scatter(array, _mm256_extracti128_si256(vindex, 0), _mm256_extracti128_si256(a, 0));
				scatter(array, _mm256_extracti128_si256(vindex, 1), _mm256_extracti128_si256(a, 1));
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
				@param a [in] The value to split and scatter (8 x 32-bit integers written as 8 x 32-bit integers)
			*/
			forceinline static void scatter(uint32_t *array, __m256i vindex, __m256i a)
				{
#if defined(USE_AXV512_WRITES_32) && defined(__AVX512F__)
			   	_mm256_i32scatter_epi32(array, vindex, a, 4);
#else
				scatter(array, _mm256_extracti128_si256(vindex, 0), _mm256_extracti128_si256(a, 0));
				scatter(array, _mm256_extracti128_si256(vindex, 1), _mm256_extracti128_si256(a, 1));
#endif
				}
#ifdef __AVX512F__
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
#if defined(USE_AXV512_WRITES_8) && defined(__AVX512F__)
				__m512i low_two_bits = _mm512_and_epi32(vindex, _mm512_set1_epi32(3));

				__mmask16 zero = _mm512_cmp_epi32_mask(low_two_bits, _mm512_setzero_si512(), _MM_CMPINT_EQ);
				__m512i was = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), zero, vindex, array, 1);
				__m512i data = _mm512_mask_blend_epi8(0x1111'1111'1111'1111, was, a);
				_mm512_mask_i32scatter_epi32(array, zero, vindex, data, 1);

				__mmask16 one = _mm512_cmp_epi32_mask(low_two_bits, _mm512_set1_epi32(1), _MM_CMPINT_EQ);
				was = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), one, vindex, array, 1);
				data = _mm512_mask_blend_epi8(0x1111'1111'1111'1111, was, a);
				_mm512_mask_i32scatter_epi32(array, one, vindex, data, 1);

				__mmask16 two = _mm512_cmp_epi32_mask(low_two_bits, _mm512_set1_epi32(2), _MM_CMPINT_EQ);
				was = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), two, vindex, array, 1);
				data = _mm512_mask_blend_epi8(0x1111'1111'1111'1111, was, a);
				_mm512_mask_i32scatter_epi32(array, two, vindex, data, 1);

				__mmask16 three = _mm512_cmp_epi32_mask(low_two_bits, _mm512_set1_epi32(3), _MM_CMPINT_EQ);
				was = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), three, vindex, array, 1);
				data = _mm512_mask_blend_epi8(0x1111'1111'1111'1111, was, a);
				_mm512_mask_i32scatter_epi32(array, three, vindex, data, 1);
#else
				scatter(array, _mm512_extracti32x4_epi32(vindex, 0), _mm512_extracti32x4_epi32(a, 0));
				scatter(array, _mm512_extracti32x4_epi32(vindex, 1), _mm512_extracti32x4_epi32(a, 1));
				scatter(array, _mm512_extracti32x4_epi32(vindex, 2), _mm512_extracti32x4_epi32(a, 2));
				scatter(array, _mm512_extracti32x4_epi32(vindex, 3), _mm512_extracti32x4_epi32(a, 3));
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
				@param a [in] The value to split and scatter (16 x 16-bit integers written as 16 x 32-bit integers)
			*/
			forceinline static void scatter(uint16_t *array, __m512i vindex, __m512i a)
				{
#if defined(USE_AXV512_WRITES_16) && defined(__AVX512F__)
				__mmask16 odd = _mm512_test_epi32_mask(vindex, _mm512_set1_epi32(1));

				__m512i was_odd = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), ~odd, vindex, array, 2);
				__m512i data_even = _mm512_mask_blend_epi16(0x5555'5555, was_odd, a);
				_mm512_mask_i32scatter_epi32(array, ~odd, vindex, data_even, 2);

				__m512i was_even = _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), odd, vindex, array, 2);
				__m512i data_odd = _mm512_mask_blend_epi16(0x5555'5555, was_even, a);
				_mm512_mask_i32scatter_epi32(array, odd, vindex, data_odd, 2);
#else
				/*
					Extracting 16-bit integers is faster than extracting 32-bit integers
				*/
				scatter(array, _mm512_extracti32x4_epi32(vindex, 0), _mm512_extracti32x4_epi32(a, 0));
				scatter(array, _mm512_extracti32x4_epi32(vindex, 1), _mm512_extracti32x4_epi32(a, 1));
				scatter(array, _mm512_extracti32x4_epi32(vindex, 2), _mm512_extracti32x4_epi32(a, 2));
				scatter(array, _mm512_extracti32x4_epi32(vindex, 3), _mm512_extracti32x4_epi32(a, 3));
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
				@param a [in] The value to split and scatter (16 x 32-bit integers written as 16 x 32-bit integers)
			*/

			forceinline static void scatter(uint32_t *array, __m512i vindex, __m512i a)
				{
				/*
					On "Intel(R) Core(TM) i7-9800X CPU @ 3.80GHz" this is faster than individual writes for both adjacent
					and seperate cache line writes.
				*/
				_mm512_i32scatter_epi32(array, vindex, a, 4);
				}
#endif

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
#ifdef __AVX512F__
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
			/*!
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

#endif
			/*
				SIMD::CUMULATIVE_SUM_256()
				--------------------------
			*/
			/*!
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
#ifdef __AVX512F__
			/*
				SIMD::POPCOUNT()
				----------------
			*/
			/*!
				@brief Compute the number of set bits in each of the 32-bit integers (the population count)
				@details
					Uses the Wilkes-Wheel-Gill algorithm, which appears to be fastest on my PC.
					see https://github.com/WojciechMula/sse-popcount
					see W. Mula, N. Kurz, D. Lemire (2018) Faster Population Counts Using AVX2 Instructions, Computer Journal 61(1):111-120
				@return 32-bit integers holding the population count.
			*/
			forceinline static __m512i popcount(__m512i value)
				{
#ifdef NEVER
				/*
					This is the Wilkes-Wheel-Gill algorithm from
					W. Mula, N. Kurz, D. Lemire (2018) Faster Population Counts Using AVX2 Instructions, Computer Journal 61(1):111-120
					It isn't quite as fast as the algorithm below. See also, https://github.com/WojciechMula/sse-popcount
				*/
				const __m512i fives = _mm512_set1_epi8(0x55);
				const __m512i threes = _mm512_set1_epi8(0x33);
				const __m512i ohfs = _mm512_set1_epi8(0x0F);
				const __m512i ohones = _mm512_set1_epi8(0x01);

				value = _mm512_sub_epi32(value, (_mm512_and_epi32(_mm512_srli_epi32(value, 1), fives)));
				value = _mm512_add_epi32(_mm512_and_epi32(value, threes), (_mm512_and_epi32(_mm512_srli_epi32(value, 2), threes)));
				return  _mm512_srli_epi32(_mm512_mullo_epi32(_mm512_and_epi32(_mm512_add_epi32(value, _mm512_srli_epi32(value, 4)), ohfs), ohones), 24);
#else
				/*
					Mula’s algorithm from
					Mula, W. SSSE3: fast popcount. http://0x80.pl/articles/sse-popcount.html [last checked June 2020].
					source is on from: https://stackoverflow.com/questions/51104493/is-it-possible-to-popcount-m256i-and-store-result-in-8-32-bit-words-instead-of
					see W. Mula, N. Kurz, D. Lemire (2018) Faster Population Counts Using AVX2 Instructions, Computer Journal 61(1):111-120

					It uses a single lookup table of the numner of bits in a nybble, then adds the result for each nybble to give the result for each word.
				*/

				const __m512i lookup = _mm512_setr_epi64
					(
					0x0302020102010100llu, 0x0403030203020201llu,
					0x0302020102010100llu, 0x0403030203020201llu,
					0x0302020102010100llu, 0x0403030203020201llu,
					0x0302020102010100llu, 0x0403030203020201llu
					);

				__m512i low_mask = _mm512_set1_epi8(0x0f);
				__m512i lo = _mm512_and_si512(value, low_mask);
				__m512i hi = _mm512_and_si512(_mm512_srli_epi16(value, 4), low_mask);
				__m512i popcnt1 = _mm512_shuffle_epi8(lookup, lo);
				__m512i popcnt2 = _mm512_shuffle_epi8(lookup, hi);
				__m512i sum8 = _mm512_add_epi8(popcnt1, popcnt2);
				return _mm512_madd_epi16(_mm512_maddubs_epi16(sum8, _mm512_set1_epi8(1)), _mm512_set1_epi16(1));
#endif
				}
#endif

#ifdef __AVX512F__
			/*
				SIMD::BZERO64()
				---------------
			*/
			/*!
				@brief zero 64-byte aligned memory in 64-byte chunks
				@param address[in] The address to start zeroing from (must be 64-byte aligned)
				@param sixty_fours The numnber of 64-byte chunks of zero to write
			*/
			forceinline static void bzero64(void *address, uint32_t sixty_fours)
				{
				__m512i zero = _mm512_setzero_si512();
				__m512i *into = reinterpret_cast<__m512i *>(address);

				auto end = sixty_fours & ~3;
				uint32_t which = 0;
				while (which < end)
					{
					_mm512_store_si512(into++, zero);
					_mm512_store_si512(into++, zero);
					_mm512_store_si512(into++, zero);
					_mm512_store_si512(into++, zero);
					which += 4;
					}

				while (which < sixty_fours)
					{
					_mm512_store_si512(into++, zero);
					which++;
					}
				}
#else
			/*
				SIMD::BZERO64()
				---------------
			*/
			/*!
				@brief zero 64-byte aligned memory in 64-byte chunks
				@param address[in] The address to start zeroing from (must be 64-byte aligned)
				@param sixty_fours The numnber of 64-byte chunks of zero to write
			*/
			forceinline static void bzero64(void *address, uint32_t sixty_fours)
				{
				__m256i zero = _mm256_setzero_si256();
				__m256i *into = reinterpret_cast<__m256i *>(address);

				for (uint32_t which = 0; which < sixty_fours; which++)
						{
						_mm256_store_si256(into++, zero);
						_mm256_store_si256(into++, zero);
						}
				}
#endif

			/*
				SIMD::UNITTEST()
				----------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				uint8_t source_8[32];
				uint16_t source_16[16];
				uint32_t source_32[8];
				uint8_t destination_8[32];
				uint16_t destination_16[16];
				uint32_t destination_32[8];
				uint32_t indexes_32[8];
				uint32_t block[16];

				/*
					Initialise
				*/
				for (uint16_t pos = 0; pos < 8; pos++)
					{
					source_8[pos] = pos;
					source_16[pos] = pos;
					source_32[pos] = pos;
					indexes_32[pos] = pos;
					}

				::memset(destination_8, 0, sizeof(destination_8));
				::memset(destination_16, 0, sizeof(destination_16));
				::memset(destination_32, 0, sizeof(destination_32));

				__m256i vindex = _mm256_lddqu_si256((__m256i const *)indexes_32);

				/*
					Check 8-bit scatter/gather
				*/
				__m256i got = simd::gather(source_8, vindex);
				_mm256_storeu_si256((__m256i *)block, got);
				for (size_t pos = 0; pos < 8; pos++)
					JASS_assert(block[pos] == pos);

				simd::scatter(destination_8, vindex, got);
				for (size_t pos = 0; pos < 8; pos++)
					JASS_assert(destination_8[pos] == pos);

				/*
					Check 16-bit scatter/gather
				*/
				got = simd::gather(source_16, vindex);
				_mm256_storeu_si256((__m256i *)block, got);
				for (size_t pos = 0; pos < 8; pos++)
					JASS_assert(block[pos] == pos);

				simd::scatter(destination_16, vindex, got);
				for (size_t pos = 0; pos < 8; pos++)
					JASS_assert(destination_16[pos] == pos);

				/*
					Check 32-bit scatter/gather
				*/
				got = simd::gather(source_32, vindex);
				_mm256_storeu_si256((__m256i *)block, got);
//std::cout << got << "\n";
				for (size_t pos = 0; pos < 8; pos++)
					JASS_assert(block[pos] == pos);

				simd::scatter(destination_32, vindex, got);
				for (size_t pos = 0; pos < 8; pos++)
					JASS_assert(destination_32[pos] == pos);

				cumulative_sum_256(destination_32, 8);
				uint32_t sum_answer[8] = {0, 1, 3, 6, 10, 15, 21, 28};
				JASS_assert(::memcmp(destination_32, sum_answer, sizeof(sum_answer)) == 0);

#ifdef __AVX512F__
				uint32_t numbers[] = {0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767};
				__m512i bit_vector = _mm512_loadu_si512(numbers);
				auto set_bits = popcount(bit_vector);
				_mm512_storeu_si512((__m512i *)block, set_bits);
				for (size_t pos = 0; pos < 16; pos++)
					JASS_assert(block[pos] == pos);
#endif

				puts("simd::PASSED");
				}
		};

	/*
		OPERATOR<<()
		------------
	*/
	/*!
		@brief Dump the contents of an object.
		@param stream [in] The stream to write to.
		@param data [in] The data to write.
		@return The stream once the data has been written.
	*/
	inline std::ostream &operator<<(std::ostream &stream, const __m128i &data)
		{
		uint32_t got[4];
		_mm_storeu_si128((__m128i *)got, data);

		stream << "[";
		for (uint32_t index = 0;  index < 3; index++)
			stream << got[index] << ", ";
		stream << got[3] << "]";

		return stream;
		}


	/*
		OPERATOR<<()
		------------
	*/
	/*!
		@brief Dump the contents of an object.
		@param stream [in] The stream to write to.
		@param data [in] The data to write.
		@return The stream once the data has been written.
	*/
	inline std::ostream &operator<<(std::ostream &stream, const __m256i &data)
		{
		uint32_t got[8];
		_mm256_storeu_si256((__m256i *)got, data);

		stream << "[";
		for (uint32_t index = 0;  index < 7; index++)
			stream << got[index] << ", ";
		stream << got[7] << "]";

		return stream;
		}

	/*
		OPERATOR<<()
		------------
	*/
	/*!
		@brief Dump the contents of an object.
		@param stream [in] The stream to write to.
		@param data [in] The data to write.
		@return The stream once the data has been written.
	*/
	inline std::ostream &operator<<(std::ostream &stream, const __m512i &data)
		{
		uint32_t got[16];
		_mm512_storeu_si512(got, data);

		stream << "[";
		for (uint32_t index = 0;  index < 15; index++)
			stream << got[index] << ", ";
		stream << got[15] << "]";

		return stream;
		}
	}


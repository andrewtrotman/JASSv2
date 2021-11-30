/*
	SORT512_UINT64_T.H
	------------------
	This code sorts an array of 64-bit integers into ascending order (from lowest to largest).  It is a conversion
	of the avx-512-sort code (by Berenger Bramas) from double-precison floating point numbers to 64-bit integers. The
	conversion was done with global substitution of the _pd to _epi64 throughoug the source file.
	The original code is from: https://gitlab.mpcdf.mpg.de/bbramas/avx-512-sort/blob/master/sort512test.cpp
	see:
		B. Bramas (2017), A Novel Hybrid Quicksort Algorithm Vectorized using AVX-512 on Intel Skylake, International Journal of Advanced Computer Science and Applications(IJACSA), 8(10):337-344.

	There is only 1 method to call in this file:
		void Sort512_uint64_t::Sort(SortType array[], const IndexType size);
*/

//////////////////////////////////////////////////////////
/// Code to sort an array of integer or double
/// using avx 512 (targeting intel KNL/SKL).
/// By berenger.bramas@mpcdf.mpg.de 2017.
/// Licence is MIT.
/// Comes without any warranty.
///
///
/// Functions to call:
/// Sort512::Sort(); to sort an array
/// Sort512::SortOmp(); to sort in parallel
/// Sort512::Partition512(); to partition
/// Sort512::SmallSort16V(); to sort a small array
/// (should be less than 16 AVX512 vectors)
///
/// To compile such flags can be used to enable avx 512 and openmp:
/// - KNL
/// Gcc : -mavx512f -mavx512pf -mavx512er -mavx512cd -fopenmp
/// Intel : -xCOMMON-AVX512 -xMIC-AVX512 -qopenmp
/// - SKL
/// Gcc : -mavx512f -mavx512cd -mavx512vl -mavx512bw -mavx512dq -fopenmp
/// Intel : -xCOMMON-AVX512 -xCORE-AVX512 -qopenmp
///
/// Or use "-march=native -mtune=native" if you are already on the right platform ("native can be replaced by "knl" or "skylake")
//////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>
#include <immintrin.h>
#include <climits>
#include <cfloat>
#include <algorithm>

#ifdef _MSC_VER
	#define __restrict__ __restrict
#endif

#ifdef  __AVX512F__
namespace Sort512_uint64_t {



///////////////////////////////////////////////////////////
/// AVX Sort functions
///////////////////////////////////////////////////////////

/// Long

inline __m512i CoreSmallSort(__m512i input){
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(4, 5, 6, 7, 0, 1, 2, 3);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
    }

    return input;
}

inline void CoreSmallSort(uint64_t* __restrict__ ptr1){
    _mm512_storeu_si512(ptr1, CoreSmallSort(_mm512_loadu_si512(ptr1)));
}


inline void CoreExchangeSort2V(__m512i& input, __m512i& input2){
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        input = _mm512_min_epi64(input2, permNeigh);
        input2 = _mm512_max_epi64(input2, permNeigh);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
    }
}

inline void CoreSmallSort2(__m512i& input, __m512i& input2){
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(4, 5, 6, 7, 0, 1, 2, 3);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
    }
    CoreExchangeSort2V(input, input2);
}

inline void CoreSmallSort2(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2 ){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    CoreSmallSort2(input1, input2);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
}


inline void CoreSmallSort3(__m512i& input, __m512i& input2, __m512i& input3 ){
    CoreSmallSort2(input, input2);
    input3 = CoreSmallSort(input3);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        input3 = _mm512_max_epi64(input2, permNeigh);
        input2 = _mm512_min_epi64(input2, permNeigh);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
    }
}

inline void CoreSmallSort3(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3  ){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    CoreSmallSort3(input1, input2, input3);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
}


inline void CoreSmallSort4(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4 ){
    CoreSmallSort2(input, input2);
    CoreSmallSort2(input3, input4);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);

        input4 = _mm512_max_epi64(input, permNeigh4);
        input = _mm512_min_epi64(input, permNeigh4);

        input3 = _mm512_max_epi64(input2, permNeigh3);
        input2 = _mm512_min_epi64(input2, permNeigh3);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xF0, permNeighMax4);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xCC, permNeighMax4);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xAA, permNeighMax4);
    }
}


inline void CoreSmallSort4(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3, uint64_t* __restrict__ ptr4  ){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    CoreSmallSort4(input1, input2, input3, input4);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
}


inline void CoreSmallSort5(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4, __m512i& input5 ){
    CoreSmallSort4(input, input2, input3, input4);
    input5 = CoreSmallSort(input5);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);

        input5 = _mm512_max_epi64(input4, permNeigh5);
        input4 = _mm512_min_epi64(input4, permNeigh5);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input3, inputCopy);
        input3 = _mm512_max_epi64(input3, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xF0, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xF0, permNeighMax5);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xCC, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xCC, permNeighMax5);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xAA, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xAA, permNeighMax5);
    }
}


inline void CoreSmallSort5(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                            uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5 ){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    CoreSmallSort5(input1, input2, input3, input4, input5);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
}


inline void CoreSmallSort6(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4, __m512i& input5, __m512i& input6 ){
    CoreSmallSort4(input, input2, input3, input4);
    CoreSmallSort2(input5, input6);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);

        input5 = _mm512_max_epi64(input4, permNeigh5);
        input6 = _mm512_max_epi64(input3, permNeigh6);

        input4 = _mm512_min_epi64(input4, permNeigh5);
        input3 = _mm512_min_epi64(input3, permNeigh6);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input3, inputCopy);
        input3 = _mm512_max_epi64(input3, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input5;
        input5 = _mm512_min_epi64(input6, inputCopy);
        input6 = _mm512_max_epi64(input6, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xF0, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xF0, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xF0, permNeighMax6);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xCC, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xCC, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xCC, permNeighMax6);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xAA, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xAA, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xAA, permNeighMax6);
    }
}


inline void CoreSmallSort6(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                            uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5, uint64_t* __restrict__ ptr6 ){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    __m512i input6 = _mm512_loadu_si512(ptr6);
    CoreSmallSort6(input1, input2, input3, input4, input5, input6);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
    _mm512_storeu_si512(ptr6, input6);
}


inline void CoreSmallSort7(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                            __m512i& input5, __m512i& input6, __m512i& input7 ){
    CoreSmallSort4(input, input2, input3, input4);
    CoreSmallSort3(input5, input6, input7);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);

        input5 = _mm512_max_epi64(input4, permNeigh5);
        input6 = _mm512_max_epi64(input3, permNeigh6);
        input7 = _mm512_max_epi64(input2, permNeigh7);

        input4 = _mm512_min_epi64(input4, permNeigh5);
        input3 = _mm512_min_epi64(input3, permNeigh6);
        input2 = _mm512_min_epi64(input2, permNeigh7);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input3, inputCopy);
        input3 = _mm512_max_epi64(input3, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input5;
        input5 = _mm512_min_epi64(input7, inputCopy);
        input7 = _mm512_max_epi64(input7, inputCopy);
    }
    {
        __m512i inputCopy = input5;
        input5 = _mm512_min_epi64(input6, inputCopy);
        input6 = _mm512_max_epi64(input6, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xF0, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xF0, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xF0, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xF0, permNeighMax7);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xCC, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xCC, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xCC, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xCC, permNeighMax7);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xAA, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xAA, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xAA, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xAA, permNeighMax7);
    }
}


inline void CoreSmallSort7(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                            uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5, uint64_t* __restrict__ ptr6,
                            uint64_t* __restrict__ ptr7){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    __m512i input6 = _mm512_loadu_si512(ptr6);
    __m512i input7 = _mm512_loadu_si512(ptr7);
    CoreSmallSort7(input1, input2, input3, input4, input5, input6, input7);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
    _mm512_storeu_si512(ptr6, input6);
    _mm512_storeu_si512(ptr7, input7);
}


inline void CoreSmallSort8(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                            __m512i& input5, __m512i& input6, __m512i& input7, __m512i& input8 ){
    CoreSmallSort4(input, input2, input3, input4);
    CoreSmallSort4(input5, input6, input7, input8);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeigh8 = _mm512_permutexvar_epi64(idxNoNeigh, input8);

        input5 = _mm512_max_epi64(input4, permNeigh5);
        input6 = _mm512_max_epi64(input3, permNeigh6);
        input7 = _mm512_max_epi64(input2, permNeigh7);
        input8 = _mm512_max_epi64(input, permNeigh8);

        input4 = _mm512_min_epi64(input4, permNeigh5);
        input3 = _mm512_min_epi64(input3, permNeigh6);
        input2 = _mm512_min_epi64(input2, permNeigh7);
        input = _mm512_min_epi64(input, permNeigh8);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input3, inputCopy);
        input3 = _mm512_max_epi64(input3, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input5;
        input5 = _mm512_min_epi64(input7, inputCopy);
        input7 = _mm512_max_epi64(input7, inputCopy);
    }
    {
        __m512i inputCopy = input6;
        input6 = _mm512_min_epi64(input8, inputCopy);
        input8 = _mm512_max_epi64(input8, inputCopy);
    }
    {
        __m512i inputCopy = input5;
        input5 = _mm512_min_epi64(input6, inputCopy);
        input6 = _mm512_max_epi64(input6, inputCopy);
    }
    {
        __m512i inputCopy = input7;
        input7 = _mm512_min_epi64(input8, inputCopy);
        input8 = _mm512_max_epi64(input8, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeigh8 = _mm512_permutexvar_epi64(idxNoNeigh, input8);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMin8 = _mm512_min_epi64(permNeigh8, input8);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        __m512i permNeighMax8 = _mm512_max_epi64(permNeigh8, input8);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xF0, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xF0, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xF0, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xF0, permNeighMax7);
        input8 = _mm512_mask_mov_epi64(permNeighMin8, 0xF0, permNeighMax8);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeigh8 = _mm512_permutexvar_epi64(idxNoNeigh, input8);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMin8 = _mm512_min_epi64(permNeigh8, input8);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        __m512i permNeighMax8 = _mm512_max_epi64(permNeigh8, input8);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xCC, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xCC, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xCC, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xCC, permNeighMax7);
        input8 = _mm512_mask_mov_epi64(permNeighMin8, 0xCC, permNeighMax8);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeigh8 = _mm512_permutexvar_epi64(idxNoNeigh, input8);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMin8 = _mm512_min_epi64(permNeigh8, input8);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        __m512i permNeighMax8 = _mm512_max_epi64(permNeigh8, input8);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xAA, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xAA, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xAA, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xAA, permNeighMax7);
        input8 = _mm512_mask_mov_epi64(permNeighMin8, 0xAA, permNeighMax8);
    }
}



inline void CoreSmallSort8(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                            uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5, uint64_t* __restrict__ ptr6,
                            uint64_t* __restrict__ ptr7, uint64_t* __restrict__ ptr8 ){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    __m512i input6 = _mm512_loadu_si512(ptr6);
    __m512i input7 = _mm512_loadu_si512(ptr7);
    __m512i input8 = _mm512_loadu_si512(ptr8);
    CoreSmallSort8(input1, input2, input3, input4, input5, input6, input7, input8);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
    _mm512_storeu_si512(ptr6, input6);
    _mm512_storeu_si512(ptr7, input7);
    _mm512_storeu_si512(ptr8, input8);
}


inline void CoreSmallEnd1(__m512i& input){
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
    }
}

inline void CoreSmallEnd2(__m512i& input, __m512i& input2){
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
    }
}

inline void CoreSmallEnd3(__m512i& input, __m512i& input2, __m512i& input3){
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input3, inputCopy);
        input3 = _mm512_max_epi64(input3, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
    }
}

inline void CoreSmallEnd4(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4){
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input3, inputCopy);
        input3 = _mm512_max_epi64(input3, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xF0, permNeighMax4);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xCC, permNeighMax4);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xAA, permNeighMax4);
    }
}

inline void CoreSmallEnd5(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                              __m512i& input5){
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input5, inputCopy);
        input5 = _mm512_max_epi64(input5, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input3, inputCopy);
        input3 = _mm512_max_epi64(input3, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xF0, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xF0, permNeighMax5);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xCC, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xCC, permNeighMax5);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xAA, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xAA, permNeighMax5);
    }
}

inline void CoreSmallEnd6(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                              __m512i& input5, __m512i& input6){
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input5, inputCopy);
        input5 = _mm512_max_epi64(input5, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input6, inputCopy);
        input6 = _mm512_max_epi64(input6, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input3, inputCopy);
        input3 = _mm512_max_epi64(input3, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input5;
        input5 = _mm512_min_epi64(input6, inputCopy);
        input6 = _mm512_max_epi64(input6, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xF0, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xF0, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xF0, permNeighMax6);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xCC, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xCC, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xCC, permNeighMax6);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xAA, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xAA, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xAA, permNeighMax6);
    }
}

inline void CoreSmallEnd7(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                              __m512i& input5, __m512i& input6, __m512i& input7){
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input5, inputCopy);
        input5 = _mm512_max_epi64(input5, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input6, inputCopy);
        input6 = _mm512_max_epi64(input6, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input7, inputCopy);
        input7 = _mm512_max_epi64(input7, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input3, inputCopy);
        input3 = _mm512_max_epi64(input3, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input5;
        input5 = _mm512_min_epi64(input7, inputCopy);
        input7 = _mm512_max_epi64(input7, inputCopy);
    }
    {
        __m512i inputCopy = input5;
        input5 = _mm512_min_epi64(input6, inputCopy);
        input6 = _mm512_max_epi64(input6, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xF0, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xF0, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xF0, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xF0, permNeighMax7);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xCC, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xCC, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xCC, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xCC, permNeighMax7);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xAA, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xAA, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xAA, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xAA, permNeighMax7);
    }
}


inline void CoreSmallEnd8(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                              __m512i& input5, __m512i& input6, __m512i& input7, __m512i& input8 ){
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input5, inputCopy);
        input5 = _mm512_max_epi64(input5, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input6, inputCopy);
        input6 = _mm512_max_epi64(input6, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input7, inputCopy);
        input7 = _mm512_max_epi64(input7, inputCopy);
    }
    {
        __m512i inputCopy = input4;
        input4 = _mm512_min_epi64(input8, inputCopy);
        input8 = _mm512_max_epi64(input8, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input3, inputCopy);
        input3 = _mm512_max_epi64(input3, inputCopy);
    }
    {
        __m512i inputCopy = input2;
        input2 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input;
        input = _mm512_min_epi64(input2, inputCopy);
        input2 = _mm512_max_epi64(input2, inputCopy);
    }
    {
        __m512i inputCopy = input3;
        input3 = _mm512_min_epi64(input4, inputCopy);
        input4 = _mm512_max_epi64(input4, inputCopy);
    }
    {
        __m512i inputCopy = input5;
        input5 = _mm512_min_epi64(input7, inputCopy);
        input7 = _mm512_max_epi64(input7, inputCopy);
    }
    {
        __m512i inputCopy = input6;
        input6 = _mm512_min_epi64(input8, inputCopy);
        input8 = _mm512_max_epi64(input8, inputCopy);
    }
    {
        __m512i inputCopy = input5;
        input5 = _mm512_min_epi64(input6, inputCopy);
        input6 = _mm512_max_epi64(input6, inputCopy);
    }
    {
        __m512i inputCopy = input7;
        input7 = _mm512_min_epi64(input8, inputCopy);
        input8 = _mm512_max_epi64(input8, inputCopy);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(3, 2, 1, 0, 7, 6, 5, 4);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeigh8 = _mm512_permutexvar_epi64(idxNoNeigh, input8);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMin8 = _mm512_min_epi64(permNeigh8, input8);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        __m512i permNeighMax8 = _mm512_max_epi64(permNeigh8, input8);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xF0, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xF0, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xF0, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xF0, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xF0, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xF0, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xF0, permNeighMax7);
        input8 = _mm512_mask_mov_epi64(permNeighMin8, 0xF0, permNeighMax8);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(5, 4, 7, 6, 1, 0, 3, 2);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeigh8 = _mm512_permutexvar_epi64(idxNoNeigh, input8);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMin8 = _mm512_min_epi64(permNeigh8, input8);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        __m512i permNeighMax8 = _mm512_max_epi64(permNeigh8, input8);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xCC, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xCC, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xCC, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xCC, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xCC, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xCC, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xCC, permNeighMax7);
        input8 = _mm512_mask_mov_epi64(permNeighMin8, 0xCC, permNeighMax8);
    }
    {
        __m512i idxNoNeigh = _mm512_set_epi64(6, 7, 4, 5, 2, 3, 0, 1);
        __m512i permNeigh = _mm512_permutexvar_epi64(idxNoNeigh, input);
        __m512i permNeigh2 = _mm512_permutexvar_epi64(idxNoNeigh, input2);
        __m512i permNeigh3 = _mm512_permutexvar_epi64(idxNoNeigh, input3);
        __m512i permNeigh4 = _mm512_permutexvar_epi64(idxNoNeigh, input4);
        __m512i permNeigh5 = _mm512_permutexvar_epi64(idxNoNeigh, input5);
        __m512i permNeigh6 = _mm512_permutexvar_epi64(idxNoNeigh, input6);
        __m512i permNeigh7 = _mm512_permutexvar_epi64(idxNoNeigh, input7);
        __m512i permNeigh8 = _mm512_permutexvar_epi64(idxNoNeigh, input8);
        __m512i permNeighMin = _mm512_min_epi64(permNeigh, input);
        __m512i permNeighMin2 = _mm512_min_epi64(permNeigh2, input2);
        __m512i permNeighMin3 = _mm512_min_epi64(permNeigh3, input3);
        __m512i permNeighMin4 = _mm512_min_epi64(permNeigh4, input4);
        __m512i permNeighMin5 = _mm512_min_epi64(permNeigh5, input5);
        __m512i permNeighMin6 = _mm512_min_epi64(permNeigh6, input6);
        __m512i permNeighMin7 = _mm512_min_epi64(permNeigh7, input7);
        __m512i permNeighMin8 = _mm512_min_epi64(permNeigh8, input8);
        __m512i permNeighMax = _mm512_max_epi64(permNeigh, input);
        __m512i permNeighMax2 = _mm512_max_epi64(permNeigh2, input2);
        __m512i permNeighMax3 = _mm512_max_epi64(permNeigh3, input3);
        __m512i permNeighMax4 = _mm512_max_epi64(permNeigh4, input4);
        __m512i permNeighMax5 = _mm512_max_epi64(permNeigh5, input5);
        __m512i permNeighMax6 = _mm512_max_epi64(permNeigh6, input6);
        __m512i permNeighMax7 = _mm512_max_epi64(permNeigh7, input7);
        __m512i permNeighMax8 = _mm512_max_epi64(permNeigh8, input8);
        input = _mm512_mask_mov_epi64(permNeighMin, 0xAA, permNeighMax);
        input2 = _mm512_mask_mov_epi64(permNeighMin2, 0xAA, permNeighMax2);
        input3 = _mm512_mask_mov_epi64(permNeighMin3, 0xAA, permNeighMax3);
        input4 = _mm512_mask_mov_epi64(permNeighMin4, 0xAA, permNeighMax4);
        input5 = _mm512_mask_mov_epi64(permNeighMin5, 0xAA, permNeighMax5);
        input6 = _mm512_mask_mov_epi64(permNeighMin6, 0xAA, permNeighMax6);
        input7 = _mm512_mask_mov_epi64(permNeighMin7, 0xAA, permNeighMax7);
        input8 = _mm512_mask_mov_epi64(permNeighMin8, 0xAA, permNeighMax8);
    }
}

inline void CoreSmallSort9(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                            __m512i& input5, __m512i& input6, __m512i& input7, __m512i& input8,
                            __m512i& input9){
    CoreSmallSort8(input, input2, input3, input4, input5, input6, input7, input8);
    input9 = CoreSmallSort(input9);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh9 = _mm512_permutexvar_epi64(idxNoNeigh, input9);

        input9 = _mm512_max_epi64(input8, permNeigh9);

        input8 = _mm512_min_epi64(input8, permNeigh9);
    }
    CoreSmallEnd8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallEnd1(input9);
}



inline void CoreSmallSort9(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                            uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5, uint64_t* __restrict__ ptr6,
                            uint64_t* __restrict__ ptr7, uint64_t* __restrict__ ptr8,
                            uint64_t* __restrict__ ptr9){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    __m512i input6 = _mm512_loadu_si512(ptr6);
    __m512i input7 = _mm512_loadu_si512(ptr7);
    __m512i input8 = _mm512_loadu_si512(ptr8);
    __m512i input9 = _mm512_loadu_si512(ptr9);
    CoreSmallSort9(input1, input2, input3, input4, input5, input6, input7, input8,
                    input9);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
    _mm512_storeu_si512(ptr6, input6);
    _mm512_storeu_si512(ptr7, input7);
    _mm512_storeu_si512(ptr8, input8);
    _mm512_storeu_si512(ptr9, input9);
}


inline void CoreSmallSort10(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                             __m512i& input5, __m512i& input6, __m512i& input7, __m512i& input8,
                             __m512i& input9, __m512i& input10){
    CoreSmallSort8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallSort2(input9, input10);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh9 = _mm512_permutexvar_epi64(idxNoNeigh, input9);
        __m512i permNeigh10 = _mm512_permutexvar_epi64(idxNoNeigh, input10);

        input9 = _mm512_max_epi64(input8, permNeigh9);
        input10 = _mm512_max_epi64(input7, permNeigh10);

        input8 = _mm512_min_epi64(input8, permNeigh9);
        input7 = _mm512_min_epi64(input7, permNeigh10);
    }
    CoreSmallEnd8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallEnd2(input9, input10);
}



inline void CoreSmallSort10(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                             uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5, uint64_t* __restrict__ ptr6,
                             uint64_t* __restrict__ ptr7, uint64_t* __restrict__ ptr8,
                             uint64_t* __restrict__ ptr9, uint64_t* __restrict__ ptr10){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    __m512i input6 = _mm512_loadu_si512(ptr6);
    __m512i input7 = _mm512_loadu_si512(ptr7);
    __m512i input8 = _mm512_loadu_si512(ptr8);
    __m512i input9 = _mm512_loadu_si512(ptr9);
    __m512i input10 = _mm512_loadu_si512(ptr10);
    CoreSmallSort10(input1, input2, input3, input4, input5, input6, input7, input8,
                     input9, input10);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
    _mm512_storeu_si512(ptr6, input6);
    _mm512_storeu_si512(ptr7, input7);
    _mm512_storeu_si512(ptr8, input8);
    _mm512_storeu_si512(ptr9, input9);
    _mm512_storeu_si512(ptr10, input10);
}

inline void CoreSmallSort11(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                             __m512i& input5, __m512i& input6, __m512i& input7, __m512i& input8,
                             __m512i& input9, __m512i& input10, __m512i& input11){
    CoreSmallSort8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallSort3(input9, input10, input11);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh9 = _mm512_permutexvar_epi64(idxNoNeigh, input9);
        __m512i permNeigh10 = _mm512_permutexvar_epi64(idxNoNeigh, input10);
        __m512i permNeigh11 = _mm512_permutexvar_epi64(idxNoNeigh, input11);

        input9 = _mm512_max_epi64(input8, permNeigh9);
        input10 = _mm512_max_epi64(input7, permNeigh10);
        input11 = _mm512_max_epi64(input6, permNeigh11);

        input8 = _mm512_min_epi64(input8, permNeigh9);
        input7 = _mm512_min_epi64(input7, permNeigh10);
        input6 = _mm512_min_epi64(input6, permNeigh11);
    }
    CoreSmallEnd8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallEnd3(input9, input10, input11);
}



inline void CoreSmallSort11(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                             uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5, uint64_t* __restrict__ ptr6,
                             uint64_t* __restrict__ ptr7, uint64_t* __restrict__ ptr8,
                             uint64_t* __restrict__ ptr9, uint64_t* __restrict__ ptr10, uint64_t* __restrict__ ptr11){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    __m512i input6 = _mm512_loadu_si512(ptr6);
    __m512i input7 = _mm512_loadu_si512(ptr7);
    __m512i input8 = _mm512_loadu_si512(ptr8);
    __m512i input9 = _mm512_loadu_si512(ptr9);
    __m512i input10 = _mm512_loadu_si512(ptr10);
    __m512i input11 = _mm512_loadu_si512(ptr11);
    CoreSmallSort11(input1, input2, input3, input4, input5, input6, input7, input8,
                     input9, input10, input11);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
    _mm512_storeu_si512(ptr6, input6);
    _mm512_storeu_si512(ptr7, input7);
    _mm512_storeu_si512(ptr8, input8);
    _mm512_storeu_si512(ptr9, input9);
    _mm512_storeu_si512(ptr10, input10);
    _mm512_storeu_si512(ptr11, input11);
}

inline void CoreSmallSort12(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                             __m512i& input5, __m512i& input6, __m512i& input7, __m512i& input8,
                             __m512i& input9, __m512i& input10, __m512i& input11, __m512i& input12){
    CoreSmallSort8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallSort4(input9, input10, input11, input12);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh9 = _mm512_permutexvar_epi64(idxNoNeigh, input9);
        __m512i permNeigh10 = _mm512_permutexvar_epi64(idxNoNeigh, input10);
        __m512i permNeigh11 = _mm512_permutexvar_epi64(idxNoNeigh, input11);
        __m512i permNeigh12 = _mm512_permutexvar_epi64(idxNoNeigh, input12);

        input9 = _mm512_max_epi64(input8, permNeigh9);
        input10 = _mm512_max_epi64(input7, permNeigh10);
        input11 = _mm512_max_epi64(input6, permNeigh11);
        input12 = _mm512_max_epi64(input5, permNeigh12);

        input8 = _mm512_min_epi64(input8, permNeigh9);
        input7 = _mm512_min_epi64(input7, permNeigh10);
        input6 = _mm512_min_epi64(input6, permNeigh11);
        input5 = _mm512_min_epi64(input5, permNeigh12);
    }
    CoreSmallEnd8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallEnd4(input9, input10, input11, input12);
}



inline void CoreSmallSort12(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                             uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5, uint64_t* __restrict__ ptr6,
                             uint64_t* __restrict__ ptr7, uint64_t* __restrict__ ptr8,
                             uint64_t* __restrict__ ptr9, uint64_t* __restrict__ ptr10, uint64_t* __restrict__ ptr11,
                             uint64_t* __restrict__ ptr12){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    __m512i input6 = _mm512_loadu_si512(ptr6);
    __m512i input7 = _mm512_loadu_si512(ptr7);
    __m512i input8 = _mm512_loadu_si512(ptr8);
    __m512i input9 = _mm512_loadu_si512(ptr9);
    __m512i input10 = _mm512_loadu_si512(ptr10);
    __m512i input11 = _mm512_loadu_si512(ptr11);
    __m512i input12 = _mm512_loadu_si512(ptr12);
    CoreSmallSort12(input1, input2, input3, input4, input5, input6, input7, input8,
                     input9, input10, input11, input12);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
    _mm512_storeu_si512(ptr6, input6);
    _mm512_storeu_si512(ptr7, input7);
    _mm512_storeu_si512(ptr8, input8);
    _mm512_storeu_si512(ptr9, input9);
    _mm512_storeu_si512(ptr10, input10);
    _mm512_storeu_si512(ptr11, input11);
    _mm512_storeu_si512(ptr12, input12);
}

inline void CoreSmallSort13(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                             __m512i& input5, __m512i& input6, __m512i& input7, __m512i& input8,
                             __m512i& input9, __m512i& input10, __m512i& input11, __m512i& input12,
                             __m512i& input13){
    CoreSmallSort8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallSort5(input9, input10, input11, input12, input13);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh9 = _mm512_permutexvar_epi64(idxNoNeigh, input9);
        __m512i permNeigh10 = _mm512_permutexvar_epi64(idxNoNeigh, input10);
        __m512i permNeigh11 = _mm512_permutexvar_epi64(idxNoNeigh, input11);
        __m512i permNeigh12 = _mm512_permutexvar_epi64(idxNoNeigh, input12);
        __m512i permNeigh13 = _mm512_permutexvar_epi64(idxNoNeigh, input13);

        input9 = _mm512_max_epi64(input8, permNeigh9);
        input10 = _mm512_max_epi64(input7, permNeigh10);
        input11 = _mm512_max_epi64(input6, permNeigh11);
        input12 = _mm512_max_epi64(input5, permNeigh12);
        input13 = _mm512_max_epi64(input4, permNeigh13);

        input8 = _mm512_min_epi64(input8, permNeigh9);
        input7 = _mm512_min_epi64(input7, permNeigh10);
        input6 = _mm512_min_epi64(input6, permNeigh11);
        input5 = _mm512_min_epi64(input5, permNeigh12);
        input4 = _mm512_min_epi64(input4, permNeigh13);
    }
    CoreSmallEnd8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallEnd5(input9, input10, input11, input12, input13);
}



inline void CoreSmallSort13(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                             uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5, uint64_t* __restrict__ ptr6,
                             uint64_t* __restrict__ ptr7, uint64_t* __restrict__ ptr8,
                             uint64_t* __restrict__ ptr9, uint64_t* __restrict__ ptr10, uint64_t* __restrict__ ptr11,
                             uint64_t* __restrict__ ptr12, uint64_t* __restrict__ ptr13){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    __m512i input6 = _mm512_loadu_si512(ptr6);
    __m512i input7 = _mm512_loadu_si512(ptr7);
    __m512i input8 = _mm512_loadu_si512(ptr8);
    __m512i input9 = _mm512_loadu_si512(ptr9);
    __m512i input10 = _mm512_loadu_si512(ptr10);
    __m512i input11 = _mm512_loadu_si512(ptr11);
    __m512i input12 = _mm512_loadu_si512(ptr12);
    __m512i input13 = _mm512_loadu_si512(ptr13);
    CoreSmallSort13(input1, input2, input3, input4, input5, input6, input7, input8,
                     input9, input10, input11, input12, input13);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
    _mm512_storeu_si512(ptr6, input6);
    _mm512_storeu_si512(ptr7, input7);
    _mm512_storeu_si512(ptr8, input8);
    _mm512_storeu_si512(ptr9, input9);
    _mm512_storeu_si512(ptr10, input10);
    _mm512_storeu_si512(ptr11, input11);
    _mm512_storeu_si512(ptr12, input12);
    _mm512_storeu_si512(ptr13, input13);
}

inline void CoreSmallSort14(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                             __m512i& input5, __m512i& input6, __m512i& input7, __m512i& input8,
                             __m512i& input9, __m512i& input10, __m512i& input11, __m512i& input12,
                             __m512i& input13, __m512i& input14){
    CoreSmallSort8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallSort6(input9, input10, input11, input12, input13, input14);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh9 = _mm512_permutexvar_epi64(idxNoNeigh, input9);
        __m512i permNeigh10 = _mm512_permutexvar_epi64(idxNoNeigh, input10);
        __m512i permNeigh11 = _mm512_permutexvar_epi64(idxNoNeigh, input11);
        __m512i permNeigh12 = _mm512_permutexvar_epi64(idxNoNeigh, input12);
        __m512i permNeigh13 = _mm512_permutexvar_epi64(idxNoNeigh, input13);
        __m512i permNeigh14 = _mm512_permutexvar_epi64(idxNoNeigh, input14);

        input9 = _mm512_max_epi64(input8, permNeigh9);
        input10 = _mm512_max_epi64(input7, permNeigh10);
        input11 = _mm512_max_epi64(input6, permNeigh11);
        input12 = _mm512_max_epi64(input5, permNeigh12);
        input13 = _mm512_max_epi64(input4, permNeigh13);
        input14 = _mm512_max_epi64(input3, permNeigh14);

        input8 = _mm512_min_epi64(input8, permNeigh9);
        input7 = _mm512_min_epi64(input7, permNeigh10);
        input6 = _mm512_min_epi64(input6, permNeigh11);
        input5 = _mm512_min_epi64(input5, permNeigh12);
        input4 = _mm512_min_epi64(input4, permNeigh13);
        input3 = _mm512_min_epi64(input3, permNeigh14);
    }
    CoreSmallEnd8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallEnd6(input9, input10, input11, input12, input13, input14);
}



inline void CoreSmallSort14(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                             uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5, uint64_t* __restrict__ ptr6,
                             uint64_t* __restrict__ ptr7, uint64_t* __restrict__ ptr8,
                             uint64_t* __restrict__ ptr9, uint64_t* __restrict__ ptr10, uint64_t* __restrict__ ptr11,
                             uint64_t* __restrict__ ptr12, uint64_t* __restrict__ ptr13, uint64_t* __restrict__ ptr14){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    __m512i input6 = _mm512_loadu_si512(ptr6);
    __m512i input7 = _mm512_loadu_si512(ptr7);
    __m512i input8 = _mm512_loadu_si512(ptr8);
    __m512i input9 = _mm512_loadu_si512(ptr9);
    __m512i input10 = _mm512_loadu_si512(ptr10);
    __m512i input11 = _mm512_loadu_si512(ptr11);
    __m512i input12 = _mm512_loadu_si512(ptr12);
    __m512i input13 = _mm512_loadu_si512(ptr13);
    __m512i input14 = _mm512_loadu_si512(ptr14);
    CoreSmallSort14(input1, input2, input3, input4, input5, input6, input7, input8,
                     input9, input10, input11, input12, input13, input14);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
    _mm512_storeu_si512(ptr6, input6);
    _mm512_storeu_si512(ptr7, input7);
    _mm512_storeu_si512(ptr8, input8);
    _mm512_storeu_si512(ptr9, input9);
    _mm512_storeu_si512(ptr10, input10);
    _mm512_storeu_si512(ptr11, input11);
    _mm512_storeu_si512(ptr12, input12);
    _mm512_storeu_si512(ptr13, input13);
    _mm512_storeu_si512(ptr14, input14);
}

inline void CoreSmallSort15(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                             __m512i& input5, __m512i& input6, __m512i& input7, __m512i& input8,
                             __m512i& input9, __m512i& input10, __m512i& input11, __m512i& input12,
                             __m512i& input13, __m512i& input14, __m512i& input15){
    CoreSmallSort8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallSort7(input9, input10, input11, input12, input13, input14, input15);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh9 = _mm512_permutexvar_epi64(idxNoNeigh, input9);
        __m512i permNeigh10 = _mm512_permutexvar_epi64(idxNoNeigh, input10);
        __m512i permNeigh11 = _mm512_permutexvar_epi64(idxNoNeigh, input11);
        __m512i permNeigh12 = _mm512_permutexvar_epi64(idxNoNeigh, input12);
        __m512i permNeigh13 = _mm512_permutexvar_epi64(idxNoNeigh, input13);
        __m512i permNeigh14 = _mm512_permutexvar_epi64(idxNoNeigh, input14);
        __m512i permNeigh15 = _mm512_permutexvar_epi64(idxNoNeigh, input15);

        input9 = _mm512_max_epi64(input8, permNeigh9);
        input10 = _mm512_max_epi64(input7, permNeigh10);
        input11 = _mm512_max_epi64(input6, permNeigh11);
        input12 = _mm512_max_epi64(input5, permNeigh12);
        input13 = _mm512_max_epi64(input4, permNeigh13);
        input14 = _mm512_max_epi64(input3, permNeigh14);
        input15 = _mm512_max_epi64(input2, permNeigh15);

        input8 = _mm512_min_epi64(input8, permNeigh9);
        input7 = _mm512_min_epi64(input7, permNeigh10);
        input6 = _mm512_min_epi64(input6, permNeigh11);
        input5 = _mm512_min_epi64(input5, permNeigh12);
        input4 = _mm512_min_epi64(input4, permNeigh13);
        input3 = _mm512_min_epi64(input3, permNeigh14);
        input2 = _mm512_min_epi64(input2, permNeigh15);
    }
    CoreSmallEnd8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallEnd7(input9, input10, input11, input12, input13, input14, input15);
}



inline void CoreSmallSort15(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                             uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5, uint64_t* __restrict__ ptr6,
                             uint64_t* __restrict__ ptr7, uint64_t* __restrict__ ptr8,
                             uint64_t* __restrict__ ptr9, uint64_t* __restrict__ ptr10, uint64_t* __restrict__ ptr11,
                             uint64_t* __restrict__ ptr12, uint64_t* __restrict__ ptr13, uint64_t* __restrict__ ptr14,
                             uint64_t* __restrict__ ptr15){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    __m512i input6 = _mm512_loadu_si512(ptr6);
    __m512i input7 = _mm512_loadu_si512(ptr7);
    __m512i input8 = _mm512_loadu_si512(ptr8);
    __m512i input9 = _mm512_loadu_si512(ptr9);
    __m512i input10 = _mm512_loadu_si512(ptr10);
    __m512i input11 = _mm512_loadu_si512(ptr11);
    __m512i input12 = _mm512_loadu_si512(ptr12);
    __m512i input13 = _mm512_loadu_si512(ptr13);
    __m512i input14 = _mm512_loadu_si512(ptr14);
    __m512i input15 = _mm512_loadu_si512(ptr15);
    CoreSmallSort15(input1, input2, input3, input4, input5, input6, input7, input8,
                     input9, input10, input11, input12, input13, input14, input15);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
    _mm512_storeu_si512(ptr6, input6);
    _mm512_storeu_si512(ptr7, input7);
    _mm512_storeu_si512(ptr8, input8);
    _mm512_storeu_si512(ptr9, input9);
    _mm512_storeu_si512(ptr10, input10);
    _mm512_storeu_si512(ptr11, input11);
    _mm512_storeu_si512(ptr12, input12);
    _mm512_storeu_si512(ptr13, input13);
    _mm512_storeu_si512(ptr14, input14);
    _mm512_storeu_si512(ptr15, input15);
}


inline void CoreSmallSort16(__m512i& input, __m512i& input2, __m512i& input3, __m512i& input4,
                             __m512i& input5, __m512i& input6, __m512i& input7, __m512i& input8,
                             __m512i& input9, __m512i& input10, __m512i& input11, __m512i& input12,
                             __m512i& input13, __m512i& input14, __m512i& input15, __m512i& input16){
    CoreSmallSort8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallSort8(input9, input10, input11, input12, input13, input14, input15, input16);
    {
        __m512i idxNoNeigh = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        __m512i permNeigh9 = _mm512_permutexvar_epi64(idxNoNeigh, input9);
        __m512i permNeigh10 = _mm512_permutexvar_epi64(idxNoNeigh, input10);
        __m512i permNeigh11 = _mm512_permutexvar_epi64(idxNoNeigh, input11);
        __m512i permNeigh12 = _mm512_permutexvar_epi64(idxNoNeigh, input12);
        __m512i permNeigh13 = _mm512_permutexvar_epi64(idxNoNeigh, input13);
        __m512i permNeigh14 = _mm512_permutexvar_epi64(idxNoNeigh, input14);
        __m512i permNeigh15 = _mm512_permutexvar_epi64(idxNoNeigh, input15);
        __m512i permNeigh16 = _mm512_permutexvar_epi64(idxNoNeigh, input16);

        input9 = _mm512_max_epi64(input8, permNeigh9);
        input10 = _mm512_max_epi64(input7, permNeigh10);
        input11 = _mm512_max_epi64(input6, permNeigh11);
        input12 = _mm512_max_epi64(input5, permNeigh12);
        input13 = _mm512_max_epi64(input4, permNeigh13);
        input14 = _mm512_max_epi64(input3, permNeigh14);
        input15 = _mm512_max_epi64(input2, permNeigh15);
        input16 = _mm512_max_epi64(input, permNeigh16);

        input8 = _mm512_min_epi64(input8, permNeigh9);
        input7 = _mm512_min_epi64(input7, permNeigh10);
        input6 = _mm512_min_epi64(input6, permNeigh11);
        input5 = _mm512_min_epi64(input5, permNeigh12);
        input4 = _mm512_min_epi64(input4, permNeigh13);
        input3 = _mm512_min_epi64(input3, permNeigh14);
        input2 = _mm512_min_epi64(input2, permNeigh15);
        input = _mm512_min_epi64(input, permNeigh16);
    }
    CoreSmallEnd8(input, input2, input3, input4, input5, input6, input7, input8);
    CoreSmallEnd8(input9, input10, input11, input12, input13, input14, input15, input16);
}



inline void CoreSmallSort16(uint64_t* __restrict__ ptr1, uint64_t* __restrict__ ptr2, uint64_t* __restrict__ ptr3,
                             uint64_t* __restrict__ ptr4, uint64_t* __restrict__ ptr5, uint64_t* __restrict__ ptr6,
                             uint64_t* __restrict__ ptr7, uint64_t* __restrict__ ptr8,
                             uint64_t* __restrict__ ptr9, uint64_t* __restrict__ ptr10, uint64_t* __restrict__ ptr11,
                             uint64_t* __restrict__ ptr12, uint64_t* __restrict__ ptr13, uint64_t* __restrict__ ptr14,
                             uint64_t* __restrict__ ptr15, uint64_t* __restrict__ ptr16){
    __m512i input1 = _mm512_loadu_si512(ptr1);
    __m512i input2 = _mm512_loadu_si512(ptr2);
    __m512i input3 = _mm512_loadu_si512(ptr3);
    __m512i input4 = _mm512_loadu_si512(ptr4);
    __m512i input5 = _mm512_loadu_si512(ptr5);
    __m512i input6 = _mm512_loadu_si512(ptr6);
    __m512i input7 = _mm512_loadu_si512(ptr7);
    __m512i input8 = _mm512_loadu_si512(ptr8);
    __m512i input9 = _mm512_loadu_si512(ptr9);
    __m512i input10 = _mm512_loadu_si512(ptr10);
    __m512i input11 = _mm512_loadu_si512(ptr11);
    __m512i input12 = _mm512_loadu_si512(ptr12);
    __m512i input13 = _mm512_loadu_si512(ptr13);
    __m512i input14 = _mm512_loadu_si512(ptr14);
    __m512i input15 = _mm512_loadu_si512(ptr15);
    __m512i input16 = _mm512_loadu_si512(ptr16);
    CoreSmallSort16(input1, input2, input3, input4, input5, input6, input7, input8,
                     input9, input10, input11, input12, input13, input14, input15, input16);
    _mm512_storeu_si512(ptr1, input1);
    _mm512_storeu_si512(ptr2, input2);
    _mm512_storeu_si512(ptr3, input3);
    _mm512_storeu_si512(ptr4, input4);
    _mm512_storeu_si512(ptr5, input5);
    _mm512_storeu_si512(ptr6, input6);
    _mm512_storeu_si512(ptr7, input7);
    _mm512_storeu_si512(ptr8, input8);
    _mm512_storeu_si512(ptr9, input9);
    _mm512_storeu_si512(ptr10, input10);
    _mm512_storeu_si512(ptr11, input11);
    _mm512_storeu_si512(ptr12, input12);
    _mm512_storeu_si512(ptr13, input13);
    _mm512_storeu_si512(ptr14, input14);
    _mm512_storeu_si512(ptr15, input15);
    _mm512_storeu_si512(ptr16, input16);
}



inline void SmallSort16V(uint64_t* __restrict__ ptr, const size_t length){
    // length is limited to 4 times size of a vec
    const uint64_t nbValuesInVec = 8;
    const uint64_t nbVecs = (length+nbValuesInVec-1)/nbValuesInVec;
    const uint64_t rest = nbVecs*nbValuesInVec-length;
    const uint64_t lastVecSize = nbValuesInVec-rest;
    const uint64_t temp_DBL_MAX = INT64_MAX;
    const uint64_t uint64_t_max = reinterpret_cast<const uint64_t&>(temp_DBL_MAX);
    switch(nbVecs){
    case 1:
    {
        __m512i v1 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr)),
                                                         _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        v1 = CoreSmallSort(v1);
        _mm512_mask_compressstoreu_epi64(ptr, 0xFF>>rest, v1);
    }
        break;
    case 2:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+8)),
                                                         _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort2(v1,v2);
        _mm512_storeu_si512(ptr, v1);
        _mm512_mask_compressstoreu_epi64(ptr+8, 0xFF>>rest, v2);
    }
        break;
    case 3:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+16)),
                                                         _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort3(v1,v2,v3);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_mask_compressstoreu_epi64(ptr+16, 0xFF>>rest, v3);
    }
        break;
    case 4:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+24)),
                                                         _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort4(v1,v2,v3,v4);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_mask_compressstoreu_epi64(ptr+24, 0xFF>>rest, v4);
    }
        break;
    case 5:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+32)),
                                                         _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort5(v1,v2,v3,v4,v5);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_mask_compressstoreu_epi64(ptr+32, 0xFF>>rest, v5);
    }
        break;
    case 6:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = _mm512_loadu_si512(ptr+32);
        __m512i v6 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+40)),
                                                         _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort6(v1,v2,v3,v4,v5, v6);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_storeu_si512(ptr+32, v5);
        _mm512_mask_compressstoreu_epi64(ptr+40, 0xFF>>rest, v6);
    }
        break;
    case 7:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = _mm512_loadu_si512(ptr+32);
        __m512i v6 = _mm512_loadu_si512(ptr+40);
        __m512i v7 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+48)),
                                                         _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort7(v1,v2,v3,v4,v5,v6,v7);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_storeu_si512(ptr+32, v5);
        _mm512_storeu_si512(ptr+40, v6);
        _mm512_mask_compressstoreu_epi64(ptr+48, 0xFF>>rest, v7);
    }
        break;
    case 8:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = _mm512_loadu_si512(ptr+32);
        __m512i v6 = _mm512_loadu_si512(ptr+40);
        __m512i v7 = _mm512_loadu_si512(ptr+48);
        __m512i v8 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+56)),
                                                         _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort8(v1,v2,v3,v4,v5,v6,v7,v8);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_storeu_si512(ptr+32, v5);
        _mm512_storeu_si512(ptr+40, v6);
        _mm512_storeu_si512(ptr+48, v7);
        _mm512_mask_compressstoreu_epi64(ptr+56, 0xFF>>rest, v8);
    }
        break;
    case 9:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = _mm512_loadu_si512(ptr+32);
        __m512i v6 = _mm512_loadu_si512(ptr+40);
        __m512i v7 = _mm512_loadu_si512(ptr+48);
        __m512i v8 = _mm512_loadu_si512(ptr+56);
        __m512i v9 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+64)),
                                                         _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort9(v1,v2,v3,v4,v5,v6,v7,v8,v9);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_storeu_si512(ptr+32, v5);
        _mm512_storeu_si512(ptr+40, v6);
        _mm512_storeu_si512(ptr+48, v7);
        _mm512_storeu_si512(ptr+56, v8);
        _mm512_mask_compressstoreu_epi64(ptr+64, 0xFF>>rest, v9);
    }
        break;
    case 10:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = _mm512_loadu_si512(ptr+32);
        __m512i v6 = _mm512_loadu_si512(ptr+40);
        __m512i v7 = _mm512_loadu_si512(ptr+48);
        __m512i v8 = _mm512_loadu_si512(ptr+56);
        __m512i v9 = _mm512_loadu_si512(ptr+64);
        __m512i v10 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+72)),
                                                          _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort10(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_storeu_si512(ptr+32, v5);
        _mm512_storeu_si512(ptr+40, v6);
        _mm512_storeu_si512(ptr+48, v7);
        _mm512_storeu_si512(ptr+56, v8);
        _mm512_storeu_si512(ptr+64, v9);
        _mm512_mask_compressstoreu_epi64(ptr+72, 0xFF>>rest, v10);
    }
        break;
    case 11:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = _mm512_loadu_si512(ptr+32);
        __m512i v6 = _mm512_loadu_si512(ptr+40);
        __m512i v7 = _mm512_loadu_si512(ptr+48);
        __m512i v8 = _mm512_loadu_si512(ptr+56);
        __m512i v9 = _mm512_loadu_si512(ptr+64);
        __m512i v10 = _mm512_loadu_si512(ptr+72);
        __m512i v11 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+80)),
                                                          _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort11(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_storeu_si512(ptr+32, v5);
        _mm512_storeu_si512(ptr+40, v6);
        _mm512_storeu_si512(ptr+48, v7);
        _mm512_storeu_si512(ptr+56, v8);
        _mm512_storeu_si512(ptr+64, v9);
        _mm512_storeu_si512(ptr+72, v10);
        _mm512_mask_compressstoreu_epi64(ptr+80, 0xFF>>rest, v11);
    }
        break;
    case 12:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = _mm512_loadu_si512(ptr+32);
        __m512i v6 = _mm512_loadu_si512(ptr+40);
        __m512i v7 = _mm512_loadu_si512(ptr+48);
        __m512i v8 = _mm512_loadu_si512(ptr+56);
        __m512i v9 = _mm512_loadu_si512(ptr+64);
        __m512i v10 = _mm512_loadu_si512(ptr+72);
        __m512i v11 = _mm512_loadu_si512(ptr+80);
        __m512i v12 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+88)),
                                                          _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort12(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_storeu_si512(ptr+32, v5);
        _mm512_storeu_si512(ptr+40, v6);
        _mm512_storeu_si512(ptr+48, v7);
        _mm512_storeu_si512(ptr+56, v8);
        _mm512_storeu_si512(ptr+64, v9);
        _mm512_storeu_si512(ptr+72, v10);
        _mm512_storeu_si512(ptr+80, v11);
        _mm512_mask_compressstoreu_epi64(ptr+88, 0xFF>>rest, v12);
    }
        break;
    case 13:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = _mm512_loadu_si512(ptr+32);
        __m512i v6 = _mm512_loadu_si512(ptr+40);
        __m512i v7 = _mm512_loadu_si512(ptr+48);
        __m512i v8 = _mm512_loadu_si512(ptr+56);
        __m512i v9 = _mm512_loadu_si512(ptr+64);
        __m512i v10 = _mm512_loadu_si512(ptr+72);
        __m512i v11 = _mm512_loadu_si512(ptr+80);
        __m512i v12 = _mm512_loadu_si512(ptr+88);
        __m512i v13 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+96)),
                                                          _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort13(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_storeu_si512(ptr+32, v5);
        _mm512_storeu_si512(ptr+40, v6);
        _mm512_storeu_si512(ptr+48, v7);
        _mm512_storeu_si512(ptr+56, v8);
        _mm512_storeu_si512(ptr+64, v9);
        _mm512_storeu_si512(ptr+72, v10);
        _mm512_storeu_si512(ptr+80, v11);
        _mm512_storeu_si512(ptr+88, v12);
        _mm512_mask_compressstoreu_epi64(ptr+96, 0xFF>>rest, v13);
    }
        break;
    case 14:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = _mm512_loadu_si512(ptr+32);
        __m512i v6 = _mm512_loadu_si512(ptr+40);
        __m512i v7 = _mm512_loadu_si512(ptr+48);
        __m512i v8 = _mm512_loadu_si512(ptr+56);
        __m512i v9 = _mm512_loadu_si512(ptr+64);
        __m512i v10 = _mm512_loadu_si512(ptr+72);
        __m512i v11 = _mm512_loadu_si512(ptr+80);
        __m512i v12 = _mm512_loadu_si512(ptr+88);
        __m512i v13 = _mm512_loadu_si512(ptr+96);
        __m512i v14 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+104)),
                                                          _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort14(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_storeu_si512(ptr+32, v5);
        _mm512_storeu_si512(ptr+40, v6);
        _mm512_storeu_si512(ptr+48, v7);
        _mm512_storeu_si512(ptr+56, v8);
        _mm512_storeu_si512(ptr+64, v9);
        _mm512_storeu_si512(ptr+72, v10);
        _mm512_storeu_si512(ptr+80, v11);
        _mm512_storeu_si512(ptr+88, v12);
        _mm512_storeu_si512(ptr+96, v13);
        _mm512_mask_compressstoreu_epi64(ptr+104, 0xFF>>rest, v14);
    }
        break;
    case 15:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = _mm512_loadu_si512(ptr+32);
        __m512i v6 = _mm512_loadu_si512(ptr+40);
        __m512i v7 = _mm512_loadu_si512(ptr+48);
        __m512i v8 = _mm512_loadu_si512(ptr+56);
        __m512i v9 = _mm512_loadu_si512(ptr+64);
        __m512i v10 = _mm512_loadu_si512(ptr+72);
        __m512i v11 = _mm512_loadu_si512(ptr+80);
        __m512i v12 = _mm512_loadu_si512(ptr+88);
        __m512i v13 = _mm512_loadu_si512(ptr+96);
        __m512i v14 = _mm512_loadu_si512(ptr+104);
        __m512i v15 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+112)),
                                                          _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort15(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_storeu_si512(ptr+32, v5);
        _mm512_storeu_si512(ptr+40, v6);
        _mm512_storeu_si512(ptr+48, v7);
        _mm512_storeu_si512(ptr+56, v8);
        _mm512_storeu_si512(ptr+64, v9);
        _mm512_storeu_si512(ptr+72, v10);
        _mm512_storeu_si512(ptr+80, v11);
        _mm512_storeu_si512(ptr+88, v12);
        _mm512_storeu_si512(ptr+96, v13);
        _mm512_storeu_si512(ptr+104, v14);
        _mm512_mask_compressstoreu_epi64(ptr+112, 0xFF>>rest, v15);
    }
        break;
        //case 16:
    default:
    {
        __m512i v1 = _mm512_loadu_si512(ptr);
        __m512i v2 = _mm512_loadu_si512(ptr+8);
        __m512i v3 = _mm512_loadu_si512(ptr+16);
        __m512i v4 = _mm512_loadu_si512(ptr+24);
        __m512i v5 = _mm512_loadu_si512(ptr+32);
        __m512i v6 = _mm512_loadu_si512(ptr+40);
        __m512i v7 = _mm512_loadu_si512(ptr+48);
        __m512i v8 = _mm512_loadu_si512(ptr+56);
        __m512i v9 = _mm512_loadu_si512(ptr+64);
        __m512i v10 = _mm512_loadu_si512(ptr+72);
        __m512i v11 = _mm512_loadu_si512(ptr+80);
        __m512i v12 = _mm512_loadu_si512(ptr+88);
        __m512i v13 = _mm512_loadu_si512(ptr+96);
        __m512i v14 = _mm512_loadu_si512(ptr+104);
        __m512i v15 = _mm512_loadu_si512(ptr+112);
        __m512i v16 = (_mm512_or_si512((_mm512_maskz_loadu_epi64(0xFF>>rest, ptr+120)),
                                                          _mm512_maskz_set1_epi64(0xFF<<lastVecSize, uint64_t_max)));
        CoreSmallSort16(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16);
        _mm512_storeu_si512(ptr, v1);
        _mm512_storeu_si512(ptr+8, v2);
        _mm512_storeu_si512(ptr+16, v3);
        _mm512_storeu_si512(ptr+24, v4);
        _mm512_storeu_si512(ptr+32, v5);
        _mm512_storeu_si512(ptr+40, v6);
        _mm512_storeu_si512(ptr+48, v7);
        _mm512_storeu_si512(ptr+56, v8);
        _mm512_storeu_si512(ptr+64, v9);
        _mm512_storeu_si512(ptr+72, v10);
        _mm512_storeu_si512(ptr+80, v11);
        _mm512_storeu_si512(ptr+88, v12);
        _mm512_storeu_si512(ptr+96, v13);
        _mm512_storeu_si512(ptr+104, v14);
        _mm512_storeu_si512(ptr+112, v15);
        _mm512_mask_compressstoreu_epi64(ptr+120, 0xFF>>rest, v16);
    }
    }
}


////////////////////////////////////////////////////////////////////////////////
/// Partitions
////////////////////////////////////////////////////////////////////////////////

template <class SortType, class IndexType>
static inline IndexType CoreScalarPartition(SortType array[], IndexType left, IndexType right,
                                    const SortType pivot){

    for(; left <= right
         && array[left] <= pivot ; ++left){
    }

    for(IndexType idx = left ; idx <= right ; ++idx){
        if( array[idx] <= pivot ){
            std::swap(array[idx],array[left]);
            left += 1;
        }
    }

    return left;
}


inline int popcount(__mmask16 mask){
    //    int res = int(mask);
    //    res = (0x5555 & res) + (0x5555 & (res >> 1));
    //    res = (res & 0x3333) + ((res>>2) & 0x3333);
    //    res = (res & 0x0F0F) + ((res>>4) & 0x0F0F);
    //    return (res & 0xFF) + ((res>>8) & 0xFF);
#ifdef __INTEL_COMPILER
    return _mm_countbits_32(mask);
#else
	#ifdef _MSC_VER
		return __popcnt(mask);
	#else
    		return __builtin_popcount(mask);
	#endif
#endif
}


template <class IndexType>
static inline IndexType Partition512(uint64_t array[], IndexType left, IndexType right,
                                         const uint64_t pivot){
    const IndexType S = 8;//(512/8)/sizeof(uint64_t);

    if(right-left+1 < 2*S){
        return CoreScalarPartition<uint64_t,IndexType>(array, left, right, pivot);
    }

    __m512i pivotvec = _mm512_set1_epi64(pivot);

    __m512i left_val = _mm512_loadu_si512(&array[left]);
    IndexType left_w = left;
    left += S;

    IndexType right_w = right+1;
    right -= S-1;
    __m512i right_val = _mm512_loadu_si512(&array[right]);

    while(left + S <= right){
        const IndexType free_left = left - left_w;
        const IndexType free_right = right_w - right;

        __m512i val;
        if( free_left <= free_right ){
            val = _mm512_loadu_si512(&array[left]);
            left += S;
        }
        else{
            right -= S;
            val = _mm512_loadu_si512(&array[right]);
        }

        __mmask8 mask = _mm512_cmp_epi64_mask(val, pivotvec, _MM_CMPINT_LE);

        const IndexType nb_low = popcount(mask);
        const IndexType nb_high = S-nb_low;

        _mm512_mask_compressstoreu_epi64(&array[left_w],mask,val);
        left_w += nb_low;

        right_w -= nb_high;
        _mm512_mask_compressstoreu_epi64(&array[right_w],~mask,val);
    }

    {
        const IndexType remaining = right - left;
        __m512i val = _mm512_loadu_si512(&array[left]);
        left = right;

        __mmask8 mask = _mm512_cmp_epi64_mask(val, pivotvec, _MM_CMPINT_LE);

        __mmask8 mask_low = mask & ~(0xFF << remaining);
        __mmask8 mask_high = (~mask) & ~(0xFF << remaining);

        const IndexType nb_low = popcount(mask_low);
        const IndexType nb_high = popcount(mask_high);

        _mm512_mask_compressstoreu_epi64(&array[left_w],mask_low,val);
        left_w += nb_low;

        right_w -= nb_high;
        _mm512_mask_compressstoreu_epi64(&array[right_w],mask_high,val);
    }
    {
        __mmask8 mask = _mm512_cmp_epi64_mask(left_val, pivotvec, _MM_CMPINT_LE);

        const IndexType nb_low = popcount(mask);
        const IndexType nb_high = S-nb_low;

        _mm512_mask_compressstoreu_epi64(&array[left_w],mask,left_val);
        left_w += nb_low;

        right_w -= nb_high;
        _mm512_mask_compressstoreu_epi64(&array[right_w],~mask,left_val);
    }
    {
        __mmask8 mask = _mm512_cmp_epi64_mask(right_val, pivotvec, _MM_CMPINT_LE);

        const IndexType nb_low = popcount(mask);
        const IndexType nb_high = S-nb_low;

        _mm512_mask_compressstoreu_epi64(&array[left_w],mask,right_val);
        left_w += nb_low;

        right_w -= nb_high;
        _mm512_mask_compressstoreu_epi64(&array[right_w],~mask,right_val);
    }
    return left_w;
}

////////////////////////////////////////////////////////////////////////////////
/// Main functions
////////////////////////////////////////////////////////////////////////////////

template <class SortType, class IndexType = size_t>
static inline IndexType CoreSortGetPivot(const SortType array[], const IndexType left, const IndexType right){
    const IndexType middle = ((right-left)/2) + left;
    if(array[left] <= array[middle] && array[middle] <= array[right]){
        return middle;
    }
    else if(array[middle] <= array[left] && array[left] <= array[right]){
        return left;
    }
    else return right;
}


template <class SortType, class IndexType = size_t>
static inline IndexType CoreSortPivotPartition(SortType array[], const IndexType left, const IndexType right){
    if(right-left > 1){
        const IndexType pivotIdx = CoreSortGetPivot(array, left, right);
        std::swap(array[pivotIdx], array[right]);
        const IndexType part = Partition512<IndexType>(array, left, right-1, array[right]);
        std::swap(array[part], array[right]);
        return part;
    }
    return left;
}

template <class SortType, class IndexType = size_t>
static inline IndexType CoreSortPartition(SortType array[], const IndexType left, const IndexType right,
                                  const SortType pivot){
    return  Partition512<IndexType>(array, left, right, pivot);
}

template <class SortType, class IndexType = size_t>
static void CoreSort(SortType array[], const IndexType left, const IndexType right){
    static const uint64_t SortLimite = 16*64/sizeof(SortType);
    if(right-left < SortLimite){
        SmallSort16V(array+left, right-left+1);
    }
    else{
        const IndexType part = CoreSortPivotPartition<SortType,IndexType>(array, left, right);
        if(part+1 < right) CoreSort<SortType,IndexType>(array,part+1,right);
        if(part && left < part-1)  CoreSort<SortType,IndexType>(array,left,part - 1);
    }
}

template <class SortType, class IndexType = size_t>
static inline void Sort(SortType array[], const IndexType size){
    CoreSort<SortType,IndexType>(array, 0, size-1);
}

}
#endif


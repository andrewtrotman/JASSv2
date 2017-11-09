/*
	MATHS.H
	-------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Basic maths functions.
	@details Some of these already exist in the C++ Standard Library or the C Runtime Library, but they are re-implemented here for portabilit and consistency reasons.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <limits>

#include <stdio.h>
#include <stdint.h>

#include "asserts.h"

namespace JASS
	{
	/*
		CLASS MATHS
		-----------
	*/
	/*!
		@brief Basic maths functions.
	*/
	class maths
		{
		public:
			/*
				MATHS()
				-------
			*/
			/*!
				@brief Constructor.
			*/
			maths() = delete;

			/*
				MATHS::MAXIMUM()
				----------------
			*/
			/*!
				@brief Return the maximum of the two parameters.
				@details This method is order-preserving - that is, if a == b then a is returned as the max.  This method is not called max to avoid macro substitution issues.
				@param first [in] First of the two.
				@param second [in] Second of the two.
				@return The largest of the two (as compated using operator>=())
			*/
			template <typename TYPE>
			static const TYPE &maximum(const TYPE &first, const TYPE &second)
				{
				return first >= second ? first : second;
				}
			
			/*
				MATHS::MAXIMUM()
				----------------
			*/
			/*!
				@brief Return the maximum of the three parameters.
				@details This method is order-preserving (if a == b == c then a is returned, etc.).  This method is not called max to avoid macro substitution issues.
				@param first [in] First of the three.
				@param second [in] Second of the three.
				@param third [in] Third of the three.
			*/
			template <typename TYPE>
			static const TYPE &maximum(const TYPE &first, const TYPE &second, const TYPE &third)
				{
				return maximum(maximum(first, second), third);
				}

			/*
				MATHS::MINIMUM()
				----------------
			*/
			/*!
				@brief Return the minimum of the two parameters.
				@details This method is order-preserving - that is, if a == b then a is returned.  This method is not called min to avoid macro substitution issues.
				@param first [in] First of the two.
				@param second [in] Second of the two.
				@return The smallest of the two (as compated using operator<=())
			*/
			template <typename TYPE>
			static const TYPE &minimum(const TYPE &first, const TYPE &second)
				{
				return first <= second ? first : second;
				}
				
			/*
				MATHS::MINIMUM()
				----------------
				Return the minimum of three integers (order preserving)
			*/
			/*!
				@brief Return the minimum of the three parameters.
				@details This method is order-preserving (if a == b == c then a is returned, etc.).  This method is not called min to avoid macro substitution issues.
				@param first [in] First of the three.
				@param second [in] Second of the three.
				@param third [in] Third of the three.
				@return The smallest of the three (as compated using operator<=())
			*/
			template <typename TYPE>
			static const TYPE &minimum(const TYPE &first, const TYPE &second, const TYPE &third)
				{
				return minimum(minimum(first, second), third);
				}

			/*
				MATHS::FLOOR_LOG2()
				-------------------
			*/
			/*!
				@brief return the floor of log2(x)
				@param x [in] the value to compute the log of.
				@return floor(log2(x));
			*/
			static constexpr size_t floor_log2(size_t x)
				{
				constexpr uint8_t floor_log2_answer[] =
					{
					0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
					4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
					5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
					5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
					6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
					6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
					6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
					6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
					};

				size_t sum = 0;
				size_t mult = 0;

				do					// LCOV_EXCL_LINE			// gcov thinks this line isn't called (but it must be)
					{
					sum = floor_log2_answer[x & 0xFF] + mult;
					mult += 8;
					x >>= 8;
					}
				while (x != 0);

				return sum;
				}

			/*
				MATHS::CEILING_LOG2()
				---------------------
			*/
			/*!
				@brief return the ceiling of log2(x)
				@param x [in] the value to compute the log of.
				@return ceiling(log2(x));
			*/
			static constexpr size_t ceiling_log2(size_t x)
				{
				constexpr uint8_t ceiling_log2_answer[] =
					{
					0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
					5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
					6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
					6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
					7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
					8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
					8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
					8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
					8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
					8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
					8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
					8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
					8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
					};

				size_t sum = 0;
				size_t mult = 0;

				do					// LCOV_EXCL_LINE			// gcov thinks this line isn't called (but it must be)
					{
					sum = ceiling_log2_answer[x & 0xFF] + mult;
					mult += 8;
					x >>= 8;
					}
				while (x != 0);

				return sum;
				}

			/*
				MATHS::FIND_FIRST_SET_BIT()
				---------------------------
			*/
			/*!
				@brief Compute position of lowest set bit. 1 == LSB.
				@details  This uses  De Bruijn sequences to compute the lowest set bit.  See https://stackoverflow.com/questions/3465098/bit-twiddling-which-bit-is-set for more details.
				@param x [in] the value to find the first bit of.
				@return The position of the least significant set bit (counting from 1), or 0 is no bit is set.
			*/
			static inline uint32_t find_first_set_bit(uint32_t x)
				{
				static const uint32_t table[] = {0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9};
				return table[((x & -(int32_t)x) * 0x077CB531U) >> 27];
				}

			/*
				MATHS::FIND_FIRST_SET_BIT()
				---------------------------
			*/
			/*!
				@brief Compute position of lowest set bit. 1 == LSB.
				@details  This uses  De Bruijn sequences to compute the lowest set bit.  See https://stackoverflow.com/questions/3465098/bit-twiddling-which-bit-is-set for more details.
				@param x [in] the value to find the first bit of.
				@return The position of the least significant set bit (counting from 1), or 0 is no bit is set.
			*/
			static inline uint32_t find_first_set_bit(uint64_t x)
				{
				static const uint32_t table[] = {0, 1, 2, 53, 3, 7, 54, 27, 4, 38, 41, 8, 34, 55, 48, 28, 62, 5, 39, 46, 44, 42, 22, 9, 24, 35, 59, 56, 49, 18, 29, 11, 63, 52, 6, 26, 37, 40, 33, 47, 61, 45, 43, 21, 23, 58, 17, 10, 51, 25, 36, 32, 60, 20, 57, 16, 50, 31, 19, 15, 30, 14, 13, 12};
				return table[((x & -(int32_t)x) * 0x022FDD63CC95386DULL) >> 58];
				}

			/*
				MATHS::SQRT_COMPILETIME_HELPER()
				--------------------------------
			*/
			/*!
				@brief Method to compute the square root at compile time using binary search
				@details from: https://stackoverflow.com/questions/8622256/in-c11-is-sqrt-defined-as-constexpr
				@param x the value to compute the square root of
				@param lo the low value in the search
				@param hi the high value in the search
				@return the square root of x
			*/
		private:
			template <typename TYPE>
			static constexpr TYPE sqrt_compiletime_helper(TYPE x, TYPE lo, TYPE hi)
				{
				if (lo == hi)
					return lo;

				const TYPE mid = (lo + hi + 1) / 2;

				if (x / mid < mid)
					return sqrt_compiletime_helper<TYPE>(x, lo, mid - 1);
				else
					return sqrt_compiletime_helper(x, mid, hi);
				}
		public:

			/*
				MATHS::SQRT_COMPILETIME()
				-------------------------
			*/
			/*!
				@brief Method to compute the square root at compile time using binary search
				@details from: https://stackoverflow.com/questions/8622256/in-c11-is-sqrt-defined-as-constexpr
				@param x the value to compute the square root of
				@return the square root of x
			*/
			template <typename TYPE>
			static constexpr TYPE sqrt_compiletime(TYPE x)
				{
				return sqrt_compiletime_helper<TYPE>(x, 0, x / 2 + 1);
				}

			/*
				MATHS::UNITTEST()
				-----------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}



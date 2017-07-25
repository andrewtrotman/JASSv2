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
		private:
			static const uint8_t ceiling_log2_answer[];		///< Lookup table for ceiling(log2(x));
			static const uint8_t floor_log2_answer[];		///< Lookup table for floor(log2(x));

		private:
			/*
				MATHS()
				-------
			*/
			/*!
				@brief Private constructor.
				@details A private constructor makes it impossable to instantiate this class - done because all methods are static.
			*/
			maths()
				{
				/* Nothing */
				}
			
		public:
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
			static size_t floor_log2(size_t x)
				{
				size_t sum, mult = 0;

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
			static size_t ceiling_log2(size_t x)
				{
				size_t sum, mult = 0;

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
				MATHS::UNITTEST()
				-----------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}



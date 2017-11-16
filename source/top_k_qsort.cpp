/*
	TOP_K_QSORT.CPP
	-------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <vector>
#include <numeric>
#include <iostream>
#include <algorithm>

#include "top_k_qsort.h"

namespace JASS
	{
	/*
		STRUCT UNITTEST_COMPARE
		-----------------------
	*/
	/*!
		@brief functor for unit testing top_k_qsort
	*/
	struct unittest_compare
		{
		/*
			UNITTEST_COMPARE::OPERATOR()()
			------------------------------
		*/
		/*!
			@brief Comparison function
			@param a [in] pointer to first parameter
			@param b [in] pointer to second parameter
			@return 1 if *a > *b, 0 if *a == *b, -1 if *a < *b
		*/
		int operator()(const size_t a, const size_t b)
			{
			return a < b ? -1 : a == b ? 0 : 1;
			}
		} comparison;			///< Object to perform an comparison

	/*
		TOP_K_SORT::UNITTEST()
		----------------------
	*/
	void top_k_qsort::unittest(void)
		{
		bool sorted = true;
		size_t trials = 0;
		size_t max_trials = 100;
		do
			{
			trials++;
			/*
				Populate an array with the shuffled sequence 0..instance.size()
			*/
			size_t length = 1024;
			size_t top_k = 10;
			std::vector<size_t> sequence(length);
			std::iota(sequence.begin(), sequence.end(), 0);
			std::random_shuffle(sequence.begin(), sequence.end());

			/*
				Sort the top-k elements of the array
			*/
			top_k_qsort::sort(&sequence[0], sequence.size(), top_k, comparison);

			/*
				Check that the top-k are in order, and compute the largest
			*/
			size_t previous = 0;
			size_t largest = 0;
			for (size_t index = 0; index < top_k; index++)
				{
				size_t value = sequence[index];
				JASS_assert(value >= previous);
				previous = value;
				largest = maths::maximum(largest, value);
				}

			/*
				Make sure all values after the top-k are larget than the largest, while verifying that they are not in order
			*/
			previous = 0;
			sorted = true;
			for (size_t index = top_k; index < sequence.size(); index++)
				{
				JASS_assert(sequence[index] > largest);
				size_t value = sequence[index];
				if (value < previous)
					sorted = false;
				previous = value;
				}
			/*
				There is some change that it is sorted by change.  If this is the case then re-try.  Normally it won't so trials == 1.
			*/


			/*
				Try a full sort
			*/
			top_k_qsort::sort(&sequence[0], sequence.size(), comparison);

			/*
				Now verify
			*/
			previous = 0;
			for (size_t index = 0; index < top_k; index++)
				{
				size_t value = sequence[index];
				JASS_assert(value >= previous);
				previous = value;
				}
			}
		while (sorted && trials < max_trials);

		JASS_assert(trials < max_trials);

		puts("top_k_qsort::PASSED");
		}
	}

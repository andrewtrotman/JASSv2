/*
	EVALUATE_PRICE_BASED_NORMALIZED_DISCOUNTED_CUMULATIVE_GAIN.H
	------------------------------------------------------------
	Copyright (c) 2021 Vaughan Kitchen, Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the Price Based Normalized Discounted Cumulative Gain of the results list
	@author Vaughan Kitchen, Andrew Trotman
	@copyright 2021 Vaughan Kitchen, Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_PRICE_BASED_NORMALIZED_DISCOUNTED_CUMULATIVE_GAIN
		----------------------------------------------------------------
	*/
	/*!
		@brief Computes price based nDCG metric over the results list.
		@details The Price Based Normalized Discounted Cumulative Gain of the top k items is computed by generating
		the ideal gain vector (price low to high) computing Discounted Cumulative Gain on this and the results list
		both in a price based manner and dividing the results list DCG by the ideal DCG. Computing in a price based
		manner results from binning each top k items in the list into bins increasing in size exponentially where the
		first bin scores a "relevancy" of 5 and the final bin of 1 to be used in the DCG calculation. DCG is computed
		as the sum of each relevancy score of the results list with a logorithmic adjusted penalty based on the position
		of the result therefore earlier relevant results affect the score more greatly.
	*/
	class evaluate_price_based_normalized_discounted_cumulative_gain : public evaluate
		{
		private:
			evaluate_price_based_normalized_discounted_cumulative_gain() = delete;
			evaluate_price_based_normalized_discounted_cumulative_gain(std::shared_ptr<evaluate>) = delete;

		public:
			using evaluate::evaluate;
			
			/*
				EVALUATE_PRICE_BASED_NORMALIZED_DISCOUNTED_CUMULATIVE_GAIN::COMPUTE()
				---------------------------------------------------------------------
			*/
			/*!
				@brief Computes price based nDCG metric over the results list.
				@copydoc evaluate::compute()

				@details The Price Based Normalized Discounted Cumulative Gain of the top k items is computed by generating
				the ideal gain vector (price low to high) computing Discounted Cumulative Gain on this and the results list
				both in a price based manner and dividing the results list DCG by the ideal DCG. Computing in a price based
				manner results from binning each top k items in the list into bins increasing in size exponentially where the
				first bin scores a "relevancy" of 5 and the final bin of 1 to be used in the DCG calculation. DCG is computed
				as the sum of each relevancy score of the results list with a logorithmic adjusted penalty based on the position
				of the result therefore earlier relevant results affect the score more greatly.

				Although prices are assumed to be in dollars and (2 is $2.00, 2.2 = $2.20), the implementation is unit agnostic
				and therefore prices might be in cents (or pence, etc.) or even units other than financial (such as time).
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = (std::numeric_limits<size_t>::max)()) const;

			/*
				EVALUATE_PRICE_BASED_NORMALIZED_DISCOUNTED_CUMULATIVE_GAIN::UNITTEST()
				----------------------------------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}

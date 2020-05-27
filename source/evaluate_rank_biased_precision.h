/*
	EVALUATE_RANK_BIASED_PRECISION.H
	--------------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the Rank Biased Precision (RBP) of the results list
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate_relevant_returned.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_RANK_BIASED_PRECISION
		------------------------------------
	*/
	/*!
		@brief Compute the Rank Biased Precision (RBP) of the results list
	*/
	class evaluate_rank_biased_precision : public evaluate
		{
		protected:
			long double continue_probability;

		public:
			evaluate_rank_biased_precision(long double probability_of_going_on, std::shared_ptr<evaluate> store) :
				evaluate(store),
				continue_probability(probability_of_going_on)
				{
				/*
					Nothing
				*/
				}

			/*
				EVALUATE_RANK_BIASED_PRECISION::COMPUTE()
				-----------------------------------------
			*/
			/*!
				@brief Compute Rank Biased Precision (RBP).
				@copydoc evaluate::compute()
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = (std::numeric_limits<size_t>::max)()) const;

			/*
				EVALUATE_RANK_BIASED_PRECISION::UNITTEST()
				------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}

/*
	EVALUATE_MEAN_RECIPROCAL_RANK.H
	-------------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the mean reciprocal rank of the results list
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate_relevant_returned.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_MEAN_RECIPROCAL_RANK
		-----------------------------------
	*/
	/*!
		@brief Compute the mean reciprocal rank given a results list
	*/
	class evaluate_mean_reciprocal_rank : public evaluate
		{
		public:
			using evaluate::evaluate;
			/*
				EVALUATE_MEAN_RECIPROCAL_RANK::COMPUTE()
				----------------------------------------
			*/
			/*!
				@brief Compute the mean reciprocal rank.
				@copydoc evaluate::compute()
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = (std::numeric_limits<size_t>::max)()) const;

			/*
				EVALUATE_MEAN_RECIPROCAL_RANK::UNITTEST()
				-----------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				unittest_one<evaluate_mean_reciprocal_rank>(0.5);
				puts("evaluate_mean_reciprocal_rank::PASSED");
				}
		};
	}

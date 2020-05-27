/*
	EVALUATE_RELEVANT_RETURNED.H
	----------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the precision number of results in the results list that are relevant.
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_RELEVANT_RETURNED
		--------------------------------
	*/
	/*!
		@brief CCompute the precision number of results in the results list that are relevant.
	*/
	class evaluate_relevant_returned : public evaluate
		{
		public:
			using evaluate::evaluate;
			/*
				EVALUATE_RELEVANT_RETURNED::COMPUTE()
				-------------------------------------
			*/
			/*!
				@brief Compute the numner of found and relevant results.
				@copydoc evaluate::compute()
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = (std::numeric_limits<size_t>::max)()) const;

			/*
				EVALUATE_RELEVANT_RETURNED::UNITTEST()
				--------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				unittest_one<evaluate_relevant_returned>(1.0);
				puts("evaluate_relevant_returned::PASSED");
				}
		};
	}

/*
	EVALUATE_RECALL.H
	-----------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the recall of the results list
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_RECALL
		---------------------
	*/
	/*!
		@brief Compute the recall given a results (where recall is found_and_relevant / relevant)
	*/
	class evaluate_recall : public evaluate
		{
		public:
			using evaluate::evaluate;
			/*
				EVALUATE_RECALL::COMPUTE()
				--------------------------
			*/
			/*!
				@brief Compute the recall as found_and_relevant / relevant.
				@copydoc evaluate::compute()
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = (std::numeric_limits<size_t>::max)()) const;

			/*
				EVALUATE_PRECISION::UNITTEST()
				------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				unittest_one<evaluate_recall>(1.0);
				puts("evaluate_recall::PASSED");
				}
		};
	}

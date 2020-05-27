/*
	EVALUATE_PRECISION.H
	--------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the precision of the results list
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_PRECISION
		------------------------
	*/
	/*!
		@brief Compute the precision given a results (where precision is found_and_relevant / found)
	*/
	class evaluate_precision : public evaluate
		{
		public:
			using evaluate::evaluate;
			/*
				EVALUATE_PRECISION::COMPUTE()
				-----------------------------
			*/
			/*!
				@brief Compute the precision as found_and_relevant / found.
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
				unittest_one<evaluate_precision>(1.0 / 5.0);
				puts("evaluate_precision::PASSED");
				}
		};
	}

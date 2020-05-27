/*
	EVALUATE_F.H
	------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the F measure of precision and recall
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_F
		----------------
	*/
	/*!
		@brief Compute the F measure of precision and recall
	*/
	class evaluate_f : public evaluate
		{
		public:
			using evaluate::evaluate;

			/*
				EVALUATE_F::COMPUTE()
				---------------------
			*/
			/*!
				@brief Compute the F measure of precision and recall.
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
				unittest_one<evaluate_f>(2.0 * 0.2 / 1.2);
				puts("evaluate_f::PASSED");
				}
		};
	}

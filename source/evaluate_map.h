/*
	EVALUATE_MAP.H
	--------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the mean average precision (MAP) of the results list
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_MAP
		------------------
	*/
	/*!
		@brief Compute the mean average precision (MAP) given a results
	*/
	class evaluate_map : public evaluate
		{
		public:
			using evaluate::evaluate;

			/*
				EVALUATE_MAP::COMPUTE()
				-----------------------
			*/
			/*!
				@copydoc evaluate::compute()
				@brief Compute the Mean Average Precision (MAP).
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
				unittest_one<evaluate_map>(0.5);
				puts("evaluate_map::PASSED");
				}
		};
	}

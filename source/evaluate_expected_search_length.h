/*
	EVALUATE_EXPECTED_SEARCH_LENGTH.H
	---------------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the Expected Search Length of the results list
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_EXPECTED_SEARCH_LENGTH
		-------------------------------------
	*/
	/*!
		@brief Compute the Expected Search Length of the results list
		@details see  W. S. Cooper, (1968) Expected search length: A single measure of retrieval effectiveness based on the weak ordering action of retrieval systems. American Documentation, 19(1):30-41
	*/
	class evaluate_expected_search_length : public evaluate
		{
		public:
			using evaluate::evaluate;
			/*
				EVALUATE_EXPECTED_SEARCH_LENGTH::COMPUTE()
				------------------------------------------
			*/
			/*!
				@brief Compute the Expected Search Length of the results list.
				@copydoc evaluate::compute()
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = (std::numeric_limits<size_t>::max)()) const;

			/*
				EVALUATE_EXPECTED_SEARCH_LENGTH::UNITTEST()
				-------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				unittest_one<evaluate_expected_search_length>(1.0);
				puts("evaluate_expected_search_length::PASSED");
				}
		};
	}

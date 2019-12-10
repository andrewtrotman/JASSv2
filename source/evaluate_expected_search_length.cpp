/*
	EVALUATE_EXPECTED_SEARCH_LENGTH.CPP
	-----------------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>

#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_expected_search_length.h"

namespace JASS
	{
	/*
		EVALUATE_EXPECTED_SEARCH_LENGTH::COMPUTE()
		------------------------------------------
	*/
	double evaluate_expected_search_length::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		size_t which = 1;

		for (const auto &result : results_list)
			{
			auto assessment = find(query_id, result);

			if (assessment.score != 0)
				return which - 1;

			which++;

			if (which > depth)
				break;
			}

		return depth;
		}
	}

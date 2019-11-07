/*
	EVALUATE_RELEVANT_RETURNED.CPP
	------------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>

#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_relevant_returned.h"

namespace JASS
	{
	/*
		EVALUATE_RELEVANT_RETURNED::COMPUTE()
		-------------------------------------
	*/
	double evaluate_relevant_returned::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		double found_and_relevant = 0;
		size_t which = 0;

		for (const auto &result : results_list)
			{
			auto assessment = find(query_id, result);

			found_and_relevant += assessment.score;

			which++;

			if (which >= depth)
				break;
			}

		return found_and_relevant;
		}
	}

/*
	EVALUATE_PRECISION.CPP
	----------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>

#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_precision.h"

namespace JASS
	{
	/*
		EVALUATE_PRECISION::COMPUTE()
		-----------------------------
	*/
	double evaluate_precision::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		double found_and_relevant = 0;
		size_t which = 0;

		if (depth == 0)
			return 1;

		if (results_list.size() == 0)
			return 0;

		for (const auto &result : results_list)
			{
			auto assessment = find(query_id, result);

			found_and_relevant += assessment.score;

			which++;

			if (which >= depth)
				break;
			}

		if (which == 0)
			return 0;
		return found_and_relevant / (depth == (std::numeric_limits<size_t>::max)() ? which : depth);
		}
	}

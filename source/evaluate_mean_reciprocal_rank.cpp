/*
	EVALUATE_MEAN_RECIPROCAL_RANK.CPP
	---------------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>

#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_mean_reciprocal_rank.h"

namespace JASS
	{
	/*
		EVALUATE_MEAN_RECIPROCAL_RANK::COMPUTE()
		----------------------------------------
	*/
	double evaluate_mean_reciprocal_rank::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		size_t which = 1;

		for (const auto &result : results_list)
			{
			auto assessment = find(query_id, result);

			if (assessment.score != 0)
				return 1.0 / static_cast<double>(which);

			which++;

			if (which > depth)
				break;
			}

		return 0;
		}
	}

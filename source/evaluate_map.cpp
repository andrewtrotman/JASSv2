/*
	EVALUATE_MAP.CPP
	----------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>

#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_map.h"

namespace JASS
	{
	/*
		EVALUATE_MAP::COMPUTE()
		-----------------------
	*/
	double evaluate_map::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		double found_and_relevant = 0;
		double cumulative = 0;
		size_t which = 0;

		size_t number_of_relevant_assessments = relevance_count(query_id);

		/*
			No relevant assessment scores 1, No results scores 0.
		*/
		if (number_of_relevant_assessments == 0)
			return 1;
		if (results_list.size() == 0)
			return 0;

		for (const auto &result : results_list)
			{
			which++;

			auto assessment = find(query_id, result);

			if (assessment.score != 0)
				{
				found_and_relevant++;
				cumulative += found_and_relevant / which;
				}

			if (which >= depth)
				break;
			}

		return cumulative / number_of_relevant_assessments;
		}
	}

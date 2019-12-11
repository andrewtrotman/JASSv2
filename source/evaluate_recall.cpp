/*
	EVALUATE_RECALL.CPP
	-------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>

#include "asserts.h"
#include "evaluate_map.h"
#include "unittest_data.h"
#include "evaluate_recall.h"

namespace JASS
	{
	/*
		EVALUATE_RECALL::COMPUTE()
		--------------------------
	*/
	double evaluate_recall::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		double relevant = relevance_count(query_id);
		double found_and_relevant = 0;
		size_t which = 0;

		for (const auto &result : results_list)
			{
			auto assessment = find(query_id, result);

			if (assessment.score != 0)
				found_and_relevant++;

			which++;

			if (which >= depth)
				break;
			}

		return found_and_relevant / (relevant < depth ? relevant : depth);
		}
	}

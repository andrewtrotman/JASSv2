/*
	EVALUATE_F.CPP
	----------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>

#include "asserts.h"
#include "evaluate_f.h"
#include "evaluate_recall.h"
#include "evaluate_precision.h"

namespace JASS
	{
	/*
		EVALUATE_F::COMPUTE()
		---------------------
	*/
	double evaluate_f::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		evaluate_precision precision_computer(assessments_pointer);
		evaluate_recall recall_computer(assessments_pointer);

		double precision = precision_computer.compute(query_id, results_list, depth);
		double recall = recall_computer.compute(query_id, results_list, depth);

		return precision + recall == 0 ? 0 : 2 * ((precision * recall) / (precision + recall));
		}
	}

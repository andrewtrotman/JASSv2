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
		EVALUATE_MAP::RELEVANCE_COUNT()
		-------------------------------
	*/
	/*!
		@brief Count the numner of relevant results for this query
		@param query_id [in] the ID of the query
		@param gold_standard_assessments [in] the assessments
	*/
	size_t evaluate_map::relevance_count(const std::string &query_id) const
		{
		size_t relevant = 0;

		for (auto assessment = assessments.find_first(query_id); assessment < assessments.assessments.end(); assessment++)
			if (query_id == assessment->query_id && assessment->score != 0)
				relevant++;

		return relevant;
		}

	/*
		EVALUATE_MAP::COMPUTE()
		-----------------------
	*/
	double evaluate_map::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth)
		{
		double found_and_relevant = 0;
		double cumulative = 0;
		size_t which = 0;

		size_t number_of_relevant_assessments = relevance_count(query_id);

		for (const auto &result : results_list)
			{
			which++;

			auto assessment = assessments.find(query_id, result);

			if (assessment.score != 0)
				{
				found_and_relevant++;
				cumulative += found_and_relevant / which;
				}

			if (which >= depth)
				break;
			}

		return cumulative / number_of_relevant_assessments;
;
		}

	/*
		EVALUATE_MAP::UNITTEST()
		------------------------
	*/
	void evaluate_map::unittest(void)
		{
		/*
			Example results list with one relevant document
		*/
		std::vector<std::string> results_list =
			{
			"AP880217-0026",
			"AP880216-0139",			// RELEVANT (all others are non-relevant).
			"AP880212-0161",
			"AP880216-0169",
			"AP880217-0030",
			};

		/*
			One of the 5 documents is relevant
		*/
		double true_precision = 1.0 / 2.0;

		/*
			Load the sample data
		*/
		evaluate container;
		std::string copy = unittest_data::five_trec_assessments;
		container.decode_assessments_trec_qrels(copy);

		/*
			Evaluate the results list
		*/
		evaluate_map calculator(container);
		double calculated_precision = calculator.compute("1", results_list);

		/*
			Compare to 5 decimal places
		*/
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision * 10000));

		puts("evaluate_map::PASSED");
		}
	}

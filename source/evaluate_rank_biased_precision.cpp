/*
	EVALUATE_RANK_BIASED_PRECISION.CPP
	----------------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>

#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_rank_biased_precision.h"

namespace JASS
	{
	/*
		EVALUATE_RANK_BIASED_PRECISION::COMPUTE()
		-----------------------------------------
	*/
	double evaluate_rank_biased_precision::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		long double sum = 0;
		size_t which = 1;

		for (const auto &result : results_list)
			{
			auto assessment = find(query_id, result);

			if (assessment.score != 0)
				{
				auto part = powl(continue_probability, which - 1);
				sum += part;
				}

			which++;

			if (which > depth)
				break;
			}

		return sum == 0 ? 0 : (1 - continue_probability) * sum;
		}

		/*
			EVALUATE_RANK_BIASED_PRECISION::UNITTEST()
			------------------------------------------
		*/
		void evaluate_rank_biased_precision::unittest(void)
			{
			/*
				Example for Moffat et al.
				$$---$----$-----$---
				gives RBP(0.95) of 0.1881
			*/
			std::vector<std::string> results_list_one =
				{
				"two",  		// Relevant
				"two",  		// Relevant
				"one",
				"one",
				"one",
				"two",  		// Relevant
				"one",
				"one",
				"one",
				"one",
				"two",  		// Relevant
				"one",
				"one",
				"one",
				"one",
				"one",
				"two",  		// Relevant
				"one",
				"one",
				"one",
				};

			/*
				Load the sample assessments
			*/
			std::shared_ptr<evaluate> container(new evaluate);
			auto copy = unittest_data::ten_price_assessments;
			container->decode_assessments_trec_qrels(copy);

			/*
				Evaluate the results list
			*/
			evaluate_rank_biased_precision calculator(0.95L, container);
			double calculated_precision = calculator.compute("1", results_list_one);

			/*
				Compare to 4 decimal places
			*/
			double true_precision_one = 0.1881;
			JASS_assert(std::round(calculated_precision * 1000) == std::round(true_precision_one * 1000));

			puts("evaluate_rank_biased_precision::PASSED");
			}

	}

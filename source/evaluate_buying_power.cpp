/*
	EVALUATE_BUYING_POWER.CPP
	-------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>

#include "maths.h"
#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_buying_power.h"

namespace JASS
	{
	/*
		EVALUATE_BUYING_POWER::COMPUTE()
		--------------------------------
	*/
	double evaluate_buying_power::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth)
		{
		size_t which = 0;
		double lowest_priced_item = std::numeric_limits<decltype(lowest_priced_item)>::max();
		double total_spending = 0;

		/*
			Get the lowest priced item's price though a linear seach for the assessments for this query
			since this is only going to happen once per run, it doesn't seem worthwhile trying to optimise this.
		*/
		for (auto assessment = assessments.find_first(query_id); assessment != assessments.assessments.end(); assessment++)
			if ((*assessment).query_id == query_id && (*assessment).score != 0)
				{
				lowest_priced_item = assessment->score;
				auto price = prices.find("PRICE", (*assessment).document_id);
				lowest_priced_item = maths::minimum(lowest_priced_item, price.score);
				}
		/*
			There is no lowest priced item as there are no relevant items.
		*/
		if (lowest_priced_item == std::numeric_limits<decltype(lowest_priced_item)>::max())
			lowest_priced_item = 0;

		/*
			Compute the buying power.
		*/
		for (const auto &result : results_list)
			{
			/*
				Sum the total spending so far.
			*/
			auto assessment = prices.find("PRICE", result);
			total_spending += assessment.score;

			/*
				Do we have a relevant item?
			*/
			assessment = assessments.find(query_id, result);
			if (assessment.score != 0)
				return lowest_priced_item / total_spending;

			/*
				Have we exceeded the search depth?
			*/
			which++;
			if (which >= depth)
				break;
			}

		return 0;
		}

	/*
		EVALUATE_BUYING_POWER::UNITTEST()
		---------------------------------
	*/
	void evaluate_buying_power::unittest(void)
		{
		/*
			Example results list with one relevant document
		*/
		std::vector<std::string> results_list_one =
			{
			"one",
			"three",
			"two",  		// lowest priced relevant item
			"five",
			"four",
			};

		/*
			Example results list with two relevant document
		*/
		std::vector<std::string> results_list_two =
			{
			"ten",
			"nine",		// relevant
			"eight",		// relevant
			"seven",		// lowest priced relevant item
			"six",
			};

		/*
			Load the sample price list
		*/
		evaluate prices;
		std::string copy = unittest_data::ten_price_assessments_prices;
		prices.decode_assessments_trec_qrels(copy);

		/*
			Load the sample assessments
		*/
		evaluate container;
		copy = unittest_data::ten_price_assessments;
		container.decode_assessments_trec_qrels(copy);

		/*
			Evaluate the first results list
		*/
		evaluate_buying_power calculator(prices, container);
		double calculated_precision = calculator.compute("1", results_list_one);

		/*
			Compare to 5 decimal places
		*/
		double true_precision_one = 2.0 / 6.0;
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_one * 10000));

		/*
			Evaluate the second results list and check the result to 5 decimal places
		*/
		calculated_precision = calculator.compute("1", results_list_one);
		double true_precision_two = 7 / 19;
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_two * 10000));


		puts("evaluate_buying_power::PASSED");
		}
	}

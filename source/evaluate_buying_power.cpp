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
	double evaluate_buying_power::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		double lowest_priced_item = (std::numeric_limits<decltype(lowest_priced_item)>::max)();

		/*
			If we're not looking at any results then we have a perfect score.
		*/
		if (depth == 0)
			return 1;

		/*
			Get the lowest priced item's price though a linear seach for the assessments for this query
			since this is only going to happen once per run, it doesn't seem worthwhile trying to optimise this.
		*/
		for (auto assessment = find_first(query_id); assessment != assessments.end(); assessment++)
			if (assessment->query_id == query_id)
				{
				if (assessment->score != 0)
					{
					auto price = find_price(assessment->document_id);
					lowest_priced_item = maths::minimum(lowest_priced_item, price.score);
					}
				}
			else
				break;

		/*
			There is no lowest priced item as there are no relevant items.
		*/
		if (lowest_priced_item == std::numeric_limits<decltype(lowest_priced_item)>::max())
			return 1;

		/*
			Compute the buying power.
		*/
		size_t which = 0;
		double total_spending = 0;
		for (const auto &result : results_list)
			{
			/*
				Sum the total spending so far.
			*/
			auto assessment = find_price(result);
			total_spending += assessment.score;

			/*
				Do we have a relevant item?
			*/
			assessment = find(query_id, result);
			if (assessment.score != 0)
				return lowest_priced_item / total_spending;

			/*
				Have we exceeded the search depth?
			*/
			which++;
			if (which >= depth)
				break;
			}

		/*
			If there are no relevant items in the results list then the score is 0.
		*/
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
		std::shared_ptr<evaluate> prices(new evaluate);
		std::string copy = unittest_data::ten_price_assessments_prices;
		prices->decode_assessments_trec_qrels(copy);

		/*
			Load the sample assessments
		*/
		std::shared_ptr<evaluate> container(new evaluate);
		copy = unittest_data::ten_price_assessments;
		container->decode_assessments_trec_qrels(copy);

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
		calculated_precision = calculator.compute("2", results_list_two);
		double true_precision_two = 7.0 / 19.0;
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_two * 10000));


		puts("evaluate_buying_power::PASSED");
		}
	}

/*
	EVALUATE_BUYING_POWER4K.CPP
	---------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>
#include <algorithm>

#include "maths.h"
#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_buying_power4k.h"

namespace JASS
	{
	/*
		EVALUATE_BUYING_POWER4K::COMPUTE()
		----------------------------------
	*/
	double evaluate_buying_power4k::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		/*
			If we're not looking at any results then we have a perfect score.
		*/
		if (depth == 0)
			return 1;

		/*
			Get the lowest k priced item's price though a linear seach for the assessments for this query
			since this is only going to happen once per run, it doesn't seem worthwhile trying to optimise this.
			We can use a vector of doubles because we don't care which items they are, we only want the lowest prices.
		*/
		std::vector<double> query_prices;
		for (auto assessment = find_first(query_id); assessment != assessments.end(); assessment++)
			if ((*assessment).query_id == query_id)
				{
				if ((*assessment).score != 0)
					{
					auto price = find_price((*assessment).document_id);
					query_prices.push_back(price.score);
					}
				}
			else
				break;

		/*
			There are fewer than k relevant items then we cannot fulfill the query - so we get a 0.
		*/
		if (query_prices.size() < top_k)
			return 0;
		std::sort(query_prices.begin(), query_prices.end());

		size_t query_k = top_k;

		/*
			Work out what the minimum price for the top k items.
		*/
		double mimimum_cost = 0;
		for (size_t item = 0; item < query_k; item++)
			mimimum_cost += query_prices[item];

		/*
			Compute the spending up to the k-th relevant item.
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
				Have we found k relevant items yet?
			*/
			assessment = find(query_id, result);
			if (assessment.score != 0)
				if (--query_k == 0)
					return mimimum_cost / total_spending;
;
			/*
				Have we exceeded the search depth?
			*/
			which++;
			if (which >= depth)
				break;
			}
			
		/*
			At the end of the results list or exceeded the depth to check.
		*/
		return 0;
		}

	/*
		EVALUATE_BUYING_POWER4K::UNITTEST()
		-----------------------------------
	*/
	void evaluate_buying_power4k::unittest(void)
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
		evaluate_buying_power4k calculator(2, prices, container);
		double calculated_precision = calculator.compute("1", results_list_one);

		/*
			Compare to 5 decimal places
		*/
		double true_precision_one = 0;		// fails because there aren't 2 relevant items in the results list.
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_one * 10000));

		/*
			Evaluate the second results list and check the result to 5 decimal places
		*/
		calculated_precision = calculator.compute("2", results_list_two);
		double true_precision_two = 15.0 / 27.0;
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_two * 10000));


		puts("evaluate_buying_power4k::PASSED");
		}
	}

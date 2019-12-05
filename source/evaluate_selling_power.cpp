/*
	EVALUATE_SELLING_POWER.CPP
	--------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>
#include <algorithm>

#include "maths.h"
#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_selling_power.h"

namespace JASS
	{
	/*
		EVALUATE_SELLING_POWER::COMPUTE()
		---------------------------------
	*/
	double evaluate_selling_power::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
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
			There is no lowest priced items as there are no relevant items.
		*/
		if (query_prices.size() == 0)
			return 1;

		std::sort(query_prices.begin(), query_prices.end());

		/*
			If there are fewer then top_k relevant items then reduce k
		*/
		size_t query_depth = maths::minimum(query_prices.size(), depth);

		/*
			Compute the selling power of each "slot"
		*/
		size_t current_cheapest = 0;
		size_t which = 0;
		double sum_of_selling_powers = 0;
		for (const auto &result : results_list)
			{
			/*
				Have we got a relevant item?
			*/
			if (find(query_id, result).score != 0)
				{
				/*
					Get the price of the item and compute the selling power at this "slot"
				*/
				auto item_price = find_price(result);
				double selling_power = query_prices[current_cheapest] / item_price.score;			// lowest it can be divided by price it is
				current_cheapest++;		// now look for the next cheapest it might be

				sum_of_selling_powers += selling_power;
				}

			/*
				Have we exceeded the search depth?
			*/
			which++;
			if (which >= query_depth)
				break;
			}

		return sum_of_selling_powers / query_depth;
		}

	/*
		EVALUATE_SELLING_POWER::UNITTEST()
		----------------------------------
	*/
	void evaluate_selling_power::unittest(void)
		{
		/*
			Example results list with one relevant document
		*/
		std::vector<std::string> results_list_one =
			{
			"one",
			"two",  		// lowest priced relevant item
			"three",
			"four",
			"five",
			};

		/*
			Example results list with three relevant documents
		*/
		std::vector<std::string> results_list_two =
			{
			"six",
			"seven",		// lowest priced relevant item
			"eight",		// relevant
			"nine",		// relevant
			"ten",
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
		evaluate_selling_power calculator(prices, container);
		double calculated_precision = calculator.compute("1", results_list_one, 5);

		/*
			Compare to 5 decimal places
		*/
		double true_precision_one = 0;		// 0 because there is 1 relecant item so the effective depth is 1 and result[1] is not relevant
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_one * 10000));

		/*
			Evaluate the second results list and check the result to 5 decimal places
		*/
		calculated_precision = calculator.compute("2", results_list_two, 5);
		double true_precision_two = 2.0 / 3.0;
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_two * 10000));

		puts("evaluate_selling_power::PASSED");
		}
	}

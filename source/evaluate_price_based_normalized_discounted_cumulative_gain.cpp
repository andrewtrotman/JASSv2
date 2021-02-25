/*
	EVALUATE_PRICE_BASED_NORMALIZED_DISCOUNTED_CUMULATIVE_GAIN.CPP
	--------------------------------------------------------------
	Copyright (c) 2021 Vaughan Kitchen, Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>
#include <algorithm>

#include "maths.h"
#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_price_based_normalized_discounted_cumulative_gain.h"

namespace JASS
	{
	/*
		EVALUATE_PRICE_BASED_NORMALIZED_DISCOUNTED_CUMULATIVE_GAIN::COMPUTE()
		---------------------------------------------------------------------
	*/
	double evaluate_price_based_normalized_discounted_cumulative_gain::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		/*
			If we're not looking at any results then we have a perfect score.
		*/
		if (depth == 0)
			return 1;

		/*
			Get the lowest k priced item's price through a linear seach for the assessments for this query
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
			Make up the exponential bins which form the basis of the nDCG "judgments"
		*/

		double lowest_price = query_prices[0];
		double highest_price = query_prices[query_prices.size() - 1];
		double price_range = highest_price - lowest_price;
		int no_bins = 5;
		std::vector<double> bins;

		/*
			Only includes the start boundary. Use 'i < no_bins + 1' to include end of range
		*/
		for (int i = 0; i < no_bins; i++) {
			double boundary = lowest_price + price_range * ((1 - exp(i)) / (1 - exp(no_bins)));
			bins.push_back(boundary);
		}

		/*
			Compute the ideal vector
		*/
		double ideal_dcg = 0;
		size_t which = 0;
		for (const auto &query_price : query_prices)
			{
			/*
				Get the bin/"relevancy score" of the item from the ideal vector and compute dcg
			*/
			int bin_no = 0;
			for (const auto &bin : bins)
				{
				if (bin > query_price)
					break;
				bin_no++;
				}

			double relevancy_score = no_bins + 1 - bin_no;
			ideal_dcg += relevancy_score / log2(which + 2);

			/*
				Have we exceeded the search depth?
			*/
			which++;
			if (which >= depth)
				break;
			}

		/*
			Compute the dcg of the results list
		*/
		double results_dcg = 0;
		which = 0;
		for (const auto &result : results_list)
			{
			/*
				Have we got a relevant item?
			*/
			if (find(query_id, result).score != 0)
				{
				/*
					Get the bin/"relevancy score" of the item and compute dcg
				*/
				auto item_price = find_price(result);

				int bin_no = 0;
				for (const auto &bin : bins)
					{
					if (bin > item_price.score)
						break;
					bin_no++;
					}

				double relevancy_score = no_bins + 1 - bin_no;
				results_dcg += relevancy_score / log2(which + 2);
				}
			/*
				Have we exceeded the search depth?
			*/
			which++;
			if (which >= depth)
				break;
			}

		double ndcg = results_dcg / ideal_dcg;
		return ndcg;
		}

	/*
		EVALUATE_PRICE_BASED_NORMALIZED_DISCOUNTED_CUMULATIVE_GAIN::UNITTEST()
		----------------------------------------------------------------------
	*/
	void evaluate_price_based_normalized_discounted_cumulative_gain::unittest(void)
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
		evaluate_price_based_normalized_discounted_cumulative_gain calculator(prices, container);
		double calculated_precision = calculator.compute("1", results_list_one, 5);

		/*
			Compare to 5 decimal places
		*/
		double dcg_one = 5 / log2(1 + 2);		// Only one bin. Result at second rank
		double ideal_dcg_one = 5 / log2(0 + 2);		// Only one bin. Result at rank 1
		double true_precision_one = dcg_one / ideal_dcg_one;	// 0.63093
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_one * 10000));

		/*
			Evaluate the second results list and check the result to 5 decimal places
		*/
		calculated_precision = calculator.compute("2", results_list_two, 5);
		double dcg_two = 5 / log2(1 + 2) + 1 / log2(2 + 2) + 1 / log2(3 + 2); // Price relevance 5, 1, 1. Positions 2, 3, 4
		double ideal_dcg_two = 5 / log2(0 + 2) + 1 / log2(1 + 2) + 1 / log2(2 + 2); // Price relevance 5, 1, 1. Position 1, 2, 3
		double true_precision_two = dcg_two / ideal_dcg_two; // 0.66635
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_two * 10000));

		puts("evaluate_price_based_normalized_discounted_cumulative_gain::PASSED");
		}
	}

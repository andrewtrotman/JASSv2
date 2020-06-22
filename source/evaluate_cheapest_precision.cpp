/*
	EVALUATE_CHEAPEST_PRECISION.CPP
	-------------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <cmath>
#include <algorithm>

#include "maths.h"
#include "asserts.h"
#include "unittest_data.h"
#include "evaluate_cheapest_precision.h"

namespace JASS
	{
	/*
		EVALUATE_CHEAPEST_PRECISION::COMPUTE()
		--------------------------------------
	*/
	double evaluate_cheapest_precision::compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth) const
		{
		size_t which = 0;
		double found_and_relevant = 0;
		std::vector<judgement> query_prices;

		/*
			No results scores 0.
		*/
		if (results_list.size() == 0)
			return 0;

		/*
			Get the cheapest k item's prices though a linear seach for the assessments for this query
			since this is only going to happen once per run, it doesn't seem worthwhile trying to optimise this.
		*/
		for (auto assessment = find_first(query_id); assessment != assessments.end(); assessment++)
			if ((*assessment).query_id == query_id)
				{
				if ((*assessment).score != 0)
					{
					auto price = find_price((*assessment).document_id);
					query_prices.push_back(*assessment);
					}
				}
			else
				break;

		/*
			If there are no relevant items then we get a perfect score.
		*/
		if (query_prices.size() == 0 || depth == 0)
			return 1;

		std::sort(query_prices.begin(), query_prices.end());

		/*
			Slice the relevant item list for this query at either depth or the number of items in the results list
			being careful to account for ties.
		*/
		size_t slice_point = JASS::maths::minimum(depth, results_list.size());
		if (slice_point < query_prices.size())
			{
			auto price = find_price(query_prices[slice_point].document_id).score;
			while (slice_point < query_prices.size() && find_price(query_prices[slice_point].document_id).score == price)
					slice_point++;
					
			query_prices.resize(slice_point);
			}


		/*
			Compute the precision
		*/
		for (const auto &result : results_list)
			{
			/*
				Have we got a relevant item?
			*/
			judgement looking_for(query_id, result, 0);
			auto found = std::lower_bound(query_prices.begin(), query_prices.end(), looking_for);
			if (found != query_prices.end())
				if (found->document_id == result && found->query_id == query_id)
					found_and_relevant++;

			/*
				Have we exceeded the search depth?
			*/
			which++;
			if (which >= depth)
				break;
			}

		/*
			Even if there are fewer than k items in the assessments, we are computing precision against the (depth limited) entire results list.
		*/
		if (which == 0)
			return 0;
		return found_and_relevant / (depth == (std::numeric_limits<size_t>::max)() ? which : depth);
		}

	/*
		EVALUATE_CHEAPEST_PRECISION::UNITTEST()
		---------------------------------------
	*/
	void evaluate_cheapest_precision::unittest(void)
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
			"ten",
			"eleven",
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
		evaluate_cheapest_precision calculator(prices, container);
		double calculated_precision = calculator.compute("1", results_list_one, 5);

		/*
			Compare to 5 decimal places
		*/
		double true_precision_one = 1.0 / 5.0;		// 1/5 because there is 1 relevant item in a results list of length 5.
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_one * 10000));

		/*
			Evaluate the second results list and check the result to 5 decimal places
		*/
		calculated_precision = calculator.compute("2", results_list_two, 5);
		double true_precision_two = 2.0 / 5.0;
		JASS_assert(std::round(calculated_precision * 10000) == std::round(true_precision_two * 10000));

		puts("evaluate_cheapest_precision::PASSED");
		}
	}

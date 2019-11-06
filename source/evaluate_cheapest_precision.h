/*
	EVALUATE_CHEAPEST_PRECISION.H
	-----------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the precision against the k cheapest items
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_CHEAPEST_PRECISION
		---------------------------------
	*/
	/*!
		@brief Compute the Cheapest Precison for the results list.
		@details The cheapest precision of the top k items is computed by taking the k cheapest items in the assessments
		and computing pfrecision against that.  This is useful for evaluating the quality of the results at the end ofthe
		first stage in the ranking pipeline.
	*/
	class evaluate_cheapest_precision : evaluate
		{
		private:
			evaluate &prices;						///< Each item has a price regardless of the query being processed
			evaluate &assessments;				///< The assessments, which items are relevant to which queries (and at what price)

		public:
			/*
				EVALUATE_CHEAPEST_PRECISION::EVALUATE_CHEAPEST_PRECISION()
				----------------------------------------------------------
			*/
			/*!
				@brief Constructor.
				@details  As all possible prices are valid prices (0 == "free", -1 == "I'll pay for you to take it away), the
				assessments are split into two seperate parts. Ther prices of the items and the relevance of the items.  Each
				of these two are stored in trec_eval format:

			 	1 0 AP880212-0161 1

			 	where the first column is the query id, the second is ignored, the third is the document ID, and the fourth is the
			 	relevance.  The prices use a query id of "PRICE" and the relevance coulmn is the price of the item.  The assessments
			 	are the usual trec_eval format where a relevance of 1 means releance, but a relefvance of 0 is not-relevant.

				@param prices [in] An assessments object which holds the price of each item
				@param assessments [in] A pre-constructed assessments object.
			*/
			evaluate_cheapest_precision(evaluate &prices, evaluate &assessments) :
				prices(prices),
				assessments(assessments)
				{
				/* Nothing */
				}

			/*
				EVALUATE_CHEAPEST_PRECISION::COMPUTE()
				--------------------------------------
			*/
			/*!
				@brief Compute the Cheapest Precision metric over the results list.

				@details The cheapest precision of the top k items is computed by taking the k cheapest items in the assessments
				and computing pfrecision against that.  This is useful for evaluating the quality of the results at the end ofthe
				first stage in the ranking pipeline.

				Although prices are assumed to be in dollars and (2 is $2.00, 2.2 = $2.20), the implementation is unit agnostic
				and therefore prices might be in cents (or pence, etc.) or even units other than financial (such as time).

				@param query_id [in] The ID of the query being evaluated.
				@param results_list [in] The results list to measure.
				@param depth [in] How far down the results list to look.
				@return The selling power of this results list for this query.
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = std::numeric_limits<size_t>::max());

			/*
				EVALUATE_CHEAPEST_PRECISION::UNITTEST()
				---------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}

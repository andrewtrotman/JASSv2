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
	class evaluate_cheapest_precision : public evaluate
		{
		private:
			evaluate_cheapest_precision() = delete;
			evaluate_cheapest_precision(std::shared_ptr<evaluate>) = delete;

		public:
			using evaluate::evaluate;

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
				@return The Cheapest Precision of this results list for this query.
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = (std::numeric_limits<size_t>::max)()) const;

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

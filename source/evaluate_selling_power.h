/*
	EVALUATE_SELLING_POWER.H
	------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the sellng power of the results list
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_SELLING_POWER
		----------------------------
	*/
	/*!
		@brief Compute the selling power for the results list.
		@details The selling power of the top k items is computed by generating the ideal gain vector (price low to high)
		for the top k items (cost_K) then computing cost_K/actual_k for each k in the results list.  That is, 0 for non-relevance
		and for relevance it is the rtatio of idealprice to chargedprice.  This is then summed and divided by k.
	*/
	class evaluate_selling_power : public evaluate
		{
		private:
			evaluate_selling_power() = delete;
			evaluate_selling_power(std::shared_ptr<evaluate>) = delete;

		public:
			using evaluate::evaluate;
			
			/*
				EVALUATE_SELLING_POWER::COMPUTE()
				---------------------------------
			*/
			/*!
				@brief Compute the selling power metric over the results list.
				@copydoc evaluate::compute()

				@details The selling power of the top k items is computed by generating the ideal gain vector (price low to high)
				for the top k items (cost_K) then computing cost_K/actual_k for each k in the results list.  That is, 0 for non-relevance
				and for relevance it is the rtatio of idealprice to chargedprice.  This is then summed and divided by k.  We assume
				a shop-front (or set-based) model in which we only increase K if we find a relevant document at k.  This guarantees
				that the metric cannot fall outside the range [0..1].

				Although prices are assumed to be in dollars and (2 is $2.00, 2.2 = $2.20), the implementation is unit agnostic
				and therefore prices might be in cents (or pence, etc.) or even units other than financial (such as time).
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = (std::numeric_limits<size_t>::max)()) const;

			/*
				EVALUATE_SELLING_POWER::UNITTEST()
				----------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}

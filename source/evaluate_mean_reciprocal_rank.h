/*
	EVALUATE_MEAN_RECIPROCAL_RANK.H
	-------------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the mean reciprocal rank of the results list
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate_relevant_returned.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_MEAN_RECIPROCAL_RANK
		-----------------------------------
	*/
	/*!
		@brief Compute the mean reciprocal rank given a results list
	*/
	class evaluate_mean_reciprocal_rank : evaluate
		{
		private:
			evaluate &assessments;				///< The assessments, can be shared with other evaluation metrics

		public:
			/*
				EVALUATE_MEAN_RECIPROCAL_RANK::EVALUATE_MEAN_RECIPROCAL_RANK()
				--------------------------------------------------------------
			*/
			/*!
				@brief Constructor.
				@param assessments [in] A pre-constructed assessments object.
			*/
			evaluate_mean_reciprocal_rank(evaluate &assessments) :
				assessments(assessments)
				{
				/* Nothing */
				}

			/*
				EVALUATE_MEAN_RECIPROCAL_RANK::COMPUTE()
				----------------------------------------
			*/
			/*!
				@brief Compute the mean reciprocal rank.
				@param query_id [in] The ID of the query being evaluated.
				@param results_list [in] The results list to measure.
				@param depth [in] How far down the results list to look.
				@return the MRR of this results list for this query.
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = std::numeric_limits<size_t>::max());

			/*
				EVALUATE_MEAN_RECIPROCAL_RANK::UNITTEST()
				-----------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}

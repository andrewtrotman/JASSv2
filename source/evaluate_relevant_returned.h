/*
	EVALUATE_RELEVANT_RETURNED.H
	----------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Compute the precision number of results in the results list that are relevant.
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include "evaluate.h"

namespace JASS
	{
	/*
		CLASS EVALUATE_RELEVANT_RETURNED
		--------------------------------
	*/
	/*!
		@brief CCompute the precision number of results in the results list that are relevant.
	*/
	class evaluate_relevant_returned : evaluate
		{
		private:
			evaluate &assessments;				///< The assessments, can be shared with other evaluation metrics
			
		public:
			/*
				EVALUATE_RELEVANT_RETURNED::EVALUATE_RELEVANT_RETURNED()
				--------------------------------------------------------
			*/
			/*!
				@brief Constructor.
				@param assessments [in] A pre-constructed assessments object.
			*/
			evaluate_relevant_returned(evaluate &assessments) :
				assessments(assessments)
				{
				/* Nothing */
				}

			/*
				EVALUATE_RELEVANT_RETURNED::COMPUTE()
				-------------------------------------
			*/
			/*!
				@brief Compute the numner of found and relevant results.
				@param query_id [in] The ID of the query being evaluated.
				@param results_list [in] The results list to measure.
				@param depth [in] How far down the results list to look.
				@return the number of found and relevant results.
			*/
			virtual double compute(const std::string &query_id, const std::vector<std::string> &results_list, size_t depth = std::numeric_limits<size_t>::max());

			/*
				EVALUATE_RELEVANT_RETURNED::UNITTEST()
				--------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
